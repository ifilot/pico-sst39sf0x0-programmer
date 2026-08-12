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

#include <QCheckBox>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QWidget>

#include <memory>

class LogWindow : public QWidget {
    Q_OBJECT

private:
    enum class Severity {
        Debug,
        Info,
        Warning,
        Error,
        Unknown
    };

    std::shared_ptr<QStringList> log_messages;
    QPlainTextEdit* text_box = nullptr;
    QLabel* status_label = nullptr;
    QLabel* live_badge = nullptr;
    QCheckBox* filter_debug = nullptr;
    QCheckBox* filter_info = nullptr;
    QCheckBox* filter_warning = nullptr;
    QCheckBox* filter_error = nullptr;
    QCheckBox* auto_scroll = nullptr;
    QPushButton* pause_button = nullptr;
    int linesread = 0;
    int first_visible_line = 0;

public:
    LogWindow() = default;

    /**
     * @brief Construct a live, filterable view of the captured log messages.
     * @param _log_messages shared list with captured log lines
     */
    explicit LogWindow(const std::shared_ptr<QStringList>& _log_messages);

private:
    Severity severity_of(const QString& line) const;
    bool severity_is_visible(Severity severity) const;
    void append_line(const QString& line);
    void rebuild_log();
    void update_status();
    void update_live_badge();

private slots:
    void update_log();
    void clear_log();
    void copy_log();
    void save_log();
    void toggle_pause(bool paused);
};

#endif // LOGWINDOW_H
