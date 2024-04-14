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

#include "ops.h"

/**
 * Load address in 74HC573 chips
 **/
void set_address(uint32_t addr) {
    gpio_put(ROE, true); // disable address output

    uint32_t addrl = addr & 0xFF;
    uint32_t addrh = (addr >> 8) & 0xFF;
    uint32_t addru = (addr >> 16) & 0xFF;

    set_address_upper(addru);
    set_address_high(addrh);
    set_address_low(addrl);

    gpio_put(ROE, false); // enable address output
}

/**
 * @brief Read a single byte from address
 * 
 * @param addr full 19-bit address
 * @return uint8_t value at address
 */
uint8_t read_byte(uint32_t addr) {
    // set pins 0-7 to input
    gpio_set_dir_in_masked(0x000000FF);

    set_address(addr);
    
    gpio_put(CE, false);
    gpio_put(OE, false);
    sleep_us(DELAY_READ);

    uint8_t val = gpio_get_all();   // read from lower 8 pins, discard rest

    gpio_put(CE, true);
    gpio_put(OE, true);

    return val;
}

/**
 * @brief Write a byte at address
 * 
 * @param addr full 19-bit address
 * @param val  8-bit value to write
 */
void write_byte(uint32_t addr, uint8_t val) {
    // set pins 0-7 to output
    gpio_set_dir_out_masked(0x000000FF);

    // set address on pins
    set_address(addr);

    gpio_put_masked(0xFF, val);
    gpio_put(CE, false);
    gpio_put(PGM, false);

    sleep_us(DELAY_WRITE);

    gpio_put(PGM, true);
    gpio_put(CE, true);

    // set pins 0-7 to input
    gpio_set_dir_in_masked(0x000000FF);
}

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
void write_byte_ignore_upper_fast(uint16_t addr, uint8_t val) {
    // set address on pins
    uint32_t addrl = addr & 0xFF;
    uint32_t addrh = (addr >> 8) & 0xFF;

    set_address_high(addrh);
    set_address_low(addrl);
    gpio_put_masked(0xFF, val);

    // toggle write
    gpio_put(PGM, false);
    sleep_us(DELAY_WRITE);
    gpio_put(PGM, true);
}