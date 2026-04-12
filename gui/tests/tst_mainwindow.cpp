#include <QtTest/QtTest>

#include "mainwindow.h"
#include "support/emulated_serial_transport.h"
#include "support/fault_injecting_transport.h"

#include <QComboBox>
#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QSet>
#include <QTimer>

#include <memory>

namespace {

class ModalDialogWatcher : public QObject {
public:
    QStringList messages;

    ModalDialogWatcher() {
        this->timer.setInterval(10);
        connect(&this->timer, &QTimer::timeout, this, &ModalDialogWatcher::captureAndAcceptDialogs);
        this->timer.start();
    }

    ~ModalDialogWatcher() override {
        this->timer.stop();
    }

private:
    QTimer timer;
    QSet<QWidget*> handled;

private:
    void captureAndAcceptDialogs() {
        const auto widgets = QApplication::topLevelWidgets();
        for(QWidget* widget : widgets) {
            if(this->handled.contains(widget)) {
                continue;
            }

            if(auto* box = qobject_cast<QMessageBox*>(widget)) {
                this->messages.append(box->text());
                this->handled.insert(widget);
                QTimer::singleShot(0, box, &QMessageBox::accept);
            }
        }
    }
};

}

class MainWindowTest : public QObject {
    Q_OBJECT

private:
    static MainWindow::SerialInterfaceFactory buildFactory(const std::shared_ptr<FirmwareEmulatorBackend>& backend) {
        return [backend](const std::string& portname) {
            return std::make_shared<SerialInterface>(portname, [backend](const std::string&) {
                return backend->create_transport();
            });
        };
    }

    static MainWindow::SerialInterfaceFactory buildFaultFactory(const std::shared_ptr<FirmwareEmulatorBackend>& backend,
                                                                const std::string& targetCommand,
                                                                FaultInjectingTransport::FaultMode mode) {
        return [backend, targetCommand, mode](const std::string& portname) {
            return std::make_shared<SerialInterface>(portname, [backend, targetCommand, mode](const std::string&) {
                return std::make_unique<FaultInjectingTransport>(
                    backend->create_transport(),
                    targetCommand,
                    mode
                );
            });
        };
    }

    static QComboBox* serialPortCombo(MainWindow& window) {
        return window.findChild<QComboBox*>("comboboxSerialPorts");
    }

    static QLabel* boardIdLabel(MainWindow& window) {
        return window.findChild<QLabel*>("labelBoardId");
    }

    static QLabel* serialLabel(MainWindow& window) {
        return window.findChild<QLabel*>("labelSerial");
    }

    static QLabel* chipTypeLabel(MainWindow& window) {
        return window.findChild<QLabel*>("labelChipType");
    }

    static QPushButton* button(MainWindow& window, const char* objectName) {
        return window.findChild<QPushButton*>(objectName);
    }

    static QProgressBar* progressBar(MainWindow& window) {
        return window.findChild<QProgressBar*>("progressBarLoad");
    }

    static HexViewWidget* hexView(MainWindow& window) {
        return window.findChild<HexViewWidget*>("hexViewWidget");
    }

    static void addEmulatedPort(MainWindow& window, const QString& portName = "emu") {
        auto* combo = serialPortCombo(window);
        QVERIFY(combo != nullptr);
        combo->addItem(portName);
        combo->setCurrentText(portName);
    }

private slots:
    void select_com_port_updates_labels_and_enables_buttons();
    void read_chip_id_updates_ui_state();
    void read_rom_populates_hex_view();
    void flash_rom_quick_runs_program_and_verify_workflow();
    void flash_rom_quick_worker_abort_is_reported();
};

void MainWindowTest::select_com_port_updates_labels_and_enables_buttons() {
    auto backend = std::make_shared<FirmwareEmulatorBackend>();
    auto logs = std::make_shared<QStringList>();
    MainWindow window(logs, nullptr, buildFactory(backend));

    addEmulatedPort(window);

    QVERIFY(QMetaObject::invokeMethod(&window, "select_com_port", Qt::DirectConnection));

    QCOMPARE(serialLabel(window)->text(), QString("Port: emu"));
    QVERIFY(boardIdLabel(window)->text().contains("PICOSST39-V1.3.0"));
    QVERIFY(button(window, "buttonIdentifyChip")->isEnabled());
    QVERIFY(button(window, "buttonReadCartridge")->isEnabled());
}

void MainWindowTest::read_chip_id_updates_ui_state() {
    auto backend = std::make_shared<FirmwareEmulatorBackend>(QByteArray(), QByteArray("PICOSST39-V1.3.0"), 0xBFB6);
    auto logs = std::make_shared<QStringList>();
    MainWindow window(logs, nullptr, buildFactory(backend));

    addEmulatedPort(window);
    QVERIFY(QMetaObject::invokeMethod(&window, "select_com_port", Qt::DirectConnection));
    QVERIFY(QMetaObject::invokeMethod(&window, "read_chip_id", Qt::DirectConnection));

    QCOMPARE(chipTypeLabel(window)->text(), QString("ROM chip: SST39SF020"));
    QCOMPARE(progressBar(window)->maximum(), 256 * 1024 / 256);
    QVERIFY(button(window, "buttonReadRom")->isEnabled());
    QVERIFY(button(window, "buttonFlashRom")->isEnabled());
    QVERIFY(button(window, "buttonFlashBank")->isEnabled());
    QVERIFY(button(window, "buttonEraseChip")->isEnabled());
    QVERIFY(button(window, "buttonScanSlots")->isEnabled());
}

void MainWindowTest::read_rom_populates_hex_view() {
    QByteArray flash(128 * 1024, static_cast<char>(0xFF));
    for(int i=0; i<flash.size(); i++) {
        flash[i] = static_cast<char>(i & 0xFF);
    }

    auto backend = std::make_shared<FirmwareEmulatorBackend>(flash, QByteArray("PICOSST39-V1.3.0"), 0xBFB5);
    auto logs = std::make_shared<QStringList>();
    MainWindow window(logs, nullptr, buildFactory(backend));

    addEmulatedPort(window);
    QVERIFY(QMetaObject::invokeMethod(&window, "select_com_port", Qt::DirectConnection));
    QVERIFY(QMetaObject::invokeMethod(&window, "read_chip_id", Qt::DirectConnection));
    QVERIFY(QMetaObject::invokeMethod(&window, "read_rom", Qt::DirectConnection));

    QTRY_COMPARE(hexView(window)->get_data().size(), flash.size());
    QCOMPARE(hexView(window)->get_data().left(256), flash.left(256));
    QVERIFY(window.statusBar()->currentMessage().contains("Done reading chip"));
}

void MainWindowTest::flash_rom_quick_runs_program_and_verify_workflow() {
    auto backend = std::make_shared<FirmwareEmulatorBackend>(QByteArray(), QByteArray("PICOSST39-V1.3.0"), 0xBFB5);
    auto logs = std::make_shared<QStringList>();
    MainWindow window(logs, nullptr, buildFactory(backend));
    ModalDialogWatcher watcher;

    addEmulatedPort(window);
    QVERIFY(QMetaObject::invokeMethod(&window, "select_com_port", Qt::DirectConnection));
    QVERIFY(QMetaObject::invokeMethod(&window, "read_chip_id", Qt::DirectConnection));

    QByteArray bankData(0x4000, static_cast<char>(0xFF));
    for(int i=0; i<bankData.size(); i++) {
        bankData[i] = static_cast<char>((i * 9) & 0xFF);
    }
    hexView(window)->set_data(bankData);

    QVERIFY(QMetaObject::invokeMethod(&window, "flash_rom_quick", Qt::DirectConnection));

    QTRY_VERIFY(watcher.messages.join("\n").contains("Cartridge was successfully flashed"));
    QCOMPARE(backend->flashContents().mid(0, bankData.size()), bankData);
    QTRY_VERIFY(progressBar(window)->value() <= 0);
}

void MainWindowTest::flash_rom_quick_worker_abort_is_reported() {
    auto backend = std::make_shared<FirmwareEmulatorBackend>(QByteArray(), QByteArray("PICOSST39-V1.3.0"), 0xBFB5);
    auto logs = std::make_shared<QStringList>();
    MainWindow window(logs, nullptr, buildFaultFactory(backend, "WRSECT00", FaultInjectingTransport::FaultMode::CorruptEcho));
    ModalDialogWatcher watcher;

    addEmulatedPort(window);
    QVERIFY(QMetaObject::invokeMethod(&window, "select_com_port", Qt::DirectConnection));
    QVERIFY(QMetaObject::invokeMethod(&window, "read_chip_id", Qt::DirectConnection));

    QByteArray bankData(0x4000, static_cast<char>(0xAA));
    hexView(window)->set_data(bankData);

    QVERIFY(QMetaObject::invokeMethod(&window, "flash_rom_quick", Qt::DirectConnection));

    QTRY_VERIFY(watcher.messages.join("\n").contains("Operation terminated unexpectedly"));
}

QTEST_MAIN(MainWindowTest)

#include "tst_mainwindow.moc"
