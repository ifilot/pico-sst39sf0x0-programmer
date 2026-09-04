#include "settingswidget.h"

/**
 * @brief SettingsWidget
 * @param parent widget
 */
SettingsWidget::SettingsWidget(QWidget *parent)
    : QWidget{parent} {

    // set button points
    this->buttonpointers = std::vector<QPushButton*>(this->label_names.size(), nullptr);

    // set icon and window title
    this->setWindowIcon(QIcon(":/assets/icon/eeprom_icon.ico"));
    this->setWindowTitle(tr(PROGRAM_NAME) + ": Settings");
    this->setMinimumWidth(400);

    QVBoxLayout *layout = new QVBoxLayout();
    this->setLayout(layout);

    layout->addWidget(new QLabel("<b>Settings</b>"));

    this->checkbox_retroroms = new QCheckBox("Show retroroms (Philips P2000T, Commodore C64)");
    this->checkbox_retroroms->setChecked(this->settings.value("SHOW_RETROROMS", QVariant(true)).toBool());
    layout->addWidget(this->checkbox_retroroms);
    connect(this->checkbox_retroroms, SIGNAL(stateChanged(int)), this, SLOT(slot_settings_update(int)));

    this->build_hexviewer_settings(layout);
}

/**
 * @brief slot to trigger settings update
 * @param state checkbox state
 */
void SettingsWidget::slot_settings_update(int state) {
    Q_UNUSED(state);
    this->settings.setValue("SHOW_RETROROMS", QVariant(this->checkbox_retroroms->checkState()));
    this->settings.sync();

    emit signal_settings_update();
}

/**
 * @brief build layout for the hexviewer widget
 * @param layout target layout
 */
void SettingsWidget::build_hexviewer_settings(QVBoxLayout* layout) {
    layout->addWidget(new QLabel("<b>Hexviewer theme</b>"));

    this->theme_combobox = new QComboBox();
    this->theme_combobox->setObjectName(QStringLiteral("hexViewerThemeComboBox"));
    this->theme_combobox->setPlaceholderText("Select color scheme");
    this->theme_combobox->addItem("Default scheme", QVariant(
        {BACKGROUND_COLOR_DEFAULT,
         ADDRESS_COLOR_DEFAULT,
         HEADER_COLOR_DEFAULT,
         COLUMN_COLOR_DEFAULT,
         ALT_COLUMN_COLOR_DEFAULT,
         ASCII_COLOR_DEFAULT}));
    this->theme_combobox->addItem("Autumn scheme", QVariant(
        {0xFFFFFFFF,
         0xFFFF0000,
         0xFFFF0000,
         0xFF2c2c2c,
         0xFFFF5500,
         0xFF686868}));
    this->theme_combobox->addItem("Solarized Grey", QVariant(
        {0xFFFFFFFF,
         0xFF002b36,
         0xFF002b36,
         0xFF586375,
         0xFF839496,
         0xFF93a1a1}));
    this->theme_combobox->addItem("Solarized Colorful", QVariant(
        {0xFFFFFFFF,
         0xFFcb4b16,
         0xFFcb4b16,
         0xFFd33682,
         0xFF6c71c4,
         0xFF2aa198}));
    this->theme_combobox->addItem("Solarized Light", QVariant(
        {0xFFfdf6e3,
         0xFF268bd2,
         0xFFd33682,
         0xFF073642,
         0xFF93a1a1,
         0xFF2aa198}));
    this->theme_combobox->addItem("Bluey", QVariant(
        {0xFF5181a1,
         0xFFfdf6e3,
         0xFFfdf6e3,
         0xFFFFFFFF,
         0xFFffaa7f,
         0xFFFFFF7f}));
    this->theme_combobox->addItem("SAA5050", QVariant(
        {0xFF323232,
         0xFF00FFFF,
         0xFFFF00FF,
         0xFFFFFFFF,
         0xFF00FF00,
         0xFF00FFFF}));
    this->theme_combobox->addItem("Dracula", QVariant(
        {0xFF282A36,
         0xFF8BE9FD,
         0xFFFF79C6,
         0xFFF8F8F2,
         0xFF50FA7B,
         0xFFF1FA8C}));
    this->theme_combobox->addItem("Dracula Alucard (Light)", QVariant(
        {0xFFFFFBEB,
         0xFF036A96,
         0xFFA3144D,
         0xFF1F1F1F,
         0xFF14710A,
         0xFF644AC9}));
    this->theme_combobox->addItem("Nord", QVariant(
        {0xFF2E3440,
         0xFF88C0D0,
         0xFFEBCB8B,
         0xFFECEFF4,
         0xFFA3BE8C,
         0xFF81A1C1}));
    this->theme_combobox->addItem("Gruvbox Dark", QVariant(
        {0xFF282828,
         0xFF83A598,
         0xFFFABD2F,
         0xFFEBDBB2,
         0xFFB8BB26,
         0xFF8EC07C}));
    this->theme_combobox->addItem("Catppuccin Mocha", QVariant(
        {0xFF1E1E2E,
         0xFF89B4FA,
         0xFFCBA6F7,
         0xFFCDD6F4,
         0xFFA6E3A1,
         0xFF89DCEB}));

    layout->addWidget(this->theme_combobox);
    connect(this->theme_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_theme_change(int)));

    QWidget* container = new QWidget();
    QGridLayout* gridlayout = new QGridLayout();
    container->setLayout(gridlayout);

    QColor color;

    for(unsigned int i=0; i<this->label_names.size(); i++) {
        QString keyword = this->label_names[i].toLower().replace(" ", "_");
        this->buttonpointers[i] = new QPushButton();
        this->buttonpointers[i]->setObjectName(QStringLiteral("colorSwatch_") + keyword);
        this->buttonpointers[i]->setMinimumSize(QSize(72, 25));
        this->buttonpointers[i]->setMaximumHeight(25);
        color = QColor(settings.value(keyword, (uint32_t)this->default_colors[i]).toUInt());
        update_color_button(this->buttonpointers[i], color);
        gridlayout->addWidget(this->buttonpointers[i], i, 0);
        gridlayout->addWidget(new QLabel(this->label_names[i]), i, 1);
    }

    QSignalMapper* signal_mapper = new QSignalMapper (this);
    for(unsigned int i=0; i<this->label_names.size(); i++) {
        QString keyword = this->label_names[i].toLower().replace(" ", "_");
        connect(this->buttonpointers[i], SIGNAL(released()), signal_mapper, SLOT(map()));
        signal_mapper->setMapping(this->buttonpointers[i], keyword);
    }

    connect(signal_mapper, SIGNAL(mappedString(QString)), this, SLOT(slot_change_color(QString))) ;

    layout->addWidget(container);
}

/**
 * @brief slot for single color change
 * @param name color setting key
 */
void SettingsWidget::slot_change_color(const QString& name) {
    QPushButton *btn = nullptr;

    for(unsigned int i=0; i<this->label_names.size(); i++) {
        QString keyword = this->label_names[i].toLower().replace(" ", "_");
        if(keyword == name) {
            btn = this->buttonpointers[i];
            QColor defcol = btn->property("swatchColor").value<QColor>();
            qDebug() << defcol.name();
            QColor color = QColorDialog::getColor(defcol);

            if(!color.isValid()) {
                return;
            }

            uint32_t value = (0xFF << 24) | (color.red() << 16) | (color.green() << 8) | (color.blue());
            qDebug() << "Changing color for " << name << " to " << color.name();

            update_color_button(btn, color);

            settings.setValue(name, value);
            settings.sync();

            emit signal_settings_update();
            return;
        }
    }
}

/**
 * @brief slot after changing color theme
 * @param idx selected theme index
 */
void SettingsWidget::slot_theme_change(int idx) {
    auto colors = this->theme_combobox->itemData(idx).toList();

    for(unsigned int i=0; i<this->label_names.size(); i++) {
        QString keyword = this->label_names[i].toLower().replace(" ", "_");
        uint32_t value = colors[i].toUInt();
        QColor color(value);
        update_color_button(this->buttonpointers[i], color);
        settings.setValue(keyword, value);
    }

    settings.sync();
    emit signal_settings_update();
}

void SettingsWidget::update_color_button(QPushButton* button, const QColor& color) {
    button->setProperty("swatchColor", color);
    button->setText(color.name(QColor::HexRgb));
    button->setStyleSheet(colorSwatchStyleSheet(color));
}
