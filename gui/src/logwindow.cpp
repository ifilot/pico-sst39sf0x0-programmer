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

#include <QApplication>
#include <QClipboard>
#include <QDateTime>
#include <QFileDialog>
#include <QFontDatabase>
#include <QFrame>
#include <QHBoxLayout>
#include <QSaveFile>
#include <QScrollBar>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QStringConverter>
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>

namespace {

class LogHighlighter final : public QSyntaxHighlighter {
public:
    explicit LogHighlighter(QTextDocument* document)
        : QSyntaxHighlighter(document) {}

protected:
    void highlightBlock(const QString& text) override {
        QTextCharFormat timestamp;
        timestamp.setForeground(QColor("#748091"));
        const int severity_start = text.indexOf(QStringLiteral(" ["));
        if(severity_start > 0) {
            setFormat(0, severity_start, timestamp);
        }

        struct HighlightRule {
            const char* marker;
            const char* foreground;
            const char* background;
        };

        static const HighlightRule rules[] = {
            {"[DEBUG]", "#a7b0bd", "#242b35"},
            {"[INFO]", "#79b8ff", "#17324d"},
            {"[WARNING]", "#f3c969", "#493b18"},
            {"[CRITICAL]", "#ff8585", "#4a2024"},
            {"[FATAL]", "#ff8585", "#4a2024"}
        };

        for(const HighlightRule& rule : rules) {
            const QString marker = QString::fromLatin1(rule.marker);
            const int marker_start = text.indexOf(marker);
            if(marker_start < 0) {
                continue;
            }

            QTextCharFormat format;
            format.setForeground(QColor(rule.foreground));
            format.setBackground(QColor(rule.background));
            format.setFontWeight(QFont::DemiBold);
            setFormat(marker_start, marker.size(), format);
            break;
        }
    }
};

QFrame* vertical_separator(QWidget* parent) {
    auto* separator = new QFrame(parent);
    separator->setFrameShape(QFrame::VLine);
    separator->setFrameShadow(QFrame::Sunken);
    return separator;
}

} // namespace

LogWindow::LogWindow(const std::shared_ptr<QStringList>& _log_messages)
    : log_messages(_log_messages) {
    setObjectName(QStringLiteral("diagnosticLogWindow"));
    setWindowIcon(QIcon(QStringLiteral(":/assets/icon/eeprom_icon.ico")));
    setWindowTitle(tr(PROGRAM_NAME) + tr(": Diagnostic Log"));
    setMinimumSize(760, 420);
    resize(1000, 620);

    auto* root_layout = new QVBoxLayout(this);
    root_layout->setContentsMargins(20, 18, 20, 18);
    root_layout->setSpacing(12);

    auto* heading_layout = new QHBoxLayout;
    auto* heading_text_layout = new QVBoxLayout;
    heading_text_layout->setSpacing(2);

    auto* title = new QLabel(tr("Diagnostic Log"), this);
    title->setObjectName(QStringLiteral("diagnosticLogTitle"));
    auto* subtitle = new QLabel(
        tr("Live application and device messages for troubleshooting."), this);
    subtitle->setObjectName(QStringLiteral("diagnosticLogSubtitle"));
    heading_text_layout->addWidget(title);
    heading_text_layout->addWidget(subtitle);
    heading_layout->addLayout(heading_text_layout);
    heading_layout->addStretch();

    live_badge = new QLabel(this);
    live_badge->setObjectName(QStringLiteral("diagnosticLogLiveBadge"));
    live_badge->setAlignment(Qt::AlignCenter);
    heading_layout->addWidget(live_badge);
    root_layout->addLayout(heading_layout);

    auto* toolbar = new QFrame(this);
    toolbar->setObjectName(QStringLiteral("diagnosticLogToolbar"));
    auto* toolbar_layout = new QHBoxLayout(toolbar);
    toolbar_layout->setContentsMargins(10, 7, 10, 7);
    toolbar_layout->setSpacing(9);

    auto* clear_button = new QPushButton(tr("Clear"), toolbar);
    clear_button->setObjectName(QStringLiteral("buttonLogClear"));
    clear_button->setToolTip(tr("Clear the messages currently shown"));
    auto* copy_button = new QPushButton(tr("Copy all"), toolbar);
    copy_button->setObjectName(QStringLiteral("buttonLogCopy"));
    copy_button->setToolTip(tr("Copy all visible messages to the clipboard"));
    auto* save_button = new QPushButton(tr("Save…"), toolbar);
    save_button->setObjectName(QStringLiteral("buttonLogSave"));
    save_button->setToolTip(tr("Save the visible log to a text file"));

    toolbar_layout->addWidget(clear_button);
    toolbar_layout->addWidget(copy_button);
    toolbar_layout->addWidget(save_button);
    toolbar_layout->addWidget(vertical_separator(toolbar));

    auto* show_label = new QLabel(tr("Show:"), toolbar);
    show_label->setObjectName(QStringLiteral("diagnosticLogShowLabel"));
    toolbar_layout->addWidget(show_label);

    filter_debug = new QCheckBox(tr("Debug"), toolbar);
    filter_info = new QCheckBox(tr("Info"), toolbar);
    filter_warning = new QCheckBox(tr("Warnings"), toolbar);
    filter_error = new QCheckBox(tr("Errors"), toolbar);
    filter_debug->setObjectName(QStringLiteral("checkLogDebug"));
    filter_info->setObjectName(QStringLiteral("checkLogInfo"));
    filter_warning->setObjectName(QStringLiteral("checkLogWarning"));
    filter_error->setObjectName(QStringLiteral("checkLogError"));
    for(QCheckBox* filter : {filter_debug, filter_info, filter_warning, filter_error}) {
        filter->setChecked(true);
        toolbar_layout->addWidget(filter);
    }

    toolbar_layout->addStretch();
    pause_button = new QPushButton(tr("Pause"), toolbar);
    pause_button->setObjectName(QStringLiteral("buttonLogPause"));
    pause_button->setCheckable(true);
    pause_button->setToolTip(tr("Temporarily stop updating the view"));
    auto_scroll = new QCheckBox(tr("Auto-scroll"), toolbar);
    auto_scroll->setObjectName(QStringLiteral("checkLogAutoScroll"));
    auto_scroll->setChecked(true);
    toolbar_layout->addWidget(pause_button);
    toolbar_layout->addWidget(auto_scroll);
    root_layout->addWidget(toolbar);

    text_box = new QPlainTextEdit(this);
    text_box->setObjectName(QStringLiteral("diagnosticLogText"));
    text_box->setReadOnly(true);
    text_box->setUndoRedoEnabled(false);
    text_box->setLineWrapMode(QPlainTextEdit::NoWrap);
    text_box->setMaximumBlockCount(10000);
    text_box->setPlaceholderText(tr("No diagnostic messages have been recorded yet."));
    text_box->setContextMenuPolicy(Qt::DefaultContextMenu);
    root_layout->addWidget(text_box, 1);

    const int font_id = QFontDatabase::addApplicationFont(
        QStringLiteral(":/assets/fonts/Consolas.ttf"));
    QString font_family = QStringLiteral("Consolas");
    const QStringList families = QFontDatabase::applicationFontFamilies(font_id);
    if(!families.isEmpty()) {
        font_family = families.constFirst();
    }
    QFont log_font(font_family, 10);
    log_font.setStyleHint(QFont::TypeWriter);
    text_box->setFont(log_font);
    new LogHighlighter(text_box->document());

    status_label = new QLabel(this);
    status_label->setObjectName(QStringLiteral("diagnosticLogStatus"));
    root_layout->addWidget(status_label);

    setStyleSheet(QStringLiteral(R"(
        #diagnosticLogWindow {
            background: #f5f7fa;
            color: #18212f;
        }
        #diagnosticLogTitle {
            color: #18212f;
            font-size: 18px;
            font-weight: 600;
        }
        #diagnosticLogSubtitle, #diagnosticLogStatus {
            color: #667085;
        }
        #diagnosticLogToolbar {
            color: #18212f;
            background: #ffffff;
            border: 1px solid #dce2ea;
            border-radius: 6px;
        }
        #diagnosticLogShowLabel {
            color: #596579;
            font-weight: 600;
        }
        #diagnosticLogLiveBadge {
            color: #176b42;
            background: #dff5e9;
            border: 1px solid #b8e5cc;
            border-radius: 11px;
            font-weight: 600;
            padding: 3px 10px;
        }
        #diagnosticLogText {
            color: #d8dee9;
            background: #111821;
            border: 1px solid #293342;
            border-radius: 6px;
            padding: 10px;
            selection-background-color: #315a82;
        }
        QPushButton {
            padding: 4px 10px;
        }
        QPushButton:checked {
            color: #8a4b08;
            background: #fff0ce;
            border: 1px solid #e4b65a;
            border-radius: 3px;
        }
    )"));

    connect(clear_button, &QPushButton::clicked, this, &LogWindow::clear_log);
    connect(copy_button, &QPushButton::clicked, this, &LogWindow::copy_log);
    connect(save_button, &QPushButton::clicked, this, &LogWindow::save_log);
    connect(pause_button, &QPushButton::toggled, this, &LogWindow::toggle_pause);
    for(QCheckBox* filter : {filter_debug, filter_info, filter_warning, filter_error}) {
        connect(filter, &QCheckBox::toggled, this, &LogWindow::rebuild_log);
    }

    rebuild_log();
    update_live_badge();

    auto* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &LogWindow::update_log);
    timer->start(250);
}

LogWindow::Severity LogWindow::severity_of(const QString& line) const {
    if(line.contains(QStringLiteral("[DEBUG]"))) {
        return Severity::Debug;
    }
    if(line.contains(QStringLiteral("[INFO]"))) {
        return Severity::Info;
    }
    if(line.contains(QStringLiteral("[WARNING]"))) {
        return Severity::Warning;
    }
    if(line.contains(QStringLiteral("[CRITICAL]")) ||
       line.contains(QStringLiteral("[FATAL]"))) {
        return Severity::Error;
    }
    return Severity::Unknown;
}

bool LogWindow::severity_is_visible(Severity severity) const {
    switch(severity) {
    case Severity::Debug:
        return filter_debug->isChecked();
    case Severity::Info:
        return filter_info->isChecked();
    case Severity::Warning:
        return filter_warning->isChecked();
    case Severity::Error:
        return filter_error->isChecked();
    case Severity::Unknown:
        return true;
    }
    return true;
}

void LogWindow::append_line(const QString& line) {
    if(!severity_is_visible(severity_of(line))) {
        return;
    }

    // Pad the severity field so that message text starts in a stable column.
    QString formatted_line = line;
    formatted_line.replace(QStringLiteral(" [INFO] "), QStringLiteral(" [INFO]     "));
    formatted_line.replace(QStringLiteral(" [DEBUG] "), QStringLiteral(" [DEBUG]    "));
    formatted_line.replace(QStringLiteral(" [WARNING] "), QStringLiteral(" [WARNING]  "));
    formatted_line.replace(QStringLiteral(" [FATAL] "), QStringLiteral(" [FATAL]    "));
    text_box->appendPlainText(formatted_line);
}

void LogWindow::rebuild_log() {
    const bool keep_at_bottom = auto_scroll->isChecked();
    text_box->clear();

    const int available = log_messages ? log_messages->size() : 0;
    const int end = pause_button && pause_button->isChecked()
        ? qMin(linesread, available)
        : available;
    for(int i = first_visible_line; i < end; ++i) {
        append_line(log_messages->at(i));
    }
    linesread = end;

    if(keep_at_bottom) {
        text_box->verticalScrollBar()->setValue(
            text_box->verticalScrollBar()->maximum());
    }
    update_status();
}

void LogWindow::update_log() {
    const int new_size = log_messages ? log_messages->size() : 0;
    if(pause_button->isChecked()) {
        update_status();
        return;
    }

    if(new_size < linesread || first_visible_line > new_size) {
        first_visible_line = 0;
        rebuild_log();
        return;
    }

    const int old_scroll_value = text_box->verticalScrollBar()->value();
    for(int i = linesread; i < new_size; ++i) {
        append_line(log_messages->at(i));
    }
    linesread = new_size;

    if(auto_scroll->isChecked()) {
        text_box->verticalScrollBar()->setValue(
            text_box->verticalScrollBar()->maximum());
    } else {
        text_box->verticalScrollBar()->setValue(old_scroll_value);
    }
    update_status();
}

void LogWindow::clear_log() {
    first_visible_line = log_messages ? log_messages->size() : 0;
    linesread = first_visible_line;
    text_box->clear();
    update_status();
}

void LogWindow::copy_log() {
    QApplication::clipboard()->setText(text_box->toPlainText());
}

void LogWindow::save_log() {
    const QString suggested_name = QStringLiteral("pico-flasher-log-%1.txt")
        .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss")));
    const QString filename = QFileDialog::getSaveFileName(
        this, tr("Save Diagnostic Log"), suggested_name, tr("Text files (*.txt);;All files (*)"));
    if(filename.isEmpty()) {
        return;
    }

    QSaveFile file(filename);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << text_box->toPlainText() << '\n';
    file.commit();
}

void LogWindow::toggle_pause(bool paused) {
    pause_button->setText(paused ? tr("Resume") : tr("Pause"));
    update_live_badge();
    if(!paused) {
        update_log();
    } else {
        update_status();
    }
}

void LogWindow::update_status() {
    const int available = log_messages ? log_messages->size() : 0;
    const int displayed_end = pause_button && pause_button->isChecked()
        ? qMin(linesread, available)
        : available;
    int messages = 0;
    int visible = 0;
    int warnings = 0;
    int errors = 0;

    for(int i = qMin(first_visible_line, displayed_end); i < displayed_end; ++i) {
        const Severity severity = severity_of(log_messages->at(i));
        ++messages;
        if(severity_is_visible(severity)) {
            ++visible;
        }
        if(severity == Severity::Warning) {
            ++warnings;
        } else if(severity == Severity::Error) {
            ++errors;
        }
    }

    QString status = tr("%1 %2 · %3 %4 · %5 %6")
        .arg(messages)
        .arg(messages == 1 ? tr("message") : tr("messages"))
        .arg(warnings)
        .arg(warnings == 1 ? tr("warning") : tr("warnings"))
        .arg(errors)
        .arg(errors == 1 ? tr("error") : tr("errors"));
    if(visible != messages) {
        status.prepend(tr("Showing %1 of ").arg(visible));
    }
    const int pending = qMax(0, available - displayed_end);
    if(pending > 0) {
        status += tr(" · %1 pending").arg(pending);
    }
    status_label->setText(status);
}

void LogWindow::update_live_badge() {
    const bool paused = pause_button && pause_button->isChecked();
    live_badge->setText(paused ? tr("Ⅱ Paused") : tr("● Live"));
    live_badge->setStyleSheet(paused
        ? QStringLiteral("color:#8a4b08;background:#fff0ce;border-color:#e4c172;")
        : QString());
}
