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

#ifndef _OPS_H
#define _OPS_H

#include <pico/stdlib.h>
#include <stdio.h>
#include "constants.h"

/**
 * @brief Load address in 74HC573 chips
 * 
 **/
void set_address(uint32_t addr);

/**
 * Set upper bits in address register
 **/
void set_address_high(uint32_t addr);

/**
 * Set lower 8 bits in address register
 **/
void set_address_low(uint32_t addr);

/**
 * Read single byte from address
 **/
uint8_t read_byte(uint32_t addr);

/**
 * Write byte at address
 **/
void write_byte(uint32_t addr, uint8_t val);

#endif