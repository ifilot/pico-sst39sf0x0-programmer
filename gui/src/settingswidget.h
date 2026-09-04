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
#include "colorcontrast.h"
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

    std::vector<QPushButton*> buttonpointers;

    /**
     * @brief Update a swatch without using its visible label as color storage.
     */
    static void update_color_button(QPushButton* button, const QColor& color);

public:
    /**
     * @brief SettingsWidget
     * @param parent widget
     */
    explicit SettingsWidget(QWidget *parent = nullptr);

signals:
    /**
     * @brief Signal emitted after settings have changed.
     */
    void signal_settings_update();

private:
    /**
     * @brief build layout for the hexviewer widget
     * @param layout target layout
     */
    void build_hexviewer_settings(QVBoxLayout* layout);

private slots:
    /**
     * @brief slot for single color change
     * @param name color setting key
     */
    void slot_change_color(const QString& name);

    /**
     * @brief slot after changing color theme
     * @param idx selected theme index
     */
    void slot_theme_change(int);

    /**
     * @brief slot to trigger settings update
     * @param state checkbox state
     */
    void slot_settings_update(int);
};

#endif // SETTINGSWIDGET_H
