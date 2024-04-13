#ifndef HEXVIEWWIDGET_H
#define HEXVIEWWIDGET_H

#include <QAbstractScrollArea>
#include <QScrollBar>
#include <QPaintEvent>
#include <QByteArray>
#include <QPainter>
#include <QRect>
#include <QDebug>
#include <QMutexLocker>
#include <QSettings>

#include "colors.h"

class HexViewWidget : public QAbstractScrollArea
{
    Q_OBJECT

private:
    QSettings settings;

    QByteArray data;
    unsigned int charheight;
    unsigned int charwidth;
    unsigned int width;
    unsigned int height;
    unsigned int pos_addr;
    unsigned int pos_hex;
    unsigned int pos_ascii;
    unsigned int bytes_per_line = 16;

    static const unsigned int CHARACTERS_LINE = 16;
    static const unsigned int GAP_ADR_HEX = 16;
    static const unsigned int GAP_HEX_ASCII = 16;
    static const unsigned int GAP_HEADER = 5;
    static const unsigned int ADR_LENGTH = 10;

    QMutex lock;

public:
    explicit HexViewWidget(QWidget *parent = nullptr);

    void set_data(const QByteArray& _data) {
        QMutexLocker(&this->lock);
        this->data = _data;
        this->viewport()->update();
    }

    inline const QByteArray& get_data() const {
        return this->data;
    }

private:
    QSize get_widget_size() const;

    void update_positions();

protected:
    void paintEvent(QPaintEvent *event);

signals:

};

#endif // HEXVIEWWIDGET_H
