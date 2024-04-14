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
 * @param addr The full 19-bit address
 **/
void set_address(uint32_t addr);

/**
 * @brief Set the 3 upper address bits
 * 
 * @param addr The bits to set
 */
inline static void set_address_upper(uint32_t addr) {
    gpio_put_masked(0x0700, addr << 8);
    gpio_put(AUS, true);
    sleep_us(DELAY_ADDR);
    gpio_put(AUS, false);
}

/**
 * @brief Set higher 8 bits in address register
 *
 * @param addr The bits to set
 **/
inline static void set_address_high(uint32_t addr) {
    gpio_put_masked(0xFF00, addr << 8);
    gpio_put(AHS, true);
    sleep_us(DELAY_ADDR);
    gpio_put(AHS, false);
}

/**
 * @brief Set lower 8 bits in address register
 * 
 * @param addr The bits to set
 **/
inline static void set_address_low(uint32_t addr) {
    gpio_put_masked(0xFF00, addr << 8);
    gpio_put(ALS, true);
    sleep_us(DELAY_ADDR);
    gpio_put(ALS, false);
}

/**
 * @brief Read a single byte from address
 * 
 * @param addr full 19-bit address
 * @return uint8_t value at address
 */
uint8_t read_byte(uint32_t addr);

/**
 * @brief Write a byte at address
 * 
 * @param addr full 19-bit address
 * @param val  8-bit value to write
 */
void write_byte(uint32_t addr, uint8_t val);

/**
 * @brief Write byte at address while ignoring any upper bytes
 * 
 * @param addr 16-bit combination of lower and higher address bits
 * @param val  8-bit value to write
 * 
 * Assumes that:
 *  - Pins are already set for output
 *  - Chip is already active (CE) 
 **/
void write_byte_ignore_upper_fast(uint16_t addr, uint8_t val);

#endif