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
        "Background color",
        "Address color",
        "Header color",
        "Column color",
        "Alt Column color",
        "Ascii color"
    };

    const std::vector<uint32_t> default_colors = {
        BACKGROUND_COLOR_DEFAULT,
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
    /**
     * @brief SettingsWidget
     * @param parent widget
     */
    explicit SettingsWidget(QWidget *parent = nullptr);

signals:
    void signal_settings_update();

private:
    /**
     * @brief build layout for the hexviewer widget
     * @param layout
     */
    void build_hexviewer_settings(QVBoxLayout* layout);

private slots:
    /**
     * @brief slot for single color change
     * @param color name
     */
    void slot_change_color(const QString& name);

    /**
     * @brief slot after changing color theme
     * @param state
     */
    void slot_theme_change(int);

    /**
     * @brief slot to trigger settings update
     * @param state
     */
    void slot_settings_update(int);
};

#endif // SETTINGSWIDGET_H
