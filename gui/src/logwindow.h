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

#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QPlainTextEdit>
#include <QDebug>
#include <QTimer>
#include <QIcon>

class LogWindow : public QWidget {

Q_OBJECT

private:
    std::shared_ptr<QStringList> log_messages;
    QPlainTextEdit* text_box;
    int linesread = 0;

public:
    /**
     * @brief Default constructor.
     */
    LogWindow(){}

    /**
     * @brief Construct the debug log window.
     * @param _log_messages shared list with captured log lines
     */
    LogWindow(const std::shared_ptr<QStringList>& _log_messages);

private slots:
    /**
     * @brief Append new log messages to the text box.
     */
    void update_log();
};

#endif // LOGWINDOW_H
