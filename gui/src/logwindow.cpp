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

#include "config.h"
#include "logwindow.h"

LogWindow::LogWindow(const std::shared_ptr<QStringList>& _log_messages) :
    log_messages(_log_messages) {
    qDebug() << "Spawning Debug log window";

    // set icon and window title
    this->setWindowIcon(QIcon(":/assets/icon/eeprom_icon.ico"));
    this->setWindowTitle(tr(PROGRAM_NAME) + ": Debug log");

    qDebug() << "Building log window layout";
    QVBoxLayout* layout = new QVBoxLayout;
    this->setLayout(layout);
    QScrollArea* scroll_area = new QScrollArea(this);
    layout->addWidget(scroll_area);
    QVBoxLayout* layout_scrollarea = new QVBoxLayout;
    scroll_area->setLayout(layout_scrollarea);
    this->text_box = new QPlainTextEdit(this);
    layout_scrollarea->addWidget(this->text_box);
    this->text_box->setReadOnly(true);
    this->text_box->setOverwriteMode(false);

    for(const auto& line : *this->log_messages.get()) {
        this->text_box->appendPlainText(line);
    }
    this->linesread = this->log_messages->size();

    this->setMinimumHeight(256);
    this->setMinimumWidth(1024);
    this->hide();

    // setup timer that checks every second whether
    // the log messages have been updated
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update_log()));
    timer->start(1000);
}

void LogWindow::update_log() {
    int newsize = this->log_messages->size();
    for(int i=this->linesread; i<newsize; i++) {
        this->text_box->appendPlainText(this->log_messages->at(i));
    }
    this->linesread = newsize;
}
