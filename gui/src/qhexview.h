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

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * SOURCE CODE OBTAINED FROM: https://github.com/virinext/QHexView
 *!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */

#ifndef Q_HEX_VIEWER_H_
#define Q_HEX_VIEWER_H_

#include <QAbstractScrollArea>
#include <QByteArray>
#include <QFile>
#include <QMutex>
#include <QScrollBar>
#include <QPainter>
#include <QSize>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QClipboard>
#include <QApplication>
#include <QDebug>
#include <stdexcept>

class QHexView: public QAbstractScrollArea

{
    Q_OBJECT
    public:
        class DataStorage
        {
            public:
                virtual ~DataStorage() {};
                virtual QByteArray getData(std::size_t position, std::size_t length) = 0;
                virtual std::size_t size() = 0;
        };


        class DataStorageArray: public DataStorage
        {
            public:
                DataStorageArray(const QByteArray &arr);
                virtual QByteArray getData(std::size_t position, std::size_t length);
                virtual std::size_t size();
            private:
                QByteArray    m_data;
        };

        class DataStorageFile: public DataStorage
        {
            public:
                DataStorageFile(const QString &fileName);
                virtual QByteArray getData(std::size_t position, std::size_t length);
                virtual std::size_t size();
            private:
                QFile      m_file;
        };



        QHexView(QWidget *parent = 0);
        ~QHexView();

        inline const QByteArray& get_data() const {
            if(this->m_pdata != nullptr) {
                return this->m_pdata->getData(0, this->m_pdata->size());
            } else {
                return QByteArray();
            }
        }

    public slots:
        void setData(DataStorage *pData);
        void clear();
        void showFromOffset(std::size_t offset);
        void setSelected(std::size_t offset, std::size_t length);

    protected:
        void paintEvent(QPaintEvent *event);
        void keyPressEvent(QKeyEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void mousePressEvent(QMouseEvent *event);
    private:
        QMutex                m_dataMtx;
        DataStorage          *m_pdata;
        std::size_t           m_posAddr;
        std::size_t           m_posHex;
        std::size_t           m_posAscii;
        std::size_t           m_charWidth;
        std::size_t           m_charHeight;


        std::size_t           m_selectBegin;
        std::size_t           m_selectEnd;
        std::size_t           m_selectInit;
        std::size_t           m_cursorPos;
        std::size_t           m_bytesPerLine;

        QSize fullSize() const;
        void updatePositions();
        void resetSelection();
        void resetSelection(std::size_t pos);
        void setSelection(std::size_t pos);
        void ensureVisible();
        void setCursorPos(std::size_t pos);
        std::size_t cursorPos(const QPoint &position);
};

#endif
