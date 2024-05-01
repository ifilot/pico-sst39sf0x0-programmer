/****************************************************************************
 *                                                                          *
 *   PICO-SST39SF0x0-FLASHER                                                *
 *   Copyright (C) 2023 Ivo Filot <ivo@ivofilot.nl>                         *
 *                                                                          *
 *   This program is free software: you can redistribute it and/or modify   *
 *   it under the terms of the GNU Lesser General Public License as         *
 *   published by the Free Software Foundation, either version 3 of the     *
 *   License, or (at your option) any later version.                        *
 *                                                                          *
 *   This program is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU General Public License for more details.                           *
 *                                                                          *
 *   You should have received a copy of the GNU General Public license      *
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>. *
 *                                                                          *
 ****************************************************************************/

#include "mainwindow.h"

/**
 * @brief MainWindow
 * @param parent
 */
MainWindow::MainWindow(const std::shared_ptr<QStringList> _log_messages, QWidget *parent)
    : QMainWindow(parent),
      log_messages(_log_messages) {

    // log window
    this->log_window = std::make_unique<LogWindow>(this->log_messages);

    // settings widget
    this->settings_widget = std::make_unique<SettingsWidget>();
    connect(this->settings_widget.get(), SIGNAL(signal_settings_update()), this, SLOT(slot_update_settings()));

    // complete Window
    QWidget* container = new QWidget();
    this->setCentralWidget(container);
    QHBoxLayout* layout = new QHBoxLayout();
    container->setLayout(layout);

    // add hex editor widget (left side)
    QWidget* container_widget = new QWidget();
    QVBoxLayout* container_layout = new QVBoxLayout();
    container_widget->setLayout(container_layout);
    layout->addWidget(container_widget);
    this->label_data_descriptor = new QLabel();
    container_layout->addWidget(this->label_data_descriptor);
    this->hex_widget = new HexViewWidget();
    this->hex_widget->setMinimumWidth(580);
    this->hex_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    container_layout->addWidget(this->hex_widget);

    this->button_reload_file = new QPushButton("Reload file");
    container_layout->addWidget(this->button_reload_file);
    this->button_reload_file->setEnabled(false);
    connect(this->button_reload_file, SIGNAL(released()), this, SLOT(slot_reload_file()));

    // create central widget for writing data
    QScrollArea *scroll_area = new QScrollArea();
    layout->addWidget(scroll_area);
    scroll_area->setMinimumWidth(360);
    scroll_area->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    QWidget* right_container = new QWidget(scroll_area);
    QVBoxLayout* right_layout = new QVBoxLayout();
    right_layout->setSizeConstraint(QLayout::SetMinimumSize);
    right_container->setLayout(right_layout);
    scroll_area->setWidget(right_container);

    // build interfaces
    this->build_serial_interface_menu(right_layout);
    this->build_rom_selection_menu(right_layout);
    this->build_operations_menu(right_layout);

    // add compile information
    this->label_compile_data = new QLabel(tr("<b>Build:</b><br>Compile time: %1<br>Git id: %2<br>Version: %3")
                                          .arg(__DATE__)
                                          .arg(GIT_HASH)
                                          .arg(PROGRAM_VERSION));
    right_layout->addWidget(this->label_compile_data);

    // add padding frame on RHS
    QSpacerItem *spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    right_layout->addSpacerItem(spacer);

    this->setMinimumWidth(800);
    this->setMinimumHeight(800);

    this->create_dropdown_menu();

    // set button connections
    connect(this->button_scan_ports, SIGNAL (released()), this, SLOT (scan_com_devices()));
    connect(this->button_select_serial, SIGNAL (released()), this, SLOT (select_com_port()));

    // set icon and window title
    this->setWindowIcon(QIcon(":/assets/icon/eeprom_icon.ico"));
    this->setWindowTitle(PROGRAM_NAME);

    // set font
    int id = QFontDatabase::addApplicationFont(":/assets/fonts/Consolas.ttf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);
    QFont font10(family, 10, QFont::Normal);
    this->hex_widget->setFont(font10);
    QFont font8(family, 8, QFont::Normal);
    this->label_data_descriptor->setFont(font8);

    // re-apply settings
    this->slot_update_settings();
}

/**
 * @brief Default destructor method
 */
MainWindow::~MainWindow() {
}

/**
 * @brief Create drop-down menus
 */
void MainWindow::create_dropdown_menu() {
    QMenuBar* menubar = new QMenuBar();

    // add drop-down menus
    QMenu *menu_file = menubar->addMenu(tr("&File"));
    QMenu *menu_edit = menubar->addMenu(tr("&Edit"));
    QMenu *menu_help = menubar->addMenu(tr("&Help"));

    // actions for file menu
    QAction *action_open = new QAction(menu_file);
    QAction *action_save = new QAction(menu_file);
    QAction *action_quit = new QAction(menu_file);
    action_open->setText(tr("Open"));
    action_open->setShortcuts(QKeySequence::Open);
    action_save->setText(tr("Save"));
    action_save->setShortcuts(QKeySequence::Save);
    action_quit->setText(tr("Quit"));
    action_quit->setShortcuts(QKeySequence::Quit);
    menu_file->addAction(action_open);
    menu_file->addAction(action_save);
    menu_file->addAction(action_quit);

    // actions for edit menu
    QAction *action_settings = new QAction(menu_edit);
    action_settings->setText(tr("Settings"));
    menu_edit->addAction(action_settings);
    connect(action_settings, &QAction::triggered, this, &MainWindow::slot_settings_widget);

    // actions for help menu
    QAction *action_about = new QAction(menu_help);
    action_about->setText(tr("About"));
    menu_help->addAction(action_about);

    // debug log
    QAction *action_debug_log = new QAction(menu_help);
    action_debug_log->setText(tr("Debug Log"));
    action_debug_log ->setShortcut(Qt::Key_F2);
    menu_help->addAction(action_debug_log);
    connect(action_debug_log, &QAction::triggered, this, &MainWindow::slot_debug_log);

    // connect actions file menu
    connect(action_open, &QAction::triggered, this, &MainWindow::slot_open);
    connect(action_save, &QAction::triggered, this, &MainWindow::slot_save);
    connect(action_about, &QAction::triggered, this, &MainWindow::slot_about);
    connect(action_quit, &QAction::triggered, this, &MainWindow::exit);

    this->setMenuBar(menubar);
}

/**
 * @brief Build GUI showing serial port interface
 * @param layout position where to put this part of the GUI
 */
void MainWindow::build_serial_interface_menu(QVBoxLayout* target_layout) {
    // create interface for serial ports
    QGroupBox* serial_container = new QGroupBox("Serial interface");
    serial_container->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    QVBoxLayout *layout_serial_vertical = new QVBoxLayout();
    serial_container->setLayout(layout_serial_vertical);
    QHBoxLayout *serial_layout = new QHBoxLayout();
    target_layout->addWidget(serial_container);
    QWidget* container_serial_interface_selector = new QWidget();
    container_serial_interface_selector->setLayout(serial_layout);
    layout_serial_vertical->addWidget(container_serial_interface_selector);
    QLabel* comportlabel = new QLabel(tr("COM port"));
    serial_layout->addWidget(comportlabel);
    this->combobox_serial_ports = new QComboBox(this);
    serial_layout->addWidget(this->combobox_serial_ports);
    this->button_scan_ports = new QPushButton(tr("Scan"));
    serial_layout->addWidget(this->button_scan_ports);
    this->button_select_serial = new QPushButton(tr("Select"));
    this->button_select_serial->setEnabled(false);
    serial_layout->addWidget(this->button_select_serial);

    // create interface for currently selected com port
    QWidget* serial_selected_container = new QWidget();
    QHBoxLayout *serial_selected_layout = new QHBoxLayout();
    serial_selected_container->setLayout(serial_selected_layout);
    this->label_serial = new QLabel(tr("Please select a COM port from the menu above"));
    serial_selected_layout->addWidget(this->label_serial);
    this->label_board_id = new QLabel();
    serial_selected_layout->addWidget(this->label_board_id);
    layout_serial_vertical->addWidget(serial_selected_container);
}

/**
 * @brief Build GUI showing serial port interface
 * @param layout position where to put this part of the GUI
 */
void MainWindow::build_rom_selection_menu(QVBoxLayout* target_layout) {
    /**
     * MULTICARTRIDGE IMAGES
     */
    // create toplevel interface
    this->multirom_container = new QGroupBox("Multicard images");
    this->multirom_container->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    QVBoxLayout *layout = new QVBoxLayout();
    this->multirom_container->setLayout(layout);

    // add individual buttons here
    QPushButton* btn1 = new QPushButton("P2000T Multicart ROM");
    btn1->setProperty("image_name", QVariant(QString("p2000t-multicart.bin")));
    layout->addWidget(btn1);
    connect(btn1, SIGNAL(released()), this, SLOT(load_default_image()));

    QPushButton* btn2 = new QPushButton("C64 Multicart ROM");
    btn2->setProperty("image_name", QVariant(QString("c64-megacart.bin")));
    layout->addWidget(btn2);
    connect(btn2, SIGNAL(released()), this, SLOT(load_default_image()));

    target_layout->addWidget(this->multirom_container);

    /**
     * SINGLE ROM IMAGES
     */

    // create toplevel interface
    this->singlerom_container = new QGroupBox("Single cartridge images");
    this->singlerom_container->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    layout = new QVBoxLayout();
    this->singlerom_container->setLayout(layout);

    // add individual buttons here
    btn1 = new QPushButton("P2000T BASICNL v1.1");
    btn1->setProperty("image_name", QVariant(QString("BASIC.BIN")));
    layout->addWidget(btn1);
    connect(btn1, SIGNAL(released()), this, SLOT(load_default_image()));

    // add individual buttons here
    btn2 = new QPushButton("Select other ROM");
    layout->addWidget(btn2);

    // list of ROM images
    QList<QPair<QString, QString>> rom_images = {
        {"Assembler v5.9", "assembler 5.9.bin"},
        {"BASICNL with Bootstrap for DATA cartridge [download]", "https://github.com/ifilot/p2000t-tape-monitor/releases/latest/download/BASICBOOTSTRAP.BIN"},
        {"BASICNL with Bootstrap for SD-CARD cartridge [download]", "https://github.com/ifilot/p2000t-sdcard/releases/latest/download/BASICBOOTSTRAP.BIN"},
        {"Cassette to EEPROM Utility [download]", "https://github.com/ifilot/p2000t-tape-monitor/releases/latest/download/CASSETTE_UTILITY.BIN"},
        {"Familiegeheugen v4", "familiegeheugen 4.bin"},
        {"Flasher for DATA cartridge [download]", "https://github.com/ifilot/p2000t-tape-monitor/releases/latest/download/FLASHER.BIN"},
        {"Flasher for SD-CARD cartridge [download]", "https://github.com/ifilot/p2000t-sdcard/releases/latest/download/FLASHER.BIN"},
        {"Forth compiler", "Forth.bin"},
        {"Maintenance cartridge", "Maintenance 2.bin"},
        {"RAM (expansion board) Test [download]", "https://github.com/ifilot/p2000t-ram-expansion-board/releases/latest/download/RAMTEST.BIN"},
        {"Word Processor v2", "WordProcessor 2.bin"},
        {"Zemon assembler v1.4", "Zemon 1.4.bin"}
    };

    QMenu* rommenu = new QMenu();
    for(int i=0; i<rom_images.size(); i++) {
        QAction* action = new QAction();
        action->setText(rom_images[i].first);
        action->setProperty("image_name", QVariant(rom_images[i].second));
        connect(action, &QAction::triggered, this, &MainWindow::load_default_image);
        rommenu->addAction(action);
    }
    btn2->setMenu(rommenu);

    target_layout->addWidget(this->singlerom_container);
}

/**
 * @brief Build GUI showing serial port interface
 * @param layout position where to put this part of the GUI
 */
void MainWindow::build_operations_menu(QVBoxLayout* target_layout) {
    // create toplevel interface
    QGroupBox* container = new QGroupBox("Operations");
    container->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    QVBoxLayout *layout = new QVBoxLayout();
    container->setLayout(layout);

    // add chip label
    this->label_chip_type = new QLabel("");
    layout->addWidget(this->label_chip_type);

    // add individual buttons here
    this->button_identify_chip = new QPushButton("Identify chip");
    this->button_erase_chip = new QPushButton("Erase chip");
    this->button_read_rom = new QPushButton("Read ROM");
    this->button_read_cartridge = new QPushButton("Read P2000T Cartridge");
    this->button_flash_rom = new QPushButton("Write ROM");
    this->button_flash_bank = new QPushButton("Write ROM to bank");
    this->button_scan_slots = new QPushButton("Scan slots");

    layout->addWidget(this->button_identify_chip);
    layout->addWidget(this->button_read_rom);
    layout->addWidget(this->button_read_cartridge);
    layout->addWidget(this->button_flash_rom);
    layout->addWidget(this->button_flash_bank);
    layout->addWidget(this->button_erase_chip);
    layout->addWidget(this->button_scan_slots);

    this->button_identify_chip->setEnabled(false);
    this->button_erase_chip->setEnabled(false);
    this->button_read_cartridge->setEnabled(false);
    this->button_read_rom->setEnabled(false);
    this->button_flash_rom->setEnabled(false);
    this->button_flash_bank->setEnabled(false);
    this->button_scan_slots->setEnabled(false);

    connect(this->button_read_rom, SIGNAL(released()), this, SLOT(read_rom()));
    connect(this->button_read_cartridge, SIGNAL(released()), this, SLOT(read_cartridge()));
    connect(this->button_flash_rom, SIGNAL(released()), this, SLOT(flash_rom()));
    connect(this->button_identify_chip, SIGNAL(released()), this, SLOT(read_chip_id()));
    connect(this->button_flash_bank, SIGNAL(released()), this, SLOT(flash_bank()));
    connect(this->button_erase_chip, SIGNAL(released()), this, SLOT(erase_chip()));
    connect(this->button_scan_slots, SIGNAL(released()), this, SLOT(scan_chip()));

    target_layout->addWidget(container);
    this->progress_bar_load = new QProgressBar();
    target_layout->addWidget(this->progress_bar_load);
}

/**
 * @brief Verify whether the chip is correct before flashing
 */
void MainWindow::verify_chip() {
    // check whether the same chip is still in the socket
    qDebug() << "Trying to read chip id from: " << this->serial_interface->get_port().c_str();
    this->serial_interface->open_port();
    uint16_t chip_id = this->serial_interface->get_chip_id();
    this->serial_interface->close_port();

    qDebug() << "Verifying chip size.";
    if((chip_id >> 8 & 0xFF) == 0xBF) {
        switch(chip_id & 0xFF) {
            case 0xB5:
                if(this->num_blocks != 128*1024/256) {
                    throw std::runtime_error("Different chip in socket.");
                } else {
                    qDebug() << "Valid chip size for SST39SF010 found.";
                }
            break;
            case 0xB6:
                if(this->num_blocks != 256*1024/256) {
                    throw std::runtime_error("Different chip in socket.");
                } else {
                    qDebug() << "Valid chip size for SST39SF020 found.";
                }
            break;
            case 0xB7:
                if(this->num_blocks != 512*1024/256) {
                    throw std::runtime_error("Different chip in socket.");
                } else {
                    qDebug() << "Valid chip size for SST39SF040 found.";
                }
            break;
            default:
                throw std::runtime_error("Unknown chip id");
            break;
        }
    } else {
        throw std::runtime_error("Unknown chip id");
    }
}

/**
 * @brief Show a window with an error message
 * @param errormsg
 */
void MainWindow::raise_error_window(QMessageBox::Icon icon, const QString errormsg) {
    QMessageBox msg_box;
    msg_box.setIcon(icon);
    msg_box.setText(errormsg);
    msg_box.setWindowIcon(QIcon(":/assets/icon/eeprom_icon.ico"));
    msg_box.exec();
}

/**
 * @brief Resize a QByteArray and append with zeros
 * @param pointer to QByteArray
 * @param size
 */
void MainWindow::resize_qbytearray(QByteArray* qb, unsigned int size) {
    unsigned int oldsize = qb->size();
    qb->resize(size);

    if(size < oldsize) {
        return;
    }

    for(unsigned int i=oldsize; i<size; i++) {
        (*qb)[i] = 0x00;
    }
}

/****************************************************************************
 *  SIGNALS :: COMMUNICATION INTERFACE ROUTINES
 ****************************************************************************/

/**
 * @brief Scan all communication ports to populate drop-down box
 */
void MainWindow::scan_com_devices() {
    // clear all previous data
    this->combobox_serial_ports->clear();
    this->port_identifiers.clear();

    // pattern to recognise COM PORTS (same ids as Raspberry Pi Pico)
    static const std::vector<std::pair<uint16_t, uint16_t> > patterns = {
        std::make_pair<uint16_t, uint16_t>(0x2E8A, 0x0A),     // Raspberry PICO
        //std::make_pair<uint16_t, uint16_t>(0x2341, 0x36),   // Arduino Leonardo / 32u4
        //std::make_pair<uint16_t, uint16_t>(0x0403, 0x6001)  // FTDI FT232RL

    };

    // get communication devices
    QSerialPortInfo serial_port_info;
    QList<QSerialPortInfo> port_list = serial_port_info.availablePorts();
    std::unordered_map<std::string, std::pair<uint16_t, uint16_t> > ports;
    QStringList device_descriptors;
    qInfo() << "Discovered COM ports.";
    for(int i=0; i<port_list.size(); i++) {
        auto ids = std::make_pair<uint16_t,uint16_t>(port_list[i].vendorIdentifier(), port_list[i].productIdentifier());
        for(int j=0; j<patterns.size(); j++) {
            if(ids == patterns[j]) {
                ports.emplace(port_list[i].portName().toStdString(), ids);
                device_descriptors.append(port_list[i].description());
                qInfo() << port_list[i].portName().toStdString().c_str()
                        << QString("pid=0x%1, vid=0x%2,").arg(port_list[i].vendorIdentifier(),2,16).arg(port_list[i].productIdentifier(),2,16).toStdString().c_str()
                        << QString("descriptor=\"%1\",").arg(port_list[i].description()).toStdString().c_str()
                        << QString("serial=\"%1\"").arg(port_list[i].serialNumber()).toStdString().c_str();
            }
        }
    }

    // populate drop-down menu with valid ports
    for(const auto& item : ports) {
        this->combobox_serial_ports->addItem(item.first.c_str());
        this->port_identifiers.push_back(item.second);
    }

    // if more than one option is available, enable the button
    if(this->combobox_serial_ports->count() > 0) {
        this->button_select_serial->setEnabled(true);
    }


    if(port_identifiers.size() == 1) {
        statusBar()->showMessage(tr("Auto-selecting ") + this->combobox_serial_ports->itemText(0) + tr(" (vid and pid match board)."));
        this->combobox_serial_ports->setCurrentIndex(0);
        this->select_com_port();
    } else if(port_identifiers.size() > 1) {
        QMessageBox msg_box;
        msg_box.setIcon(QMessageBox::Warning);
        msg_box.setText(tr(
              "There are at least %1 devices that share the same id. Please ensure that only a single programmer device is plugged in."
              " If multiple devices are plugged in, ensure you select the correct port. Please also note that the device id overlaps"
              " with the one from the Raspberry Pi Pico. If you have a Raspberry Pi Pico or compatible device plugged in,"
              " take care to unplug it or carefully select the correct port."
        ).arg(port_identifiers.size()));
        msg_box.setWindowIcon(QIcon(":/assets/icon/eeprom_icon.ico"));
        msg_box.exec();
    } else {
        QMessageBox msg_box;
        msg_box.setIcon(QMessageBox::Warning);
        msg_box.setText("Could not find a communication port with a matching id. Please make sure the programmer device is plugged in.");
        msg_box.setWindowIcon(QIcon(":/assets/img/logo.ico"));
        msg_box.exec();
    }
}

/**
 * @brief Select communication port for serial to 32u4
 */
void MainWindow::select_com_port() {
    this->serial_interface = std::make_shared<SerialInterface>(this->combobox_serial_ports->currentText().toStdString());

    this->serial_interface->open_port();
    std::string board_info = this->serial_interface->get_board_info();
    this->serial_interface->close_port();
    this->label_serial->setText(tr("Port: ") + this->combobox_serial_ports->currentText());
    this->label_board_id->setText(tr("Board id: ") + tr(board_info.c_str()));

    if(QString(board_info.c_str()).startsWith("PICOSST39")) {
        QString versionstr = QString(board_info.c_str()).split("-")[1].mid(1);
        qDebug() << "Extracting version string: " << versionstr;
        QStringList version = versionstr.split(".");
        if(version.size() != 3) {
            QMessageBox msg_box;
            msg_box.setIcon(QMessageBox::Warning);
            msg_box.setText(tr(
                  "Something went wrong parsing the version string."
            ));
            msg_box.setWindowIcon(QIcon(":/assets/icon/eeprom_icon.ico"));
            msg_box.exec();
        } else {
            int major = version[0].toInt();
            int minor = version[1].toInt();
            int patch = version[2].toInt();

            int versionint = (major * 1000000) + (minor * 1000) + patch;

            if(versionint < (1 * 1000000) + (3 * 1000) + 0) {
                QMessageBox msg_box;
                msg_box.setIcon(QMessageBox::Warning);
                msg_box.setText(tr(
                      "Your board firmware is outdated. Please update your board "
                      "to version 1.3.0 or higher. Visit: "
                      "<a href=\"https://github.com/ifilot/pico-sst39sf0x0-programmer\">https://github.com/ifilot/pico-sst39sf0x0-programmer</a>."
                ));
                msg_box.setWindowIcon(QIcon(":/assets/icon/eeprom_icon.ico"));
                msg_box.exec();
            } else {
                this->button_identify_chip->setEnabled(true);
                this->button_read_cartridge->setEnabled(true);
            }
        }
    } else {
        QMessageBox msg_box;
        msg_box.setIcon(QMessageBox::Warning);
        msg_box.setText(tr(
              "Invalid board identifier token read. Most likely, this is not the correct board."
        ));
        msg_box.setWindowIcon(QIcon(":/assets/icon/eeprom_icon.ico"));
        msg_box.exec();
    }
}

/**
 * @brief Open a binary file
 */
void MainWindow::slot_open() {
    // load last open path from settings
    QDir dir = QDir(settings.value("last_open_dir", QString(QDir::homePath())).toString());
    if(!dir.exists()) {
        dir.setPath(QDir::homePath());
    }
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), dir.absolutePath());

    // do nothing if user has cancelled
    if(filename.isEmpty()) {
        return;
    }

    // store entry of last opened folder
    QDir storepath = QFileInfo(filename).dir();
    qDebug() << storepath;
    settings.setValue("last_open_dir", storepath.path());

    // load file
    QFile file(filename);

    // provide a warning to the user if the file is larger than half a megabyte
    if(file.size() > (512 * 1024)) {
        QMessageBox msg_box;
        msg_box.setIcon(QMessageBox::Warning);
        msg_box.setText(tr("This file is larger than 512kb. It is most likely not a Z80 binary file."));
        msg_box.setWindowIcon(QIcon(":/assets/icon/eeprom_icon.ico"));
        msg_box.exec();
        return;
    }

    if(file.exists()) {
        file.open(QIODevice::ReadOnly);
        QByteArray data = file.readAll();
        this->current_filename = file.fileName();
        int filesize = data.size();

        // ask the user whether they want to expand the image
        if(data.size() < 0x4000) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Expand this file to cartridge size?",
                                                "This file is less than 16kb. If this file is intended to "
                                                "be used as a cartridge, would you like to to expand it "
                                                "to 16kb by padding the data with 0x00?", QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                this->current_file_expanded = true;
                resize_qbytearray(&data, 0x4000);
            } else {
                this->current_file_expanded = false;
            }
        }

        this->hex_widget->set_data(data);
        this->button_reload_file->setEnabled(true);

        QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Md5);
        QFileInfo finfo(file);

        QString usage;
        if(data.size() == 0x4000) {
            usage = QString(" | %1 kb / %2 kb (%3 %)")
                    .arg(QString::number((float)filesize/(float)1024, 'f', 1))
                    .arg(QString::number(16, 'f', 1))
                    .arg((float)filesize/(float)(16 * 1024) * 100, 0, 'f', 1);
        }

        this->label_data_descriptor->setText(QString("<b>%1</b> | Size: %2 kb | MD5: %3" + usage)
              .arg(finfo.fileName())
              .arg(data.size() / 1024)
              .arg(QString(hash.toHex()))
        );
    }
}

/**
 * @brief MainWindow::reload a file
 */
void MainWindow::slot_reload_file() {
    QFile file(this->current_filename);

    if(file.exists()) {
        file.open(QIODevice::ReadOnly);
        QByteArray data = file.readAll();
        this->current_filename = file.fileName();
        int filesize = data.size();

        // ask the user whether they want to expand the image
        if(data.size() < 0x4000 && this->current_file_expanded) {
            resize_qbytearray(&data, 0x4000);
        }

        this->hex_widget->set_data(data);

        QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Md5);
        QFileInfo finfo(file);

        QString usage;
        if(data.size() == 0x4000) {
            usage = QString(" | %1 kb / %2 kb (%3 %)")
                        .arg(QString::number((float)filesize/(float)1024, 'f', 1))
                        .arg(QString::number(16, 'f', 1))
                        .arg((float)filesize/(float)(16 * 1024) * 100, 0, 'f', 1);
        }

        this->label_data_descriptor->setText(QString("<b>%1</b> | Size: %2 kb | MD5: %3" + usage)
                                                 .arg(finfo.fileName())
                                                 .arg(data.size() / 1024)
                                                 .arg(QString(hash.toHex()))
                                             );

        statusBar()->showMessage(tr("Reloaded %1 from drive.").arg(this->current_filename));
    }
}

/**
 * @brief Open a binary file
 */
void MainWindow::slot_save() {
    // do nothing if data size is zero
    auto data = this->hex_widget->get_data();
    if(data.size() == 0) {
        return;
    }

    // load last save path from settings
    QDir dir = QDir(settings.value("last_save_dir", QString(QDir::homePath())).toString());
    if(!dir.exists()) {
        dir.setPath(QDir::homePath());
    }
    QString filename = QFileDialog::getSaveFileName(this, tr("Save File"), dir.absolutePath(), tr("roms (*.bin *.rom)"));

    // do nothing if user has cancelled
    if(filename.isEmpty()) {
        return;
    }

    // store entry of last opened folder
    QDir storepath = QFileInfo(filename).dir();
    qDebug() << storepath;
    settings.setValue("last_save_dir", storepath.path());

    QFile file(filename);

    file.open(QIODevice::WriteOnly);
    file.write(this->hex_widget->get_data());
    file.close();
}

/**
 * @brief Show an about window
 */
void MainWindow::slot_about() {
    QMessageBox message_box;
    //message_box.setStyleSheet("QLabel{min-width: 250px; font-weight: normal;}");
    message_box.setText(PROGRAM_NAME
                        " version "
                        PROGRAM_VERSION
                        ".\n\nAuthor:\nIvo Filot <ivo@ivofilot.nl>\n\n"
                        PROGRAM_NAME " is licensed under the GPLv3 license.\n\n"
                        PROGRAM_NAME " is dynamically linked to Qt, which is licensed under LGPLv3.\n");
    message_box.setIcon(QMessageBox::Information);
    message_box.setWindowTitle("About " + tr(PROGRAM_NAME));
    message_box.setWindowIcon(QIcon(":/assets/icon/eeprom_icon.ico"));
    message_box.exec();
}

/**
 * @brief Show an about window
 */
void MainWindow::slot_debug_log() {
    this->log_window->show();
}

/**
 * @brief Show an about window
 */
void MainWindow::slot_settings_widget() {
    this->settings_widget->show();
}

/**
 * @brief Update the main window after a settings update / change
 */
void MainWindow::slot_update_settings() {
    //qDebug() << "Settings update triggered";

    bool show_retroroms = this->settings.value("SHOW_RETROROMS", QVariant(true)).toBool();
    this->multirom_container->setVisible(show_retroroms);
    this->singlerom_container->setVisible(show_retroroms);
    dynamic_cast<QWidget*>(this->singlerom_container->parent())->layout()->invalidate();
    this->hex_widget->viewport()->repaint();
}

/**
 * @brief Convenience function on completing a chip scan
 */
void MainWindow::parse_chip_read_results() {
    this->progress_bar_load->setValue(this->progress_bar_load->maximum());

    QByteArray data;
    if(this->readerthread) {
        data = this->readerthread->get_data();
        this->readerthread.reset(); // delete object
    } else if(this->cartridgereaderthread) {
        data = this->cartridgereaderthread->get_data();
        this->readerthread.reset(); // delete object
    } else {
        qDebug() << "This function should not have been called.";
    }

    qDebug() << "Read " << data.size() << " bytes from chip.";
    this->hex_widget->set_data(data);

    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Md5);
    this->label_data_descriptor->setText(QString("<b>%1</b> | Size: %2 kb | MD5: %3")
                                             .arg("ROM data")
                                             .arg(data.size() / 1024)
                                             .arg(QString(hash.toHex()))
                                         );

    statusBar()->showMessage(QString("Done reading chip in %1 seconds.").arg(this->timer1.elapsed() / 1000.f));
}

/**
 * @brief      Close the application
 */
void MainWindow::exit() {
    QApplication::quit();
}

void MainWindow::load_default_image() {
    QString image = sender()->property("image_name").toString();
    qDebug() << "Loading default image: " << image;

    if(image.startsWith("http")) { // try to grab image from the web
        QTimer timer;
        timer.setSingleShot(true);
        QUrl url(image);
        FileDownloader *fd = new FileDownloader(url, this);
        QEventLoop loop;
        connect(fd, SIGNAL(downloaded()), &loop, SLOT(quit()));
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.start(5000); // time out after 5 seconds
        loop.exec();

        if(timer.isActive()) {
            QByteArray data = fd->downloadedData();
            this->hex_widget->set_data(data);
            this->button_reload_file->setEnabled(false);

            QString rom_name = image.split(QChar('/')).back();

            QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Md5);
            this->label_data_descriptor->setText(QString("<b>%1</b> | Size: %2 kb | MD5: %3")
                                                     .arg(rom_name + " (download)")
                                                     .arg(data.size() / 1024)
                                                     .arg(QString(hash.toHex()))
                                                 );
            statusBar()->showMessage(tr("Succesfully downloaded %1 from web source.").arg(rom_name));
        } else {
            QMessageBox message_box;
            message_box.setText("Could not download the image. Please check your internet connection and/or try again.");
            message_box.setIcon(QMessageBox::Critical);
            message_box.setWindowTitle("Download of ROM failed");
            message_box.setWindowIcon(QIcon(":/assets/icon/eeprom_icon.ico"));
            message_box.exec();
        }

    } else {    // image is stored
        QString path = ":/assets/roms/" + image;
        QFile file(path);
        file.open(QIODevice::ReadOnly);
        if(file.exists()) {
            QByteArray data = file.readAll();
            this->hex_widget->set_data(data);

            QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Md5);
            this->label_data_descriptor->setText(QString("<b>%1</b> | Size: %2 kb | MD5: %3")
                                                     .arg(image)
                                                     .arg(data.size() / 1024)
                                                     .arg(QString(hash.toHex()))
                                                 );
        } else {
            QMessageBox message_box;
            message_box.setText("Unknown image. This error should not happen. Please contact the developer.");
            message_box.setIcon(QMessageBox::Critical);
            message_box.setWindowTitle("Could not find rom image");
            message_box.setWindowIcon(QIcon(":/assets/icon/eeprom_icon.ico"));
            message_box.exec();
        }
    }
}

/****************************************************************************
 *  SIGNALS :: READ ROM ROUTINES
 ****************************************************************************/

/**
 * @brief Read data from chip
 */
void MainWindow::read_chip_id() {
    statusBar()->showMessage("Reading from chip, please wait...");
    this->timer1.start();

    try {
        qDebug() << "Trying to read chip id from: " << this->serial_interface->get_port().c_str();
        this->serial_interface->open_port();
        uint16_t chip_id = this->serial_interface->get_chip_id();
        this->serial_interface->close_port();

        qDebug() << tr("Reading 0x%1 0x%2").arg((uint16_t)chip_id >> 8, 2, 16, QChar('0')).arg((uint8_t)chip_id & 0xFF, 2, 16, QChar('0'));
        std::string chip_id_str = tr("0x%1%2").arg((uint16_t)chip_id >> 8, 2, 16, QChar('0')).arg((uint8_t)chip_id & 0xFF, 2, 16, QChar('0')).toStdString();
        if((chip_id >> 8 & 0xFF) == 0xBF) {
            switch(chip_id & 0xFF) {
                case 0xB5:
                    this->num_blocks = 128*1024/256;
                    statusBar()->showMessage("Identified a SST39SF010 chip");
                    this->label_chip_type->setText("ROM chip: SST39SF010");
                break;
                case 0xB6:
                this->num_blocks = 256*1024/256;
                    statusBar()->showMessage("Identified a SST39SF020 chip");
                    this->label_chip_type->setText("ROM chip: SST39SF020");
                break;
                case 0xB7:
                    this->num_blocks = 512*1024/256;
                    statusBar()->showMessage("Identified a SST39SF040 chip");
                    this->label_chip_type->setText("ROM chip: SST39SF040");
                break;
                default:
                    throw std::runtime_error("Unknown chip id: " + chip_id_str);
                break;
            }
        } else {
            throw std::runtime_error("Unknown chip id: " + chip_id_str);
        }

        // set progress bar
        this->progress_bar_load->reset();
        this->progress_bar_load->setMaximum(this->num_blocks);

        // enable buttons
        this->button_read_rom->setEnabled(true);
        this->button_flash_rom->setEnabled(true);
        this->button_flash_bank->setEnabled(true);
        this->button_erase_chip->setEnabled(true);
        this->button_scan_slots->setEnabled(true);
    } catch (const std::exception& e) {
        QMessageBox msg_box;
        msg_box.setIcon(QMessageBox::Warning);
        msg_box.setText(tr("The chip id does not match the proper value for a SST39SF0x0 chip. Please ensure "
                           "that a correct SST39SF0x0 chip is inserted. If so, resocket the chip and try again.\n\nError message: %1.").arg(e.what()));
        msg_box.setWindowIcon(QIcon(":/assets/icon/eeprom_icon.ico"));
        msg_box.exec();
    }
}

/**
 * @brief Read data from chip
 */
void MainWindow::read_rom() {
    // ask where to store file
    statusBar()->showMessage("Reading from chip, please wait...");
    this->label_data_descriptor->clear();

    // dispatch thread
    this->timer1.start();

    // disable all buttons so that the user cannot interrupt this task
    //this->disable_all_buttons();

    // dispatch thread
    //this->operation = "Reading"; // message for statusbar
    this->readerthread = std::make_unique<ReadThread>(this->serial_interface);
    this->readerthread->set_serial_port(this->combobox_serial_ports->currentText().toStdString());
    connect(this->readerthread.get(), SIGNAL(read_result_ready()), this, SLOT(read_result_ready()));
    connect(this->readerthread.get(), SIGNAL(read_block_start(uint,uint)), this, SLOT(read_block_start(uint,uint)));
    connect(this->readerthread.get(), SIGNAL(read_block_done(uint,uint)), this, SLOT(read_block_done(uint,uint)));
    connect(this->readerthread.get(), SIGNAL(thread_abort(const QString&)), this, SLOT(thread_abort(const QString&)));
    this->readerthread->start();
}

/**
 * @brief Read data from chip
 */
void MainWindow::read_cartridge() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "P2000T Adapter board present?", "Please ensure you have attached the P2000T adapter board. Continue?", QMessageBox::Yes|QMessageBox::No);
    if (reply != QMessageBox::Yes) {
        return;
    }

    // dispatch thread
    this->timer1.start();

    // ask where to store file
    statusBar()->showMessage("Reading P2000T cartridge, please wait...");

    // disable all buttons so that the user cannot interrupt this task
    //this->disable_all_buttons();

    // dispatch thread
    //this->operation = "Reading"; // message for statusbar
    this->cartridgereaderthread = std::make_unique<CartridgeReadThread>(this->serial_interface);
    this->cartridgereaderthread->set_serial_port(this->combobox_serial_ports->currentText().toStdString());
    connect(this->cartridgereaderthread.get(), SIGNAL(read_result_ready()), this, SLOT(read_result_ready()));
    connect(this->cartridgereaderthread.get(), SIGNAL(read_block_start(uint,uint)), this, SLOT(read_block_start(uint,uint)));
    connect(this->cartridgereaderthread.get(), SIGNAL(read_block_done(uint,uint)), this, SLOT(read_block_done(uint,uint)));
    connect(this->readerthread.get(), SIGNAL(thread_abort(const QString&)), this, SLOT(thread_abort(const QString&)));
    this->cartridgereaderthread->start();
}

/**
 * @brief Slot to accept when a block is ready
 */
void MainWindow::read_block_start(unsigned int block_id, unsigned int nr_blocks) {
    this->progress_bar_load->setValue(block_id);
}

/**
 * @brief Slot to accept when a block is ready
 */
void MainWindow::read_block_done(unsigned int block_id, unsigned int nr_blocks) {
    double seconds_passed = (double)this->timer1.elapsed() / 1000.0;
    double seconds_per_block = seconds_passed / (double)(block_id+1);
    double seconds_remaining = seconds_per_block * (nr_blocks - block_id);
    if(block_id < (this->num_blocks - 1)) {
        statusBar()->showMessage(QString("%1 block %2 / %3 : %4 seconds remaining.").arg("Reading").arg(block_id+1).arg(nr_blocks).arg(seconds_remaining));
    }
    this->progress_bar_load->setValue(block_id+1);
    this->progress_bar_load->setMaximum(nr_blocks);
}

/*
 * @brief Signal that a read operation is finished
 */
void MainWindow::read_result_ready() {
    this->parse_chip_read_results();
}

/*
 * @brief Signal that a scan chip operation is finished
 */
void MainWindow::scan_chip_result_ready() {
    this->parse_chip_read_results();

    // at this point, the chip has been scanned and we can process the results
    BankViewer bankviewer(this->num_blocks / BLOCKSPERBANK, this->hex_widget->get_data(), this);
    bankviewer.exec();
}

/****************************************************************************
 *  SIGNALS :: FLASH ROM
 ****************************************************************************/

/**
 * @brief Put rom on flash cartridge
 */
void MainWindow::flash_rom() {
    // store flash data
    qDebug() << "Loading flash data";
    this->flash_data = this->hex_widget->get_data();

    // verify whether the chip is correct
    qDebug() << "Verifying chip";
    try {
        this->verify_chip();
    } catch(const std::exception& e) {
        this->raise_error_window(QMessageBox::Critical,
                                 "Cannot flash this ROM to bank due to a problem with "
                                 "with the CHIP id. Please carefully check the chip and "
                                 "whether it is properly inserted into the socket."
                                 "\n\nError message: " + QString(e.what())
                                 );
        return;
    }

    if((this->num_blocks * 256) < this->flash_data.size()) {

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Truncate file?",
                                            QString("The capacity of the chip (%1 bytes) is less than the amount "
                                                    "of data (%2 bytes) to be flashed. The data will be truncated. Continue?")
                                            .arg(this->num_blocks * 256)
                                            .arg(this->flash_data.size()),
                                       QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            this->resize_qbytearray(&this->flash_data, this->num_blocks * 256);
        } else {
            return;
        }
    }

    // dispatch thread
    this->timer1.start();

    this->progress_bar_load->setMaximum(8 * SECTORSPERBANK);
    this->flashthread = std::make_unique<FlashThread>(this->serial_interface);
    this->flashthread->set_serial_port(this->combobox_serial_ports->currentText().toStdString());
    this->flashthread->set_data(this->flash_data);

    connect(this->flashthread.get(), SIGNAL(flash_result_ready()), this, SLOT(flash_result_ready()));
    connect(this->flashthread.get(), SIGNAL(flash_sector_start(uint,uint)), this, SLOT(flash_sector_start(uint,uint)));
    connect(this->flashthread.get(), SIGNAL(flash_sector_done(uint,uint)), this, SLOT(flash_sector_done(uint,uint)));
    connect(this->readerthread.get(), SIGNAL(thread_abort(const QString&)), this, SLOT(thread_abort(const QString&)));
    flashthread->start();

    // disable all buttons
    //this->disable_all_buttons();
}

/**
 * @brief Put rom on flash cartridge
 */
void MainWindow::flash_bank() {
    // select to which bank to flash
    DialogSlotSelection dialog(this->num_blocks / BLOCKSPERBANK);
    int res = dialog.exec();
    if(res != QDialog::Accepted) {
        qDebug() << "Cancelled operation.";
        return;
    }

    // load starting bank from DialogSlotSelection class
    uint8_t starting_bank = dialog.get_slot_id();
    qDebug() << "Start writing at bank: " << starting_bank;

    // store flash data
    this->flash_data = this->hex_widget->get_data();

    // verify whether the chip is correct
    try {
        this->verify_chip();
    } catch(const std::exception& e) {
        this->raise_error_window(QMessageBox::Critical,
                                 "Cannot flash this ROM to bank due to a problem with "
                                 "with the CHIP id. Please carefully check the chip and "
                                 "whether it is properly inserted into the socket."
                                 "\n\nError message: " + QString(e.what())
                                 );
        return;
    }

    if(this->flash_data.size() > 16 * 1024) {
        this->raise_error_window(QMessageBox::Critical,
                                 "Image size does not match 16kb. Cannot flash this image to a bank."
                                 " Most likely, you have selected a multicartridge image. "
                                 "Please select a single-cartridge image and try again.");
        return;
    } else {    // expand size to 16 kb, pad with zeros if size is smaller than 16kb
        this->resize_qbytearray(&this->flash_data, 16*1024);
    }

    // dispatch thread
    this->timer1.start();

    this->progress_bar_load->setMaximum(SECTORSPERBANK); // 4 sectors for a 16 kb bank
    this->flashthread = std::make_unique<FlashThread>(this->serial_interface, starting_bank);
    this->flashthread->set_serial_port(this->combobox_serial_ports->currentText().toStdString());
    this->flashthread->set_data(this->flash_data);

    connect(this->flashthread.get(), SIGNAL(flash_result_ready()), this, SLOT(flash_result_ready()));
    connect(this->flashthread.get(), SIGNAL(flash_sector_start(uint,uint)), this, SLOT(flash_sector_start(uint,uint)));
    connect(this->flashthread.get(), SIGNAL(flash_sector_done(uint,uint)), this, SLOT(flash_sector_done(uint,uint)));
    connect(this->readerthread.get(), SIGNAL(thread_abort(const QString&)), this, SLOT(thread_abort(const QString&)));
    flashthread->start();

    // disable all buttons
    //this->disable_all_buttons();
}

/**
 * @brief Put rom on flash cartridge
 */
void MainWindow::erase_chip() {
    // verify whether the chip is correct
    try {
        this->verify_chip();
    } catch(const std::exception& e) {
        this->raise_error_window(QMessageBox::Critical,
                                 "Cannot flash this ROM to bank due to a problem with "
                                 "with the CHIP id. Please carefully check the chip and "
                                 "whether it is properly inserted into the socket."
                                 "\n\nError message: " + QString(e.what())
                                 );
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Wipe chip?", "Are you sure you want to completely wipe the chip?", QMessageBox::Yes|QMessageBox::No);
    if (reply != QMessageBox::Yes) {
        return;
    }

    this->serial_interface->open_port();
    this->serial_interface->erase_chip();
    this->serial_interface->close_port();

    QMessageBox::information(this, "Chip erased.", "Done erasing the chip. All bytes are set of 0xFF.");
}

/**
 * @brief MainWindow::Scan the ROMs on the chip
 */
void MainWindow::scan_chip() {
    // ask where to store file
    statusBar()->showMessage("Reading from chip, please wait...");
    this->label_data_descriptor->clear();

    // dispatch thread
    this->timer1.start();

    // disable all buttons so that the user cannot interrupt this task
    //this->disable_all_buttons();

    // dispatch thread
    //this->operation = "Reading"; // message for statusbar
    this->readerthread = std::make_unique<ReadThread>(this->serial_interface);
    this->readerthread->set_serial_port(this->combobox_serial_ports->currentText().toStdString());
    connect(this->readerthread.get(), SIGNAL(read_result_ready()), this, SLOT(scan_chip_result_ready()));
    connect(this->readerthread.get(), SIGNAL(read_block_start(uint,uint)), this, SLOT(read_block_start(uint,uint)));
    connect(this->readerthread.get(), SIGNAL(read_block_done(uint,uint)), this, SLOT(read_block_done(uint,uint)));
    connect(this->readerthread.get(), SIGNAL(thread_abort(const QString&)), this, SLOT(thread_abort(const QString&)));
    this->readerthread->start();
}

/**
 * @brief Slot to indicate that a sector is about to be written
 */
void MainWindow::flash_sector_start(unsigned int sector_id, unsigned int num_sectors) {
    this->progress_bar_load->setValue(sector_id);
    this->progress_bar_load->setMaximum(num_sectors);
}

/**
 * @brief Slot to accept that a sector is written
 */
void MainWindow::flash_sector_done(unsigned int sector_id, unsigned int num_sectors) {
    double seconds_passed = (double)this->timer1.elapsed() / 1000.0;
    double seconds_per_sector = seconds_passed / (double)(sector_id+1);
    double seconds_remaining = seconds_per_sector * (num_sectors - sector_id - 1);
    if(sector_id < (num_sectors - 1)) {
        statusBar()->showMessage(QString("%1 sector %2 / %3 : %4 seconds remaining.").arg("Flashing").arg(sector_id+1).arg(num_sectors).arg(seconds_remaining));
    }
    this->progress_bar_load->setValue(sector_id+1);
}

/*
 * @brief Signal that a flash operation is finished
 */
void MainWindow::flash_result_ready() {
    this->progress_bar_load->setValue(this->progress_bar_load->maximum());
    statusBar()->showMessage("Ready - Done flashing in " + QString::number((double)this->timer1.elapsed() / 1000) + " seconds.");

    // dispatch thread
    this->timer1.restart();
    unsigned int nr_banks = this->flashthread->get_num_sectors() / SECTORSPERBANK;
    qDebug() << tr("Requesting %1 banks to be read for verification.").arg(nr_banks);
    this->readerthread = std::make_unique<ReadThread>(this->serial_interface,
                                                      this->flashthread->get_starting_bank(),  // which bank to start reading at
                                                      nr_banks);                               // number of banks
    this->readerthread->set_serial_port(this->combobox_serial_ports->currentText().toStdString());

    // set progress bar
    this->progress_bar_load->reset();
    this->progress_bar_load->setMaximum(this->flashthread->get_num_sectors() / SECTORSPERBANK);

    connect(this->readerthread.get(), SIGNAL(read_result_ready()), this, SLOT(verify_result_ready()));
    connect(this->readerthread.get(), SIGNAL(read_block_start(uint,uint)), this, SLOT(verify_block_start(uint,uint)));
    connect(this->readerthread.get(), SIGNAL(read_block_done(uint,uint)), this, SLOT(verify_block_done(uint,uint)));
    connect(this->readerthread.get(), SIGNAL(thread_abort(const QString&)), this, SLOT(thread_abort(const QString&)));
    this->readerthread->start();
}

/*
 * @brief Response that the chip id could not be verified
 */
void MainWindow::flash_chip_id_error(unsigned int chip_id) {
    QMessageBox msg_box;
    msg_box.setIcon(QMessageBox::Warning);
    msg_box.setText(tr("The chip id (%1) does not match the proper value for a SST39SF0x0 chip. Please verify that you inserted"
                       " and/or selected the right FLASH cartridge. If so, resocket the cartridge and try again.").arg(chip_id,0,16));
    msg_box.setWindowIcon(QIcon(":/assets/icon/eeprom_icon.ico"));
    msg_box.exec();

    // reset flash button
    this->button_flash_rom->setEnabled(true);
}

/**
 * @brief Slot to accept when a block is about to be verified
 */
void MainWindow::verify_block_start(unsigned int block_id, unsigned int nr_blocks) {
    this->progress_bar_load->setValue(block_id);
}

/**
 * @brief Slot to accept when a block is verified
 */
void MainWindow::verify_block_done(unsigned int block_id, unsigned int nr_blocks) {
    double seconds_passed = (double)this->timer1.elapsed() / 1000.0;
    double seconds_per_block = seconds_passed / (double)(block_id+1);
    double seconds_remaining = seconds_per_block * (nr_blocks - block_id - 1);
    if(block_id < (this->num_blocks - 1)) {
        statusBar()->showMessage(QString("%1 block %2 / %3 : %4 seconds remaining.").arg("Verifying").arg(block_id+1).arg(nr_blocks).arg(seconds_remaining));
    }
    this->progress_bar_load->setValue(block_id+1);
}

/*
 * @brief Signal that a verified operation is finished
 */
void MainWindow::verify_result_ready() {
    this->progress_bar_load->setValue(this->num_blocks);
    QByteArray verify_data = this->readerthread->get_data();
    this->readerthread.reset(); // delete object

    if(verify_data == this->flash_data) {
        statusBar()->showMessage("Ready - Done verification in " + QString::number((double)this->timer1.elapsed() / 1000) + " seconds.");
        QMessageBox msg_box(QMessageBox::Information,
                "Flash complete",
                "Cartridge was successfully flashed. Data integrity verified.",
                QMessageBox::Ok, this);
        msg_box.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
        msg_box.exec();
    } else {
        QMessageBox msg_box(QMessageBox::Critical,
                "Error",
                "Data integrity could not be verified. Please try to reflash the cartridge. It might help to resocket the flash cartridge.",
                QMessageBox::Ok, this);
        msg_box.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
        msg_box.exec();
    }

    // re-enable all buttons when data is read
    // this->enable_all_buttons();
    this->progress_bar_load->reset();
}

void MainWindow::thread_abort(const QString& error) {
    // clean up threads
    if(this->readerthread) {
        while(!this->readerthread->isFinished()) {}
        this->readerthread.reset(); // delete object
    }

    if(this->flashthread) {
        while(!this->flashthread->isFinished()) {}
        this->flashthread.reset(); // delete object
    }

    if(this->cartridgereaderthread) {
        while(!this->cartridgereaderthread->isFinished()) {}
        this->cartridgereaderthread.reset(); // delete object
    }

    // throw error message
    this->raise_error_window(QMessageBox::Critical,
                             "Operation terminated unexpectedly. Please carefully "
                             "check all settings and verify that the chip is "
                             "properly inserted into the socket."
                             "\n\nError message: " + error
                             );
}
