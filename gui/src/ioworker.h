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

#ifndef IOWORKER_H
#define IOWORKER_H

#include <QThread>
#include <iostream>
#include <memory>

#include "serial_interface.h"

/**
 * @brief General worker thread that manages I/O with the cardreader
 *
 * This is a parent class containing general routines for reading and
 * writing data to cartridges.
 *
 * The following derivative classes exist:
 * - ReaderThread : Reads ROM from a cartridge
 * - FlashThread: Flashes a ROM to a cartridge
 */
class IOWorker : public QThread {

    Q_OBJECT

protected:
    std::string portname;           // port address
    QByteArray data;                // data package
    unsigned int nr_sectors;        // number of sectors to read
    uint8_t mapper_type;            // bank switching mapper id

    // use shared pointer to SerialInterface, this serial
    // interface should be created in the MainWindow class
    std::shared_ptr<SerialInterface> serial_interface;

public:
    IOWorker() {}

    IOWorker(const std::shared_ptr<SerialInterface>& _serial_interface) :
        serial_interface(_serial_interface)
    {}

    /**
     * @brief set the address of the serial
     * @param serial port address
     */
    inline void set_serial_port(const std::string& _portname) {
        this->portname = _portname;
    }

    /**
     * @brief set data package (rom / ram)
     * @param data package
     *
     * This routines is used for writing data to a cartridge
     */
    inline void set_data(const QByteArray& _data) {
        this->data = _data;
    }

    /**
     * @brief set_data_package
     * @param _nr_sectors
     * @param _mapper_type
     */
    inline void set_data_package(unsigned int _nr_sectors, uint8_t _mapper_type) {
        this->mapper_type = _mapper_type;
        this->nr_sectors = _nr_sectors;
        this->data.clear();
    }

    /**
     * @brief get data package
     * @return data package
     *
     * This routine is used for reading data from a cartridge
     */
    inline const QByteArray& get_data() const {
        return this->data;
    }

    /**
     * @brief run routine
     *
     * Every derivative class needs to implement this routine
     */
    virtual void run() = 0;

    /**
     * @brief Destructor of base class needs to be virtual
     */
    virtual ~IOWorker() {}

protected:

};

#endif // IOWORKER_H
