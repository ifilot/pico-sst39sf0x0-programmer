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

#include "picoflasherapplication.h"

/**
 * @brief Default constructor
 * @param argc number of command line argument
 * @param argv command line arguments
 */
PicoFlasherApplication::PicoFlasherApplication(int& argc, char** argv) :
QApplication(argc, argv) {}

/**
 * @brief notify
 * @param receiver
 * @param event
 * @return
 */
bool PicoFlasherApplication::notify(QObject* receiver, QEvent* event) {
    bool done = true;
    try {
        done = QApplication::notify(receiver, event);
    } catch (const std::exception& e) {
        this->throw_message_window(tr("Uncaught exception was thrown."),
                                   tr("An uncaught exception was thrown by the program. This is undiserable behavior and "
                                      "the program will be terminated. Please carefully read the error message "
                                      "and consult the manual or contact one of the developers.\n\nError:\n") + e.what());
        throw(e);
    }
    return done;
}


void PicoFlasherApplication::throw_message_window(const QString& title, const QString& message) {
    QMessageBox message_box;
    message_box.setText(message);
    message_box.setIcon(QMessageBox::Critical);
    message_box.setWindowTitle(title);
    message_box.setWindowIcon(QIcon(":/assets/icon/eeprom_icon.ico"));
    message_box.exec();
}
