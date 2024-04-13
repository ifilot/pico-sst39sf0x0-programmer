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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QApplication>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QGroupBox>
#include <QStatusBar>
#include <QMessageBox>
#include <QFile>
#include <QProgressBar>
#include <QTimer>
#include <QElapsedTimer>
#include <QFrame>
#include <QFileDialog>
#include <QCryptographicHash>
#include <QSettings>

#include "config.h"
#include "hexviewwidget.h"
#include "serial_interface.h"
#include "cartridgereadthread.h"
#include "readthread.h"
#include "flashthread.h"
#include "dialogslotselection.h"
#include "romsizes.h"
#include "logwindow.h"
#include "settingswidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    // widgets
    QLabel* label_data_descriptor;
    HexViewWidget* hex_widget;

    // window for log messages
    std::unique_ptr<LogWindow> log_window;

    // settings widget
    std::unique_ptr<SettingsWidget> settings_widget;

    // storage for log messages
    std::shared_ptr<QStringList> log_messages;

    // Serial port selection
    QPushButton* button_select_serial;
    QPushButton* button_scan_ports;
    QLabel* label_serial;
    QLabel* label_board_id;
    QComboBox* combobox_serial_ports;
    std::vector<std::pair<uint16_t, uint16_t>> port_identifiers;
    std::shared_ptr<SerialInterface> serial_interface;

    // buttons
    QLabel* label_chip_type;
    QPushButton* button_identify_chip;
    QPushButton* button_erase_chip;
    QPushButton* button_read_rom;
    QPushButton* button_read_cartridge;
    QPushButton* button_flash_rom;
    QPushButton* button_flash_bank;

    // file data
    QString current_filename;
    std::unique_ptr<CartridgeReadThread> cartridgereaderthread;
    std::unique_ptr<ReadThread> readerthread;
    std::unique_ptr<FlashThread> flashthread;
    QElapsedTimer timer1;
    int num_blocks = 0;

    QProgressBar* progress_bar_load;

    QByteArray flash_data;
    QByteArray save_data;

    QLabel* label_compile_data;
    QSettings settings;

    QGroupBox *multirom_container;
    QGroupBox *singlerom_container;

public:
    /**
     * @brief MainWindow
     * @param parent
     */
    MainWindow(const std::shared_ptr<QStringList> _log_messages,
               QWidget *parent = nullptr);

    /**
     * @brief Default destructor method
     */
    ~MainWindow();

private:
    /**
     * @brief Create drop-down menus
     */
    void create_dropdown_menu();

    /**
     * @brief Build GUI showing serial port interface
     * @param layout position where to put this part of the GUI
     */
    void build_serial_interface_menu(QVBoxLayout* target_layout);

    /**
     * @brief Build GUI showing set of default ROMS to burn
     * @param layout position where to put this part of the GUI
     */
    void build_rom_selection_menu(QVBoxLayout* target_layout);

    /**
     * @brief Build GUI showing operations to perform
     * @param layout position where to put this part of the GUI
     */
    void build_operations_menu(QVBoxLayout* target_layout);

    /**
     * @brief Verify whether the chip is correct before flashing
     */
    void verify_chip();

    /**
     * @brief Show a window with an error message
     * @param errormsg
     */
    void raise_error_window(QMessageBox::Icon icon, const QString errormsg);

    /**
     * @brief Resize a QByteArray and append with zeros
     * @param pointer to QByteArray
     * @param size
     */
    void resize_qbytearray(QByteArray* qb, unsigned int size);

private slots:
    /**
     * @brief      Close the application
     */
    void exit();

    /**
     * @brief Open a binary file
     */
    void slot_open();

    /**
     * @brief Save a binary file
     */
    void slot_save();

    /**
     * @brief Show an about window
     */
    void slot_about();

    /**
     * @brief Show the debug log
     */
    void slot_debug_log();

    /**
     * @brief Show the settings widget
     */
    void slot_settings_widget();

    /**
     * @brief Update the main window after a settings update / change
     */
    void slot_update_settings();

    /****************************************************************************
     *  SIGNALS :: COMMUNICATION INTERFACE ROUTINES
     ****************************************************************************/

    /**
     * @brief Scan all communication ports to populate drop-down box
     */
    void scan_com_devices();

    /**
     * @brief Select communication port for serial to 32u4
     */
    void select_com_port();

    /**
     * @brief Load default image into HexWidget window
     */
    void load_default_image();

    /****************************************************************************
     *  SIGNALS :: READ ROM ROUTINES
     ****************************************************************************/

    /**
     * @brief Read data from chip
     */
    void read_chip_id();

    /**
     * @brief Read data from chip
     */
    void read_rom();

    /**
     * @brief Read data from chip
     */
    void read_cartridge();

    /**
     * @brief Slot to indicate that a block is about to be read / written
     */
    void read_block_start(unsigned int block_id, unsigned int nr_blocks);

    /**
     * @brief Slot to accept that a block is read / written
     */
    void read_block_done(unsigned int block_id, unsigned int nr_blocks);

    /*
     * @brief Signal that a read operation is finished
     */
    void read_result_ready();

    /****************************************************************************
     *  SIGNALS :: FLASH ROM
     ****************************************************************************/

    /**
     * @brief Put rom on flash cartridge
     */
    void flash_rom();

    /**
     * @brief Put rom on flash cartridge
     */
    void flash_bank();

    /**
     * @brief Erase the chip
     */
    void erase_chip();

    /**
     * @brief Slot to indicate that a page is about to be written
     */
    void flash_sector_start(unsigned int sector_id, unsigned int num_sectors);

    /**
     * @brief Slot to accept that a page is written
     */
    void flash_sector_done(unsigned int sector_id, unsigned int num_sectors);

    /*
     * @brief Signal that a flash operation is finished
     */
    void flash_result_ready();

    /*
     * @brief Signal that a flash operation is finished
     */
    void flash_chip_id_error(unsigned int chip_id);

    /**
     * @brief Slot to accept when a sector is about to be verified
     */
    void verify_block_start(unsigned int block_id, unsigned int nr_blocks);

    /**
     * @brief Slot to accept when a sector is verified
     */
    void verify_block_done(unsigned int block_id, unsigned int nr_blocks);

    /*
     * @brief Signal that a verified operation is finished
     */
    void verify_result_ready();

};
#endif // MAINWINDOW_H
