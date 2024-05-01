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

#include "readthread.h"

/**
 * @brief read the ROM from a cartridge
 *
 * This routine will be called when a thread containing this
 * class is runned
 */
void ReadThread::run() {
    this->serial_interface->open_port();

    // get chip id
    if(this->nr_banks == 0) {
        uint16_t chip_id = this->serial_interface->get_chip_id();
        if((chip_id >> 8 & 0xFF) == 0xBF) {
            switch(chip_id & 0xFF) {
                case 0xB5:
                    this->nr_banks = 128 / 16;
                break;
                case 0xB6:
                    this->nr_banks = 256 / 16;
                break;
                case 0xB7:
                    this->nr_banks = 512 / 16;
                break;
                default:
                    emit(thread_abort("Invalid suffix for chip id."));
                    this->serial_interface->close_port();
                    return;
                break;
            }
        } else {
            std::string chip_id_str = tr("0x%1%2").arg((uint16_t)chip_id >> 8, 2, 16, QChar('0')).arg((uint8_t)chip_id & 0xFF, 2, 16, QChar('0')).toStdString();
            emit(thread_abort(tr("Unknown chip id: %1").arg(chip_id_str.c_str())));
            this->serial_interface->close_port();
            return;
        }
    }

    // read blocks
    for(int i=0; i < this->nr_banks; i++) {
        emit(read_block_start(i, this->nr_banks));
        auto blockdata = this->serial_interface->read_bank(i + this->starting_bank);
        this->data.append(blockdata);
        emit(read_block_done(i, this->nr_banks));
    }

    this->serial_interface->close_port();
    emit(read_result_ready());
}
