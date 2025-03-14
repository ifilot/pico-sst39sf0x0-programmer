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

#ifndef _CONSTANTS_H
#define _CONSTANTS_H

/*
There is one 8-bit bus that is used to set the address registers. This
8-bit bus corresponds to GPIO8 - GPIO15.

GPIO16 - OE of all address registers
GPIO17 - LOAD for A0-A7
GPIO18 - LOAD for A8-A15
GPIO19 - LOAD for A16-A19
*/

#define ROE 16
#define ALS 17
#define AHS 18
#define AUS 19
#define PGM 20
#define CE  21
#define OE  22
#define LED_RD 26
#define LED_WR 27

#define DELAY_READ 3
#define DELAY_READ_VERIFY 1
#define DELAY_WRITE 1
#define DELAY_ADDR 1
#define BOARD_ID "PICOSST39-v1.6.0"

#define BLOCK_SIZE  0x100
#define SECTOR_SIZE 0x1000
#define BANK_SIZE   0x4000

#define NOP __asm volatile ("nop\n");
#define NOP50 __asm volatile ("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");

#endif
