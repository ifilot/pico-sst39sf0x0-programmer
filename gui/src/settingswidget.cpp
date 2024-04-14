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
 * @param state
 */
void SettingsWidget::slot_settings_update(int state) {
    this->settings.setValue("SHOW_RETROROMS", QVariant(this->checkbox_retroroms->checkState()));
    this->settings.sync();

    emit signal_settings_update();
}

/**
 * @brief build layout for the hexviewer widget
 * @param layout
 */
void SettingsWidget::build_hexviewer_settings(QVBoxLayout* layout) {
    layout->addWidget(new QLabel("<b>Hexviewer theme</b>"));

    this->theme_combobox = new QComboBox();
    this->theme_combobox->setPlaceholderText("Select color scheme");
    this->theme_combobox->addItem("Default scheme", QVariant(
        {BACKGROUND_COLOR_DEFAULT,
         ADDRESS_COLOR_DEFAULT,
         HEADER_COLOR_DEFAULT,
         COLUMN_COLOR_DEFAULT,
         ALT_COLUMN_COLOR_DEFAULT,
         ASCII_COLOR_DEFAULT}));
    this->theme_combobox->addItem("Autum scheme", QVariant(
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
    this->theme_combobox->addItem("Bluey", QVariant(
        {0xFF5181a1,
         0xFFFFFFFF,
         0xFFFFFFFF,
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

    layout->addWidget(this->theme_combobox);
    connect(this->theme_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_theme_change(int)));

    QWidget* container = new QWidget();
    QGridLayout* gridlayout = new QGridLayout();
    container->setLayout(gridlayout);

    QColor color;

    for(unsigned int i=0; i<this->label_names.size(); i++) {
        QString keyword = this->label_names[i].toLower().replace(" ", "_");
        this->buttonpointers[i] = new QPushButton();
        this->buttonpointers[i]->setMinimumSize(QSize(55, 25));
        this->buttonpointers[i]->setMaximumSize(QSize(55, 25));
        color = QColor(settings.value(keyword, (uint32_t)this->default_colors[i]).toUInt());
        this->buttonpointers[i]->setStyleSheet(this->sheet.arg(color.name()));
        this->buttonpointers[i]->setText(color.name());
        gridlayout->addWidget(this->buttonpointers[i], i, 0);
        gridlayout->addWidget(new QLabel(this->label_names[i]), i, 1);
    }

    QSignalMapper* signal_mapper = new QSignalMapper (this);
    for(unsigned int i=0; i<this->label_names.size(); i++) {
        QString keyword = this->label_names[i].toLower().replace(" ", "_");
        connect(this->buttonpointers[i], SIGNAL(released()), signal_mapper, SLOT(map()));
        signal_mapper->setMapping(this->buttonpointers[i], keyword);
    }

    connect(signal_mapper, SIGNAL(mapped(QString)), this, SLOT(slot_change_color(QString))) ;

    layout->addWidget(container);
}

/**
 * @brief slot for single color change
 * @param color name
 */
void SettingsWidget::slot_change_color(const QString& name) {
    QPushButton *btn = nullptr;

    for(unsigned int i=0; i<this->label_names.size(); i++) {
        QString keyword = this->label_names[i].toLower().replace(" ", "_");
        if(keyword == name) {
            btn = this->buttonpointers[i];
            QColor defcol = QColor(btn->text());
            qDebug() << defcol.name();
            QColor color = QColorDialog::getColor(defcol);

            uint32_t value = (0xFF << 24) | (color.red() << 16) | (color.green() << 8) | (color.blue());
            qDebug() << "Changing color for " << name << " to " << color.name();

            btn->setStyleSheet(this->sheet.arg(color.name()));
            btn->setText(color.name());

            settings.setValue(name, value);
            settings.sync();

            emit signal_settings_update();
            return;
        }
    }
}

/**
 * @brief slot after changing color theme
 * @param state
 */
void SettingsWidget::slot_theme_change(int idx) {
    auto colors = this->theme_combobox->itemData(idx).toList();

    for(unsigned int i=0; i<this->label_names.size(); i++) {
        QString keyword = this->label_names[i].toLower().replace(" ", "_");
        uint32_t value = colors[i].toUInt();
        QColor color(value);
        this->buttonpointers[i]->setStyleSheet(this->sheet.arg(color.name()));
        settings.setValue(keyword, value);
    }

    settings.sync();
    emit signal_settings_update();
}
