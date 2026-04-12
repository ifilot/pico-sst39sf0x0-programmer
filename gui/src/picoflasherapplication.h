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

#ifndef PICOFLASHERAPPLICATION_H
#define PICOFLASHERAPPLICATION_H

#include <QApplication>
#include <QMessageBox>
#include <QIcon>

#include "config.h"

class PicoFlasherApplication : public QApplication
{
public:
    /**
     * @brief Default constructor
     * @param argc number of command line argument
     * @param argv command line arguments
     */
    PicoFlasherApplication(int& argc, char** argv);

    /**
     * @brief Deliver an event while catching uncaught exceptions.
     * @param receiver object receiving the event
     * @param event event to deliver
     * @return true when the event was handled
     */
    bool notify(QObject* receiver, QEvent* event);

private:
    /**
     * @brief Show a critical error window.
     * @param title message window title
     * @param message detailed error message
     */
    void throw_message_window(const QString& title, const QString& message);
};

#endif // PICOFLASHERAPPLICATION_H
