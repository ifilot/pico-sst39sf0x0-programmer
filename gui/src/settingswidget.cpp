#include "settingswidget.h"

SettingsWidget::SettingsWidget(QWidget *parent)
    : QWidget{parent} {

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
    connect(this->checkbox_retroroms, SIGNAL(stateChanged(int)), this, SLOT(signal_settings_update(int)));

    this->build_hexviewer_settings(layout);
}

void SettingsWidget::signal_settings_update(int state) {
    this->settings.setValue("SHOW_RETROROMS", QVariant(this->checkbox_retroroms->checkState()));
    this->settings.sync();

    emit signal_settings_update();
}

void SettingsWidget::build_hexviewer_settings(QVBoxLayout* layout) {
    layout->addWidget(new QLabel("<b>Hexviewer</b>"));

    QWidget* container = new QWidget();
    QGridLayout* gridlayout = new QGridLayout();
    container->setLayout(gridlayout);

    QString sheet = R"(
        QPushButton {
           background-color: %1;
        }
    )";

    QColor color;

    gridlayout->addWidget(new QLabel("Text color"), 0, 1);
    this->text_color = new QPushButton();
    this->text_color->setMinimumSize(QSize(15, 15));
    this->text_color->setMaximumSize(QSize(15, 15));
    color = QColor(settings.value("text_color", (unsigned int)0x000000).toUInt());
    this->text_color->setStyleSheet(sheet.arg(color.name()));
    gridlayout->addWidget(this->text_color, 0, 0);

    gridlayout->addWidget(new QLabel("Header color"), 1, 1);
    this->header_color = new QPushButton();
    this->header_color->setMinimumSize(QSize(15, 15));
    this->header_color->setMaximumSize(QSize(15, 15));
    color = QColor(settings.value("header_color", (unsigned int)0x7a613d).toUInt());
    this->header_color->setStyleSheet(sheet.arg(color.name()));
    gridlayout->addWidget(this->header_color, 1, 0);

    gridlayout->addWidget(new QLabel("Alternate column color"), 2, 1);
    this->column_color = new QPushButton();
    this->column_color->setMinimumSize(QSize(15, 15));
    this->column_color->setMaximumSize(QSize(15, 15));
    color = QColor(settings.value("column_color", (unsigned int)0x888888).toUInt());
    this->column_color->setStyleSheet(sheet.arg(color.name()));
    gridlayout->addWidget(this->column_color, 2, 0);

    QSignalMapper* signal_mapper = new QSignalMapper (this);
    connect(this->text_color, SIGNAL(released()), signal_mapper, SLOT(map()));
    connect(this->header_color, SIGNAL(released()), signal_mapper, SLOT(map()));
    connect(this->column_color, SIGNAL(released()), signal_mapper, SLOT(map()));

    signal_mapper->setMapping(this->text_color, "text_color");
    signal_mapper->setMapping(this->header_color, "header_color");
    signal_mapper->setMapping(this->column_color, "column_header");

    connect(signal_mapper, SIGNAL(mapped(QString)), this, SLOT(slot_change_color(QString))) ;

    layout->addWidget(container);
}

void SettingsWidget::slot_change_color(const QString& name) {
    auto color = QColorDialog::getColor();

    QString sheet = R"(
        QPushButton {
           background-color: %1;
        }
    )";

    QPushButton *btn;

    if(name == "text_color") {
        btn = this->text_color;
    } else if(name == "header_color") {
        btn = this->header_color;
    } else if(name == "column_header") {
        btn = this->column_color;
    } else {
        throw std::runtime_error("Invalid keyword received for color change.");
    }

    uint32_t value = (0xFF << 24) | (color.red() << 16) | (color.green() << 8) | (color.blue());
    qDebug() << "Changing color for " << name << " to " << color.name();

    settings.setValue(name, value);
    settings.sync();

    btn->setStyleSheet(sheet.arg(color.name()));

    emit signal_settings_update();
}
