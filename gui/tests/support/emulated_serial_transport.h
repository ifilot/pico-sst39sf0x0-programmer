#ifndef EMULATED_SERIAL_TRANSPORT_H
#define EMULATED_SERIAL_TRANSPORT_H

#include "serial_transport.h"

#include <QByteArray>

#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <cstdint>

/**
 * @brief Stateful flash backend used by the emulated serial transport.
 */
class FirmwareEmulatorBackend;

/**
 * @brief In-memory transport that mimics the board's USB CDC command protocol.
 *
 * The transport consumes exactly the same byte stream as the GUI sends to the
 * programmer firmware: 8-byte ASCII commands followed by optional raw payload
 * bytes for write operations.
 */
class EmulatedSerialTransport : public SerialTransport {
private:
    std::shared_ptr<FirmwareEmulatorBackend> backend;
    bool open_state = false;
    QByteArray write_buffer;
    QByteArray read_buffer;
    bool ready_read_pending = false;
    int read_chunk_size = 0;

    /**
     * @brief Type of write payload the transport is currently collecting.
     */
    enum class PendingWriteKind {
        None,
        Block,
        Sector
    };

    PendingWriteKind pending_write_kind = PendingWriteKind::None;
    uint32_t pending_address = 0;
    int pending_size = 0;

    /**
     * @brief Parse pending bytes and emit protocol responses.
     */
    void process_write_buffer();

    /**
     * @brief Append bytes to the receive buffer and mark them as newly arrived.
     * @param data response bytes
     */
    void queue_response(const QByteArray& data);

public:
    /**
     * @brief Construct a new emulated transport.
     * @param _backend shared flash backend
     * @param _read_chunk_size maximum bytes exposed by one readAll() call;
     *        zero exposes the complete buffered response
     */
    explicit EmulatedSerialTransport(const std::shared_ptr<FirmwareEmulatorBackend>& _backend,
                                     int _read_chunk_size = 0);

    /**
     * @brief Open the emulated transport.
     * @return always true
     */
    bool open() override;

    /**
     * @brief Close the emulated transport and clear transient buffers.
     */
    void close() override;

    /**
     * @brief Check whether the emulated transport is open.
     * @return true when open
     */
    bool isOpen() const override;

    /**
     * @brief Accept a DTR change from the client.
     * @param ready ignored by the emulator
     */
    void setDataTerminalReady(bool ready) override;

    /**
     * @brief Get the last transport error string.
     * @return empty string because the emulator is in-memory
     */
    std::string errorString() const override;

    /**
     * @brief Write raw bytes into the emulator.
     * @param data bytes to write
     * @param maxSize number of bytes
     * @return number of consumed bytes
     */
    qint64 write(const char* data, qint64 maxSize) override;

    /**
     * @brief Write a QByteArray into the emulator.
     * @param data bytes to write
     * @return number of consumed bytes
     */
    qint64 write(const QByteArray& data) override;

    /**
     * @brief Wait for bytes to be written.
     * @param msecs ignored by the emulator
     * @return false because writes are immediate
     */
    bool waitForBytesWritten(int msecs) override;

    /**
     * @brief Report whether newly queued read bytes are available.
     * @param msecs ignored by the emulator
     * @return true once for each newly queued response
     */
    bool waitForReadyRead(int msecs) override;

    /**
     * @brief Read all queued response bytes.
     * @return buffered response data
     */
    QByteArray readAll() override;

    /**
     * @brief Return the number of queued response bytes.
     * @return available byte count
     */
    qint64 bytesAvailable() const override;
};

/**
 * @brief Shared state backing the board emulator and its fictitious flash chip.
 *
 * The backend emulates the flash contents and the programming restrictions of
 * SST39SF010/020/040 devices. Data is stored in a chip-sized array and write
 * operations can only transition bits from 1 to 0 until an erase resets them.
 */
class FirmwareEmulatorBackend : public std::enable_shared_from_this<FirmwareEmulatorBackend> {
private:
    mutable std::mutex mutex;
    QByteArray flash;
    QByteArray board_info;
    uint16_t chip_id = 0xBFB7;
    std::vector<std::string> command_history;
    bool corrupt_next_echo = false;

    /**
     * @brief Determine the emulated flash size from the configured chip id.
     * @return number of bytes addressable by the emulated chip
     */
    int chipSizeBytes() const;

    /**
     * @brief Determine the flash size for a specific chip id.
     * @param device_id SST39 chip id
     * @return number of bytes addressable by the emulated chip
     */
    static int chipSizeBytesForId(uint16_t device_id);

    /**
     * @brief Normalize the initial flash image to the active chip capacity.
     * @param data initial flash bytes
     * @return resized image filled with erased bytes
     */
    QByteArray normalize_flash(const QByteArray& data, uint16_t device_id) const;

    /**
     * @brief Normalize the board information string to the firmware's 16-byte response.
     * @param data board id bytes
     * @return 16-byte board identification payload
     */
    QByteArray normalized_board_info(const QByteArray& data) const;

public:
    /**
     * @brief Construct a new firmware emulator backend.
     * @param initial_flash optional initial flash contents
     * @param board_id 16-byte board identification string
     * @param device_id SST39 chip identifier to emulate
     */
    explicit FirmwareEmulatorBackend(const QByteArray& initial_flash = QByteArray(),
                                     const QByteArray& board_id = QByteArray("PICOSST39-V1.3.0"),
                                     uint16_t device_id = 0xBFB7);

    /**
     * @brief Create a new transport connected to this backend.
     * @param read_chunk_size maximum bytes exposed per read; zero for unlimited
     * @return transport instance
     */
    std::unique_ptr<SerialTransport> create_transport(int read_chunk_size = 0);

    /**
     * @brief Get the 16-byte board identification string.
     * @return board info payload
     */
    QByteArray boardInfo() const;

    /**
     * @brief Get the emulated SST39 device id.
     * @return chip id
     */
    uint16_t chipId() const;

    /**
     * @brief Get a snapshot of the full flash contents.
     * @return flash image
     */
    QByteArray flashContents() const;

    /**
     * @brief Read a range from the emulated chip.
     * @param offset byte offset
     * @param length number of bytes to read
     * @return range contents padded with erased bytes out of bounds
     */
    QByteArray readRange(uint32_t offset, int length) const;

    /**
     * @brief Return the recorded command history.
     * @return chronological list of 8-byte commands
     */
    std::vector<std::string> commandHistory() const;

    /**
     * @brief Record a command received by the board emulator.
     * @param command 8-byte command string
     */
    void recordCommand(const std::string& command);

    /**
     * @brief Return the command echo that should be observed by the client.
     * @param command 8-byte command string
     * @return echoed command, optionally corrupted for negative tests
     */
    QByteArray commandEcho(const QByteArray& command);

    /**
     * @brief Corrupt the next echoed command for negative-path tests.
     */
    void corruptNextEcho();

    /**
     * @brief Program data into the emulated chip.
     * @param offset byte offset
     * @param data bytes to program
     *
     * Programming follows real flash semantics: bits may only change from 1 to
     * 0. Any attempt to set a bit back to 1 without erasing first is ignored.
     */
    void writeRange(uint32_t offset, const QByteArray& data);

    /**
     * @brief Erase a range to 0xFF.
     * @param offset byte offset
     * @param length number of bytes to erase
     */
    void eraseRange(uint32_t offset, int length);
};

/**
 * @brief Calculate the CRC16 XMODEM checksum used by sector writes.
 * @param data input bytes
 * @param length number of bytes
 * @return CRC16 checksum
 */
uint16_t crc16_xmodem_emulator(const QByteArray& data, uint16_t length);

#endif // EMULATED_SERIAL_TRANSPORT_H
