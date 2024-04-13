#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include <QSettings>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QLabel>

#include "config.h"

class SettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsWidget(QWidget *parent = nullptr);

private:
    QSettings settings;
    QCheckBox *checkbox_retroroms;

signals:
    void signal_settings_update();

private slots:
    void signal_settings_update(int state);
};

#endif // SETTINGSWIDGET_H
