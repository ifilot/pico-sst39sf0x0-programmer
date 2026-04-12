#include <QtTest/QtTest>

#include "serial_interface.h"
#include "support/fault_injecting_transport.h"
#include "support/emulated_serial_transport.h"

#include <memory>
#include <stdexcept>

class SerialInterfaceTest : public QObject {
    Q_OBJECT

private:
    static SerialInterface::TransportFactory buildFactory(const std::shared_ptr<FirmwareEmulatorBackend>& backend) {
        return [backend](const std::string&) {
            return backend->create_transport();
        };
    }

    static SerialInterface::TransportFactory buildFaultFactory(const std::shared_ptr<FirmwareEmulatorBackend>& backend,
                                                               const std::string& targetCommand,
                                                               FaultInjectingTransport::FaultMode mode) {
        return [backend, targetCommand, mode](const std::string&) {
            return std::make_unique<FaultInjectingTransport>(
                backend->create_transport(),
                targetCommand,
                mode
            );
        };
    }

private slots:
    void reads_board_info();
    void reads_chip_id();
    void reads_bank_payload();
    void reads_cartridge_segment();
    void writes_sector_and_updates_emulated_flash();
    void erases_chip_clears_flash();
    void erases_target_sector_only();
    void full_program_erase_read_workflow_matches_flash_behavior();
    void smaller_chip_out_of_range_reads_as_erased_flash();
    void write_without_erase_cannot_restore_bits();
    void rejects_invalid_block_checksum();
    void rejects_invalid_sector_checksum();
    void times_out_when_device_stops_responding();
    void rejects_invalid_echo();
};

void SerialInterfaceTest::reads_board_info() {
    auto backend = std::make_shared<FirmwareEmulatorBackend>();
    SerialInterface serial("emu", buildFactory(backend));

    serial.open_port();
    const std::string boardInfo = serial.get_board_info();
    serial.close_port();

    QCOMPARE(QString::fromStdString(boardInfo), QString("PICOSST39-V1.3.0"));
    QVERIFY(backend->commandHistory() == std::vector<std::string>{"READINFO"});
}

void SerialInterfaceTest::reads_chip_id() {
    auto backend = std::make_shared<FirmwareEmulatorBackend>(QByteArray(), QByteArray("PICOSST39-V1.3.0"), 0xBFB6);
    SerialInterface serial("emu", buildFactory(backend));

    serial.open_port();
    const uint16_t chipId = serial.get_chip_id();
    serial.close_port();

    QCOMPARE(chipId, static_cast<uint16_t>(0xBFB6));
}

void SerialInterfaceTest::reads_bank_payload() {
    QByteArray flash(0x80000, '\0');
    for(int i=0; i<0x4000; i++) {
        flash[i] = static_cast<char>(i & 0xFF);
    }

    auto backend = std::make_shared<FirmwareEmulatorBackend>(flash);
    SerialInterface serial("emu", buildFactory(backend));

    serial.open_port();
    const QByteArray payload = serial.read_bank(0);
    serial.close_port();

    QCOMPARE(payload.size(), 0x4000);
    QCOMPARE(payload.left(32), flash.left(32));
    QVERIFY(backend->commandHistory() == std::vector<std::string>{"RDBANK00"});
}

void SerialInterfaceTest::reads_cartridge_segment() {
    QByteArray flash(0x80000, static_cast<char>(0xFF));
    for(int i=0; i<0x1000; i++) {
        flash[0x2000 + i] = static_cast<char>((i * 5) & 0xFF);
    }

    auto backend = std::make_shared<FirmwareEmulatorBackend>(flash);
    SerialInterface serial("emu", buildFactory(backend));

    serial.open_port();
    const QByteArray payload = serial.read_segment_cartridge(2);
    serial.close_port();

    QCOMPARE(payload, flash.mid(0x2000, 0x1000));
}

void SerialInterfaceTest::writes_sector_and_updates_emulated_flash() {
    auto backend = std::make_shared<FirmwareEmulatorBackend>();
    SerialInterface serial("emu", buildFactory(backend));

    QByteArray sector(0x1000, '\0');
    for(int i=0; i<sector.size(); i++) {
        sector[i] = static_cast<char>((i * 3) & 0xFF);
    }

    serial.open_port();
    serial.burn_sector(2, sector);
    serial.close_port();

    const QByteArray flash = backend->flashContents();
    QCOMPARE(flash.mid(0x2000, 0x1000), sector);
    QVERIFY(backend->commandHistory() == std::vector<std::string>{"WRSECT02"});
}

void SerialInterfaceTest::erases_chip_clears_flash() {
    QByteArray flash(0x80000, static_cast<char>(0x00));
    auto backend = std::make_shared<FirmwareEmulatorBackend>(flash);
    SerialInterface serial("emu", buildFactory(backend));

    serial.open_port();
    serial.erase_chip();
    serial.close_port();

    QCOMPARE(backend->flashContents(), QByteArray(0x80000, static_cast<char>(0xFF)));
}

void SerialInterfaceTest::erases_target_sector_only() {
    QByteArray flash(0x80000, static_cast<char>(0x00));
    auto backend = std::make_shared<FirmwareEmulatorBackend>(flash);
    SerialInterface serial("emu", buildFactory(backend));

    serial.open_port();
    serial.erase_sector(0x0010);
    serial.close_port();

    const QByteArray image = backend->flashContents();
    QCOMPARE(image.mid(0x0000, 0x1000), QByteArray(0x1000, static_cast<char>(0x00)));
    QCOMPARE(image.mid(0x1000, 0x1000), QByteArray(0x1000, static_cast<char>(0xFF)));
    QCOMPARE(image.mid(0x2000, 0x1000), QByteArray(0x1000, static_cast<char>(0x00)));
}

void SerialInterfaceTest::full_program_erase_read_workflow_matches_flash_behavior() {
    auto backend = std::make_shared<FirmwareEmulatorBackend>();
    SerialInterface serial("emu", buildFactory(backend));

    QByteArray sector0(0x1000, static_cast<char>(0xAA));
    QByteArray sector1(0x1000, static_cast<char>(0x55));

    serial.open_port();
    serial.erase_chip();
    serial.burn_sector(0, sector0);
    serial.burn_sector(1, sector1);
    const QByteArray bankBeforeErase = serial.read_bank(0);
    serial.erase_sector(0x0010);
    const QByteArray bankAfterErase = serial.read_bank(0);
    serial.close_port();

    QCOMPARE(bankBeforeErase.left(0x1000), sector0);
    QCOMPARE(bankBeforeErase.mid(0x1000, 0x1000), sector1);
    QCOMPARE(bankAfterErase.left(0x1000), sector0);
    QCOMPARE(bankAfterErase.mid(0x1000, 0x1000), QByteArray(0x1000, static_cast<char>(0xFF)));

    QVERIFY(backend->commandHistory() == std::vector<std::string>({
        "ERASEALL", "WRSECT00", "WRSECT01", "RDBANK00", "ESST0010", "RDBANK00"
    }));
}

void SerialInterfaceTest::smaller_chip_out_of_range_reads_as_erased_flash() {
    QByteArray flash(128 * 1024, static_cast<char>(0x11));
    auto backend = std::make_shared<FirmwareEmulatorBackend>(flash, QByteArray("PICOSST39-V1.3.0"), 0xBFB5);
    SerialInterface serial("emu", buildFactory(backend));

    serial.open_port();
    const QByteArray validBank = serial.read_bank(7);
    const QByteArray outOfRangeBank = serial.read_bank(8);
    serial.close_port();

    QCOMPARE(validBank, QByteArray(0x4000, static_cast<char>(0x11)));
    QCOMPARE(outOfRangeBank, QByteArray(0x4000, static_cast<char>(0xFF)));
}

void SerialInterfaceTest::write_without_erase_cannot_restore_bits() {
    auto backend = std::make_shared<FirmwareEmulatorBackend>();
    SerialInterface serial("emu", buildFactory(backend));

    QByteArray blockLow(0x100, static_cast<char>(0x00));
    QByteArray blockHigh(0x100, static_cast<char>(0xFF));

    serial.open_port();
    serial.burn_block(0, blockLow);
    serial.burn_block(0, blockHigh);
    const QByteArray payload = serial.read_block(0);
    serial.close_port();

    QCOMPARE(payload, blockLow);
}

void SerialInterfaceTest::rejects_invalid_block_checksum() {
    auto backend = std::make_shared<FirmwareEmulatorBackend>();
    SerialInterface serial("emu", buildFaultFactory(backend, "WRBK0000", FaultInjectingTransport::FaultMode::CorruptBlockChecksum));

    QByteArray block(0x100, static_cast<char>(0x12));

    serial.open_port();
    QVERIFY_EXCEPTION_THROWN(serial.burn_block(0, block), std::runtime_error);
    serial.close_port();
}

void SerialInterfaceTest::rejects_invalid_sector_checksum() {
    auto backend = std::make_shared<FirmwareEmulatorBackend>();
    SerialInterface serial("emu", buildFaultFactory(backend, "WRSECT00", FaultInjectingTransport::FaultMode::CorruptSectorChecksum));

    QByteArray sector(0x1000, static_cast<char>(0x34));

    serial.open_port();
    QVERIFY_EXCEPTION_THROWN(serial.burn_sector(0, sector), std::runtime_error);
    serial.close_port();
}

void SerialInterfaceTest::times_out_when_device_stops_responding() {
    auto backend = std::make_shared<FirmwareEmulatorBackend>();
    SerialInterface serial("emu", buildFaultFactory(backend, "READINFO", FaultInjectingTransport::FaultMode::SuppressResponse));

    serial.open_port();
    QVERIFY_EXCEPTION_THROWN(serial.get_board_info(), std::runtime_error);
    serial.close_port();
}

void SerialInterfaceTest::rejects_invalid_echo() {
    auto backend = std::make_shared<FirmwareEmulatorBackend>();
    SerialInterface serial("emu", buildFactory(backend));

    backend->corruptNextEcho();

    serial.open_port();
    QVERIFY_EXCEPTION_THROWN(serial.get_board_info(), std::runtime_error);
    serial.close_port();
}

QTEST_MAIN(SerialInterfaceTest)

#include "tst_serial_interface.moc"
