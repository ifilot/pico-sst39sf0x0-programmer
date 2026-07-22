#include "emulated_serial_transport.h"

#include <QtGlobal>

#include <algorithm>

namespace {

constexpr int FLASH_SIZE = 0x80000;
constexpr int BANK_SIZE = 0x4000;
constexpr int SECTOR_SIZE = 0x1000;
constexpr int BLOCK_SIZE = 0x100;

/**
 * @brief Convert a hex-encoded command suffix to an integer.
 * @param input ASCII hex digits
 * @return parsed integer or zero when parsing fails
 */
uint32_t parse_hex(const QByteArray& input) {
    bool ok = false;
    const uint32_t value = input.toUInt(&ok, 16);
    return ok ? value : 0;
}

}

/**
 * @brief Construct a new emulated transport.
 * @param _backend shared flash backend
 * @param _read_chunk_size maximum bytes exposed by one readAll() call
 */
EmulatedSerialTransport::EmulatedSerialTransport(const std::shared_ptr<FirmwareEmulatorBackend>& _backend,
                                                 int _read_chunk_size) :
    backend(_backend),
    read_chunk_size(_read_chunk_size) {}

/**
 * @brief Open the in-memory transport.
 * @return always true
 */
bool EmulatedSerialTransport::open() {
    this->open_state = true;
    return true;
}

/**
 * @brief Close the in-memory transport and reset transient state.
 */
void EmulatedSerialTransport::close() {
    this->open_state = false;
    this->write_buffer.clear();
    this->read_buffer.clear();
    this->ready_read_pending = false;
    this->pending_write_kind = PendingWriteKind::None;
    this->pending_address = 0;
    this->pending_size = 0;
}

/**
 * @brief Check whether the transport is open.
 * @return true when open
 */
bool EmulatedSerialTransport::isOpen() const {
    return this->open_state;
}

/**
 * @brief Accept a DTR change from the client.
 * @param ready ignored by the emulator
 */
void EmulatedSerialTransport::setDataTerminalReady(bool) {
}

/**
 * @brief Get the current error string.
 * @return empty string because the emulator has no transport-level errors
 */
std::string EmulatedSerialTransport::errorString() const {
    return std::string();
}

/**
 * @brief Write raw bytes into the transport.
 * @param data bytes to write
 * @param maxSize number of bytes
 * @return number of consumed bytes
 */
qint64 EmulatedSerialTransport::write(const char* data, qint64 maxSize) {
    if(!this->open_state) {
        return -1;
    }

    this->write_buffer.append(data, static_cast<qsizetype>(maxSize));
    this->process_write_buffer();
    return maxSize;
}

/**
 * @brief Write a QByteArray into the transport.
 * @param data bytes to write
 * @return number of consumed bytes
 */
qint64 EmulatedSerialTransport::write(const QByteArray& data) {
    return this->write(data.constData(), data.size());
}

/**
 * @brief Report completion of a write operation.
 * @param ignored by the emulator
 * @return false because writes complete immediately
 */
bool EmulatedSerialTransport::waitForBytesWritten(int) {
    return false;
}

/**
 * @brief Report newly queued response bytes.
 * @param ignored by the emulator
 * @return true once when fresh bytes have arrived
 */
bool EmulatedSerialTransport::waitForReadyRead(int) {
    if(this->ready_read_pending) {
        this->ready_read_pending = false;
        return true;
    }

    return false;
}

/**
 * @brief Return all queued response bytes.
 * @return buffered response bytes
 */
QByteArray EmulatedSerialTransport::readAll() {
    int bytes_to_read = this->read_buffer.size();
    if(this->read_chunk_size > 0) {
        bytes_to_read = std::min(bytes_to_read, this->read_chunk_size);
    }

    const QByteArray data = this->read_buffer.left(bytes_to_read);
    this->read_buffer.remove(0, bytes_to_read);
    if(!this->read_buffer.isEmpty()) {
        this->ready_read_pending = true;
    }
    return data;
}

/**
 * @brief Return the number of queued response bytes.
 * @return available byte count
 */
qint64 EmulatedSerialTransport::bytesAvailable() const {
    if(this->read_chunk_size > 0) {
        return std::min(this->read_buffer.size(), this->read_chunk_size);
    }

    return this->read_buffer.size();
}

/**
 * @brief Parse commands and write payloads from the outgoing byte stream.
 *
 * Each command is processed exactly like the board firmware: emit an 8-byte
 * echo first, then stream the requested payload or wait for an incoming write
 * payload before producing a checksum response.
 */
void EmulatedSerialTransport::process_write_buffer() {
    while(true) {
        if(this->pending_write_kind != PendingWriteKind::None) {
            if(this->write_buffer.size() < this->pending_size) {
                return;
            }

            const QByteArray payload = this->write_buffer.left(this->pending_size);
            this->write_buffer.remove(0, this->pending_size);

            this->backend->writeRange(this->pending_address, payload);

            if(this->pending_write_kind == PendingWriteKind::Block) {
                uint8_t checksum = 0;
                for(char byte : payload) {
                    checksum = static_cast<uint8_t>(checksum + static_cast<uint8_t>(byte));
                }
                this->queue_response(QByteArray(1, static_cast<char>(checksum)));
            } else {
                const uint16_t checksum = crc16_xmodem_emulator(payload, SECTOR_SIZE);
                QByteArray response;
                response.append(static_cast<char>(checksum & 0xFF));
                response.append(static_cast<char>((checksum >> 8) & 0xFF));
                this->queue_response(response);
            }

            this->pending_write_kind = PendingWriteKind::None;
            this->pending_address = 0;
            this->pending_size = 0;
            continue;
        }

        if(this->write_buffer.size() < 8) {
            return;
        }

        const QByteArray command = this->write_buffer.left(8);
        this->write_buffer.remove(0, 8);

        this->backend->recordCommand(command.toStdString());
        this->queue_response(this->backend->commandEcho(command));

        if(command == "READINFO") {
            this->queue_response(this->backend->boardInfo());
        } else if(command == "DEVIDSST") {
            const uint16_t chip_id = this->backend->chipId();
            QByteArray response;
            response.append(static_cast<char>((chip_id >> 8) & 0xFF));
            response.append(static_cast<char>(chip_id & 0xFF));
            this->queue_response(response);
        } else if(command.startsWith("RDBANK")) {
            this->queue_response(this->backend->readRange(parse_hex(command.mid(6, 2)) * BANK_SIZE, BANK_SIZE));
        } else if(command.startsWith("RDBK")) {
            this->queue_response(this->backend->readRange(parse_hex(command.mid(4, 4)) * BLOCK_SIZE, BLOCK_SIZE));
        } else if(command.startsWith("RP2KCR")) {
            this->queue_response(this->backend->readRange(parse_hex(command.mid(6, 2)) * SECTOR_SIZE, SECTOR_SIZE));
        } else if(command.startsWith("RDSECT")) {
            this->queue_response(this->backend->readRange(parse_hex(command.mid(6, 2)) * SECTOR_SIZE, SECTOR_SIZE));
        } else if(command.startsWith("ESST")) {
            const uint32_t block_id = parse_hex(command.mid(4, 4));
            this->backend->eraseRange(block_id * BLOCK_SIZE, SECTOR_SIZE);
            QByteArray response;
            response.append(static_cast<char>(0x00));
            response.append(static_cast<char>(0x01));
            this->queue_response(response);
        } else if(command == "ERASEALL") {
            this->backend->eraseRange(0, FLASH_SIZE);
            QByteArray response;
            response.append(static_cast<char>(0x00));
            response.append(static_cast<char>(0x01));
            this->queue_response(response);
        } else if(command.startsWith("WRBK")) {
            this->pending_write_kind = PendingWriteKind::Block;
            this->pending_address = parse_hex(command.mid(4, 4)) * BLOCK_SIZE;
            this->pending_size = BLOCK_SIZE;
        } else if(command.startsWith("WRSECT")) {
            this->pending_write_kind = PendingWriteKind::Sector;
            this->pending_address = parse_hex(command.mid(6, 2)) * SECTOR_SIZE;
            this->pending_size = SECTOR_SIZE;
        }
    }
}

/**
 * @brief Queue response bytes for the client.
 * @param data response bytes
 */
void EmulatedSerialTransport::queue_response(const QByteArray& data) {
    if(data.isEmpty()) {
        return;
    }
    this->read_buffer.append(data);
    this->ready_read_pending = true;
}

/**
 * @brief Construct a new firmware emulator backend.
 * @param initial_flash optional initial flash contents
 * @param board_id board identification string
 * @param device_id SST39 device id
 */
FirmwareEmulatorBackend::FirmwareEmulatorBackend(const QByteArray& initial_flash,
                                                 const QByteArray& board_id,
                                                 uint16_t device_id) :
    flash(this->normalize_flash(initial_flash, device_id)),
    board_info(this->normalized_board_info(board_id)),
    chip_id(device_id) {}

/**
 * @brief Create a transport wired to this backend.
 * @param read_chunk_size maximum bytes exposed per read; zero for unlimited
 * @return transport instance
 */
std::unique_ptr<SerialTransport> FirmwareEmulatorBackend::create_transport(int read_chunk_size) {
    return std::make_unique<EmulatedSerialTransport>(shared_from_this(), read_chunk_size);
}

/**
 * @brief Get the board identification string.
 * @return 16-byte board info payload
 */
QByteArray FirmwareEmulatorBackend::boardInfo() const {
    std::lock_guard<std::mutex> guard(this->mutex);
    return this->board_info;
}

/**
 * @brief Get the emulated device id.
 * @return chip id
 */
uint16_t FirmwareEmulatorBackend::chipId() const {
    std::lock_guard<std::mutex> guard(this->mutex);
    return this->chip_id;
}

/**
 * @brief Get a snapshot of the emulated flash contents.
 * @return flash image
 */
QByteArray FirmwareEmulatorBackend::flashContents() const {
    std::lock_guard<std::mutex> guard(this->mutex);
    return this->flash;
}

/**
 * @brief Read a range from the emulated chip.
 * @param offset byte offset
 * @param length number of bytes to read
 * @return requested bytes or erased bytes outside the chip size
 */
QByteArray FirmwareEmulatorBackend::readRange(uint32_t offset, int length) const {
    std::lock_guard<std::mutex> guard(this->mutex);

    QByteArray response(length, static_cast<char>(0xFF));
    if(offset >= static_cast<uint32_t>(this->flash.size())) {
        return response;
    }

    const int readable = std::min(length, this->flash.size() - static_cast<int>(offset));
    std::copy_n(this->flash.constData() + offset, readable, response.data());
    return response;
}

/**
 * @brief Get the chronological command history.
 * @return list of commands
 */
std::vector<std::string> FirmwareEmulatorBackend::commandHistory() const {
    std::lock_guard<std::mutex> guard(this->mutex);
    return this->command_history;
}

/**
 * @brief Record a command in the command history.
 * @param command 8-byte command string
 */
void FirmwareEmulatorBackend::recordCommand(const std::string& command) {
    std::lock_guard<std::mutex> guard(this->mutex);
    this->command_history.push_back(command);
}

/**
 * @brief Return the echo for a command.
 * @param command command bytes
 * @return echoed command, optionally corrupted for negative tests
 */
QByteArray FirmwareEmulatorBackend::commandEcho(const QByteArray& command) {
    std::lock_guard<std::mutex> guard(this->mutex);

    QByteArray echo = command;
    if(this->corrupt_next_echo && !echo.isEmpty()) {
        echo[0] = echo[0] == 'X' ? 'Y' : 'X';
        this->corrupt_next_echo = false;
    }

    return echo;
}

/**
 * @brief Corrupt the next command echo.
 */
void FirmwareEmulatorBackend::corruptNextEcho() {
    std::lock_guard<std::mutex> guard(this->mutex);
    this->corrupt_next_echo = true;
}

/**
 * @brief Program bytes into the emulated flash chip.
 * @param offset byte offset
 * @param data bytes to program
 */
void FirmwareEmulatorBackend::writeRange(uint32_t offset, const QByteArray& data) {
    std::lock_guard<std::mutex> guard(this->mutex);
    if(offset >= static_cast<uint32_t>(this->flash.size())) {
        return;
    }

    const int writable = std::min(data.size(), this->flash.size() - static_cast<int>(offset));
    for(int i=0; i<writable; i++) {
        this->flash[static_cast<int>(offset) + i] = static_cast<char>(
            static_cast<uint8_t>(this->flash[static_cast<int>(offset) + i]) &
            static_cast<uint8_t>(data[i])
        );
    }
}

/**
 * @brief Erase a range of the emulated flash chip.
 * @param offset byte offset
 * @param length number of bytes to erase
 */
void FirmwareEmulatorBackend::eraseRange(uint32_t offset, int length) {
    std::lock_guard<std::mutex> guard(this->mutex);
    if(offset >= static_cast<uint32_t>(this->flash.size())) {
        return;
    }

    const int erasable = std::min(length, this->flash.size() - static_cast<int>(offset));
    std::fill_n(this->flash.begin() + static_cast<int>(offset), erasable, static_cast<char>(0xFF));
}

/**
 * @brief Get the flash capacity of the selected chip.
 * @return number of addressable bytes
 */
int FirmwareEmulatorBackend::chipSizeBytes() const {
    return chipSizeBytesForId(this->chip_id);
}

/**
 * @brief Get the flash capacity of a specific chip id.
 * @param device_id SST39 chip id
 * @return number of addressable bytes
 */
int FirmwareEmulatorBackend::chipSizeBytesForId(uint16_t device_id) {
    switch(device_id) {
        case 0xBFB5:
            return 128 * 1024;
        case 0xBFB6:
            return 256 * 1024;
        case 0xBFB7:
        default:
            return FLASH_SIZE;
    }
}

/**
 * @brief Normalize an initial flash image to the supplied chip capacity.
 * @param data initial flash contents
 * @param device_id SST39 chip id
 * @return resized image padded with erased bytes
 */
QByteArray FirmwareEmulatorBackend::normalize_flash(const QByteArray& data, uint16_t device_id) const {
    const int chip_size = chipSizeBytesForId(device_id);
    QByteArray normalized = data.left(chip_size);
    if(normalized.size() < chip_size) {
        normalized.append(QByteArray(chip_size - normalized.size(), static_cast<char>(0xFF)));
    }
    return normalized;
}

/**
 * @brief Normalize the board information string.
 * @param data source board info
 * @return 16-byte payload
 */
QByteArray FirmwareEmulatorBackend::normalized_board_info(const QByteArray& data) const {
    QByteArray normalized = data.left(16);
    if(normalized.size() < 16) {
        normalized.append(QByteArray(16 - normalized.size(), '\0'));
    }
    return normalized;
}

/**
 * @brief Calculate a CRC16 XMODEM checksum.
 * @param data input bytes
 * @param length number of bytes
 * @return checksum
 */
uint16_t crc16_xmodem_emulator(const QByteArray& data, uint16_t length) {
    uint32_t crc = 0;
    static const uint16_t poly = 0x1021;

    for(uint16_t i=0; i<length; i++) {
        crc = crc ^ (static_cast<uint8_t>(data[i]) << 8);
        for(uint8_t j=0; j<8; j++) {
            crc = crc << 1;
            if(crc & 0x10000) {
                crc = (crc ^ poly) & 0xFFFF;
            }
        }
    }

    return static_cast<uint16_t>(crc);
}
