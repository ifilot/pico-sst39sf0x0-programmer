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
#include <qDebug>

#include "config.h"

class SettingsWidget : public QWidget
{
    Q_OBJECT

private:
    QSettings settings;
    QCheckBox *checkbox_retroroms;
    QPushButton *text_color;
    QPushButton *header_color;
    QPushButton *column_color;

public:
    explicit SettingsWidget(QWidget *parent = nullptr);

signals:
    void signal_settings_update();

private:
    void build_hexviewer_settings(QVBoxLayout* layout);

private slots:
    void signal_settings_update(int state);

    void slot_change_color(const QString& name);
};

#endif // SETTINGSWIDGET_H
