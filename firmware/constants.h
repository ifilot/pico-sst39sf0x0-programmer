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

#define DELAY_READ 5
#define DELAY_WRITE 5
#define DELAY_ADDR 4
#define BOARD_ID "PICOSST39-v1.4.0"

#endif