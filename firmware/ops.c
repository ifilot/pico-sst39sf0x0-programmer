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

    gpio_put_masked(0xFF00, addrl << 8);
    gpio_put(ALS, true);
    sleep_us(DELAY_ADDR);
    gpio_put(ALS, false);

    gpio_put_masked(0xFF00, addrh << 8);
    gpio_put(AHS, true);
    sleep_us(DELAY_ADDR);
    gpio_put(AHS, false);

    gpio_put_masked(0x0700, addru << 8);
    gpio_put(AUS, true);
    sleep_us(DELAY_ADDR);
    gpio_put(AUS, false);

    gpio_put(ROE, false); // enable address output
}

/**
 * Set upper bits in address register
 **/
void set_address_high(uint32_t addr) {
    set_address(addr & 0xFFFFFF00);
}

/**
 * Set lower 8 bits in address register
 **/
void set_address_low(uint32_t addr) {
    uint32_t addrl = addr & 0xFF;

    gpio_put_masked(0xFF00, addrl << 8);
    gpio_put(ALS, true);
    sleep_us(DELAY_ADDR);
    gpio_put(ALS, false);

    gpio_put(ROE, false); // enable address output
}

/**
 * Read single byte from address
 **/
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
 * Write byte at address
 **/
void write_byte(uint32_t addr, uint8_t val) {
    // set pins 0-7 to output
    gpio_set_dir_out_masked(0x000000FF);

    set_address(addr);

    gpio_put(CE, false);
    gpio_put(PGM, false);

    // sleep_us(DELAY_TIME);
    gpio_put_masked(0xFF, val);
    sleep_us(DELAY_WRITE);

    gpio_put(PGM, true);
    gpio_put(CE, true);

    // set pins 0-7 to input
    gpio_set_dir_in_masked(0x000000FF);
}