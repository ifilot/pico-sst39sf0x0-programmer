#include <QtTest/QtTest>

#include "cartridgereadthread.h"
#include "flashthread.h"
#include "readthread.h"
#include "support/emulated_serial_transport.h"

#include <QSignalSpy>

#include <memory>

class WorkerThreadTest : public QObject {
    Q_OBJECT

private:
    static SerialInterface::TransportFactory buildFactory(const std::shared_ptr<FirmwareEmulatorBackend>& backend) {
        return [backend](const std::string&) {
            return backend->create_transport();
        };
    }

private slots:
    void read_thread_reads_all_banks_for_detected_chip();
    void cartridge_read_thread_reads_all_segments();
    void flash_thread_quickflash_writes_only_non_empty_sectors();
    void flash_thread_regular_mode_erases_and_writes_each_sector();
    void worker_threads_surface_serial_errors();
};

void WorkerThreadTest::read_thread_reads_all_banks_for_detected_chip() {
    QByteArray flash(0x80000, static_cast<char>(0xFF));
    for(int i=0; i<0x20000; i++) {
        flash[i] = static_cast<char>(i & 0xFF);
    }

    auto backend = std::make_shared<FirmwareEmulatorBackend>(flash, QByteArray("PICOSST39-V1.3.0"), 0xBFB5);
    auto serial = std::make_shared<SerialInterface>("emu", buildFactory(backend));
    ReadThread thread(serial);

    QSignalSpy readySpy(&thread, SIGNAL(read_result_ready()));
    QSignalSpy abortSpy(&thread, SIGNAL(thread_abort(const QString&)));
    QSignalSpy blockSpy(&thread, SIGNAL(read_block_done(uint,uint)));

    thread.start();
    QVERIFY(thread.wait(2000));

    QCOMPARE(readySpy.count(), 1);
    QCOMPARE(abortSpy.count(), 0);
    QCOMPARE(blockSpy.count(), 8);
    QCOMPARE(thread.get_data().size(), 0x20000);
    QCOMPARE(thread.get_data().left(64), flash.left(64));

    const auto history = backend->commandHistory();
    QCOMPARE(QString::fromStdString(history.front()), QString("DEVIDSST"));
    QCOMPARE(QString::fromStdString(history.back()), QString("RDBANK07"));
}

void WorkerThreadTest::cartridge_read_thread_reads_all_segments() {
    QByteArray flash(0x80000, static_cast<char>(0xFF));
    for(int i=0; i<0x4000; i++) {
        flash[i] = static_cast<char>((i * 7) & 0xFF);
    }

    auto backend = std::make_shared<FirmwareEmulatorBackend>(flash);
    auto serial = std::make_shared<SerialInterface>("emu", buildFactory(backend));
    CartridgeReadThread thread(serial);

    QSignalSpy readySpy(&thread, SIGNAL(read_result_ready()));
    QSignalSpy abortSpy(&thread, SIGNAL(thread_abort(const QString&)));

    thread.start();
    QVERIFY(thread.wait(2000));

    QCOMPARE(readySpy.count(), 1);
    QCOMPARE(abortSpy.count(), 0);
    QCOMPARE(thread.get_data().size(), 0x4000);
    QCOMPARE(thread.get_data(), flash.left(0x4000));

    const auto history = backend->commandHistory();
    QVERIFY(history == std::vector<std::string>({"RP2KCR00", "RP2KCR01", "RP2KCR02", "RP2KCR03"}));
}

void WorkerThreadTest::flash_thread_quickflash_writes_only_non_empty_sectors() {
    auto backend = std::make_shared<FirmwareEmulatorBackend>();
    auto serial = std::make_shared<SerialInterface>("emu", buildFactory(backend));
    FlashThread thread(serial, 0, true);

    QByteArray data(0x2000, static_cast<char>(0xFF));
    for(int i=0; i<0x1000; i++) {
        data[i] = static_cast<char>((255 - i) & 0xFF);
    }
    thread.set_data(data);

    QSignalSpy readySpy(&thread, SIGNAL(flash_result_ready()));
    QSignalSpy abortSpy(&thread, SIGNAL(thread_abort(const QString&)));

    thread.start();
    QVERIFY(thread.wait(2000));

    QCOMPARE(readySpy.count(), 1);
    QCOMPARE(abortSpy.count(), 0);

    const QByteArray flash = backend->flashContents();
    QCOMPARE(flash.mid(0, 0x1000), data.left(0x1000));
    QCOMPARE(flash.mid(0x1000, 0x1000), QByteArray(0x1000, static_cast<char>(0xFF)));

    const auto history = backend->commandHistory();
    QVERIFY(history == std::vector<std::string>({"DEVIDSST", "ERASEALL", "WRSECT00"}));
}

void WorkerThreadTest::flash_thread_regular_mode_erases_and_writes_each_sector() {
    auto backend = std::make_shared<FirmwareEmulatorBackend>(QByteArray(0x80000, static_cast<char>(0x00)));
    auto serial = std::make_shared<SerialInterface>("emu", buildFactory(backend));
    FlashThread thread(serial, 0, false);

    QByteArray data(0x2000, static_cast<char>(0xA5));
    thread.set_data(data);

    QSignalSpy readySpy(&thread, SIGNAL(flash_result_ready()));
    QSignalSpy abortSpy(&thread, SIGNAL(thread_abort(const QString&)));

    thread.start();
    QVERIFY(thread.wait(2000));

    QCOMPARE(readySpy.count(), 1);
    QCOMPARE(abortSpy.count(), 0);
    QCOMPARE(backend->flashContents().mid(0, 0x2000), data);

    const auto history = backend->commandHistory();
    QVERIFY(history == std::vector<std::string>({
        "DEVIDSST", "ESST0000", "WRSECT00", "ESST0010", "WRSECT01"
    }));
}

void WorkerThreadTest::worker_threads_surface_serial_errors() {
    {
        auto serial = std::make_shared<SerialInterface>("");
        ReadThread thread(serial);
        QSignalSpy readySpy(&thread, SIGNAL(read_result_ready()));
        QSignalSpy abortSpy(&thread, SIGNAL(thread_abort(const QString&)));

        thread.start();
        QVERIFY(thread.wait(2000));
        QCOMPARE(readySpy.count(), 0);
        QCOMPARE(abortSpy.count(), 1);
        QVERIFY(abortSpy.takeFirst().at(0).toString().contains("No port has been set"));
    }

    {
        auto serial = std::make_shared<SerialInterface>("");
        CartridgeReadThread thread(serial);
        QSignalSpy readySpy(&thread, SIGNAL(read_result_ready()));
        QSignalSpy abortSpy(&thread, SIGNAL(thread_abort(const QString&)));

        thread.start();
        QVERIFY(thread.wait(2000));
        QCOMPARE(readySpy.count(), 0);
        QCOMPARE(abortSpy.count(), 1);
        QVERIFY(abortSpy.takeFirst().at(0).toString().contains("No port has been set"));
    }

    {
        auto serial = std::make_shared<SerialInterface>("");
        FlashThread thread(serial, 0, true);
        QSignalSpy readySpy(&thread, SIGNAL(flash_result_ready()));
        QSignalSpy abortSpy(&thread, SIGNAL(thread_abort(const QString&)));

        thread.start();
        QVERIFY(thread.wait(2000));
        QCOMPARE(readySpy.count(), 0);
        QCOMPARE(abortSpy.count(), 1);
        QVERIFY(abortSpy.takeFirst().at(0).toString().contains("No port has been set"));
    }
}

QTEST_MAIN(WorkerThreadTest)

#include "tst_worker_threads.moc"
