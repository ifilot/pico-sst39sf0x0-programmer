/****************************************************************************
 *                                                                          *
 *   PICO-SST39SF0x0-FLASHER-FIRMWARE                                       *
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

#ifndef _ROUTINES_H
#define _ROUTINES_H

#include <pico/stdlib.h>
#include <pico/stdio.h>
#include "constants.h"
#include "ops.h"

/*
 * Read device ID and print it to console
 **/
void read_chip_id();

/*
 * Print data for first 32 bytes on chip
 **/
void printbytes32();

/*
 * @brief Read block of 256 bytes
 **/
void read_block(uint32_t block_id);

/*
 * @brief Read block of 0x1000 bytes, only use this function when
 * reading P2000T cartridgess!!
 **/
void read_p2k_cartridge_block(uint8_t block_id);

/*
 * @brief Read bank of 0x4000 bytes
 **/
void read_bank(uint8_t bank_id);

/*
 * @brief Read bank of 0x4000 bytes
 **/
void read_sector(uint32_t sector_id);

/*
 * @brief Write a sector of 0x1000 bytes
 **/
void write_sector(uint32_t sector_id);

/**
 * Poll a byte and count number of cycles wherein upper bit is not 1
 **/
uint16_t pollbyte(uint32_t addr);

/**
 * Write a block of data (256 bytes)
*/
void write_block(uint32_t block_id);

/**
 * @brief      Erase the complete chip
 */
void erase_chip();

/**
 * Erase sector (4kb)
*/
void erase_sector(uint32_t block_id);

/**
 * Return board identification string
 **/
void write_board_id();

/**
 * Generates CRC16 XMODEM checksum for data
*/
uint16_t crc16_xmodem(uint8_t *data, uint16_t length);

#endif // _ROUTINES_H