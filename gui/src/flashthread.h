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

#ifndef FLASHTHREAD_H
#define FLASHTHREAD_H

#include <QMessageBox>
#include <QIcon>
#include <algorithm>
#include <QDebug>

#include "ioworker.h"
#include "romsizes.h"

/**
 * @brief class for Flashing a cartridge
 *
 * Currently, only support for the AT28C256 is implemented.
 */
class FlashThread : public IOWorker {

    Q_OBJECT
private:
    /**
     * @brief number of blocks to flash
     */
    uint16_t num_sectors = 0;

    /**
     * @brief Which bank to start writing at
     */
    uint8_t starting_bank;

    /**
     * @brief Whether to only flash those segments that are not 0xFF
     */
    bool quickflash = false;

public:
    /**
     * @brief Default constructor
     */
    FlashThread(bool _quickflash = false) : quickflash(_quickflash) {}

    /**
     * @brief Constructor allocating SerialInterface
     * @param _serial_interface
     */
    FlashThread(const std::shared_ptr<SerialInterface>& _serial_interface,
                uint8_t _starting_bank = 0,
                bool _quickflash = false) :
        IOWorker(_serial_interface),
        starting_bank(_starting_bank),
        quickflash(_quickflash) {}

    /**
     * @brief run cart flash routine
     */
    void run() override;

    /**
     * @brief get_num_pages
     * @return number of pages / blocks
     */
    inline uint16_t get_num_sectors() const {
        return this->num_sectors;
    }

    /**
     * @brief Get the starting bank
     * @return Bank to start reading from
     */
    inline uint8_t get_starting_bank() const {
        return this->starting_bank;
    }

private:
    /**
     * @brief run flash cart routine for a sst39sf0x0 chip
     */
    void flash_sst39sf0x0();

signals:
    /**
     * @brief signal when flash process is ready
     */
    void flash_result_ready();

    /**
     * @brief signal when new block is about to be written
     * @param block_id
     */
    void flash_sector_start(unsigned int sector_id, unsigned int nr_sectors);

    /**
     * @brief signal when new page is written
     * @param block_id
     */
    void flash_sector_done(unsigned int sector_id, unsigned int nr_sectors);
};

#endif // FLASHTHREAD_H
