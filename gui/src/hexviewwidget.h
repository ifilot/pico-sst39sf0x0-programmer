/****************************************************************************
 *                                                                          *
 *   PICO-SST39SF0x0-FLASHER                                                *
 *   Copyright (C) 2023 Ivo Filot <ivo@ivofilot.nl>                         *
 *                                                                          *
 *   This program is free software: you can redistribute it and/or modify   *
 *   it under the terms of the GNU Lesser General Public License as         *
 *   published by the Free Software Foundation, either version 3 of the     *
 *   License, or (at your option) any later version.                        *
 *                                                                          *
 *   This program is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU General Public License for more details.                           *
 *                                                                          *
 *   You should have received a copy of the GNU General Public license      *
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>. *
 *                                                                          *
 ****************************************************************************/

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
#include <QFontDatabase>

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
    /**
     * @brief HexViewWidget::HexViewWidget
     * @param parent window
     */
    explicit HexViewWidget(QWidget *parent = nullptr);

    /**
     * @brief set data for the HexView class to display
     * @param _data data to display
     */
    void set_data(const QByteArray& _data);

    /**
     * @brief Grab data from HexViewer class
     * @return data
     */
    inline const QByteArray& get_data() const {
        return this->data;
    }

private:
    /**
     * @brief Calculate widget size
     * @return widget size
     */
    QSize get_widget_size() const;

    /**
     * @brief Calculate positions for the columns
     */
    void update_positions();

protected:
    /**
     * @brief Draw the contents of the HexViewer widget
     * @param event
     */
    void paintEvent(QPaintEvent *event);

signals:

};

#endif // HEXVIEWWIDGET_H
