#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QString>

class AboutDialog final : public QDialog
{
public:
    explicit AboutDialog(QWidget *parent = nullptr);

private:
    QString system_information() const;
};

#endif // ABOUTDIALOG_H
