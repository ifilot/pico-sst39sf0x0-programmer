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
    if(this->nr_rom_banks == 0) {
        uint16_t chip_id = this->serial_interface->get_chip_id();
        if((chip_id >> 8 & 0xFF) == 0xBF) {
            switch(chip_id & 0xFF) {
                case 0xB5:
                    this->nr_rom_banks = 128 * 1024 / BANKSIZE;
                break;
                case 0xB6:
                    this->nr_rom_banks = 256 * 1024 / BANKSIZE;
                break;
                case 0xB7:
                    this->nr_rom_banks = 512 * 1024 / BANKSIZE;
                break;
                default:
                    throw std::runtime_error("Unknown chip suffix: " + std::to_string(chip_id));
                break;
            }
        } else {
            throw std::runtime_error("Unknown chip id: " + std::to_string(chip_id));
        }
    }

    int numblocks = this->nr_rom_banks * BLOCKSPERBANK;

    // read blocks
    for(int i=0; i < numblocks; i++) {
        unsigned int block_id = i + this->starting_bank * BLOCKSPERBANK;
        emit(read_block_start(i, numblocks));
        auto blockdata = this->serial_interface->read_block(block_id);
        this->data.append(blockdata);
        emit(read_block_done(i, numblocks));
    }

    this->serial_interface->close_port();
    emit(read_result_ready());
}
