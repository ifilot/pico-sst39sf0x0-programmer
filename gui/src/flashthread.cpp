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

#include "flashthread.h"

/**
 * @brief run cart flash routine
 */
void FlashThread::run() {
    flash_sst39sf0x0();
}

/**
 * @brief run flash cart routine for a sst39sf0x0 chip
 */
void FlashThread::flash_sst39sf0x0() {
    qDebug() << "Attempting to flash " << this->data.size() << " bytes.";
    this->num_sectors = this->data.size() / SECTORSIZE;

    this->serial_interface->open_port();

    // check that the chip id is correct
    unsigned int chip_id = this->serial_interface->get_chip_id();
    if(!(chip_id == 0xBFB5 || chip_id == 0xBFB6 || chip_id == 0xBFB7)) {
        emit(flash_chip_id_error(chip_id));
        return;
    }

    for(unsigned int i=0; i<this->num_sectors; i++) {
        unsigned int sector_id = i + this->starting_bank * SECTORSPERBANK; // shift block id by the starting bank

        emit(flash_sector_start(sector_id, this->num_sectors));

        // erase the sector and write new data to it
        this->serial_interface->erase_sector(sector_id << 4);
        this->serial_interface->burn_sector(sector_id, this->data.mid(i * SECTORSIZE, SECTORSIZE));

        emit(flash_sector_done(i, this->num_sectors));
    }

    this->serial_interface->close_port();

    emit(flash_result_ready());
}
