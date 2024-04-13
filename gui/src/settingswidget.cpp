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
}

void SettingsWidget::signal_settings_update(int state) {
    this->settings.setValue("SHOW_RETROROMS", QVariant(this->checkbox_retroroms->checkState()));
    this->settings.sync();

    emit signal_settings_update();
}
