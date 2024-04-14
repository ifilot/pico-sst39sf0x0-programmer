#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include <QSettings>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QColorDialog>
#include <QPushButton>
#include <QSignalMapper>
#include <QDebug>
#include <QComboBox>
#include <QList>
#include <array>

#include "config.h"
#include "colors.h"

class SettingsWidget : public QWidget
{
    Q_OBJECT

private:
    QSettings settings;
    QCheckBox *checkbox_retroroms;
    QComboBox *theme_combobox;

    const QStringList label_names = {
        "Address color",
        "Header color",
        "Column color",
        "Alt Column color",
        "Ascii color"
    };

    const std::vector<uint32_t> default_colors = {
        ADDRESS_COLOR_DEFAULT,
        HEADER_COLOR_DEFAULT,
        COLUMN_COLOR_DEFAULT,
        ALT_COLUMN_COLOR_DEFAULT,
        ASCII_COLOR_DEFAULT,
    };

    const QString sheet = R"(
        QPushButton {
            background-color: %1;
            color: #FFFFFF;
        }
    )";

    std::vector<QPushButton*> buttonpointers;

public:
    explicit SettingsWidget(QWidget *parent = nullptr);

signals:
    void signal_settings_update();

private:
    void build_hexviewer_settings(QVBoxLayout* layout);

private slots:
    void signal_settings_update(int state);

    void slot_change_color(const QString& name);

    void slot_theme_change(int);
};

#endif // SETTINGSWIDGET_H
