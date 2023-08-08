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

#include "routines.h"

/*
 * Read device ID and print it to console
 */
void read_chip_id() {
    write_byte(0x5555, 0xAA);
    write_byte(0x2AAA, 0x55);
    write_byte(0x5555, 0x90);

    uint8_t id1 = read_byte(0x0000);
    uint8_t id2 = read_byte(0x0001);

    write_byte(0x5555, 0xAA);
    write_byte(0x2AAA, 0x55);
    write_byte(0x5555, 0xF0);

    putchar_raw(id1);
    putchar_raw(id2);
}

/*
 * Print data for first 32 bytes on chip
 */
void printbytes32() {
    for(uint32_t addr = 0; addr < 32; addr++) {
        if(addr == 16) {
            printf("\n");
        }
        
        uint8_t val = read_byte(addr);
        printf("%02X ", val);
    }
    printf("\n\n");
}

/*
 * @brief Read block of 256 bytes
 *
 * sector_addr: upper bytes
 */
void read_block(uint32_t block_id) {
    // set pins 0-7 to input
    gpio_set_dir_in_masked(0x000000FF);
    
    // bitshift to get full address
    uint32_t sector_addr = block_id << 8;
    set_address_high(sector_addr);
    
    gpio_put(CE, false);

    // set address to read from
    for(unsigned int i=0; i<256; i++) {
        set_address_low(sector_addr);
    
        gpio_put(OE, false);
        sleep_us(DELAY_READ);

        // read from lower 8 pins, discard rest
        uint8_t val = gpio_get_all();

        // send to UART
        putchar_raw(val);

        gpio_put(OE, true);

        sector_addr++;
    }

    gpio_put(CE, true);
}

uint16_t pollbyte(uint32_t addr) {
    // set pins 0-7 to input
    gpio_set_dir_in_masked(0x000000FF);

    uint16_t cnts = 0;     // keep track of number of polling attempts
    uint8_t val = 0;

    while((val >> 7) != 1 && cnts < 0xF000) {
        gpio_put(CE, false);
        gpio_put(OE, false);

        set_address(addr);
        val = gpio_get_all();   // read from lower 8 pins, discard rest

        cnts++;
        gpio_put(CE, true);
        gpio_put(OE, true);
    }
    
    return cnts;
}

/*
 * @brief Write block of 256 bytes
 *
 * sector_addr: upper bytes
 */
void write_block(uint32_t block_id) {
    
    // write new data
    uint32_t addr = block_id << 8;
    uint16_t bitsread = 0;
    uint8_t checksum = 0;
    
    while(bitsread < 256) {
        int uartc = getchar_timeout_us(DELAY_WRITE);
        if(uartc != PICO_ERROR_TIMEOUT ) {
            uint8_t c = uartc & 0xFF; 
            write_byte(0x5555, 0xAA);
            write_byte(0x2AAA, 0x55);
            write_byte(0x5555, 0xA0);
            write_byte(addr, c);
            checksum += c;

            addr++;
            bitsread++;
        }
    }
    
    // return checksum
    putchar_raw(checksum);
}

/*
 * @brief Erase sector
 */
void erase_sector(uint32_t block_id) {
    
    uint32_t erase_addr = block_id << 8;
    
    write_byte(0x5555, 0xAA);
    write_byte(0x2AAA, 0x55);
    write_byte(0x5555, 0x80);
    write_byte(0x5555, 0xAA);
    write_byte(0x2AAA, 0x55);
    write_byte(erase_addr, 0x30);

    // get number of iterations in pollbyte routine
    uint16_t cnts = pollbyte(erase_addr);
    
    // return result
    putchar_raw(cnts >> 8);
    putchar_raw(cnts & 0xFF);    
}

/**
 * Return board identification string
 **/
void write_board_id() {
    static const char* board_id = BOARD_ID;
    for(unsigned int i=0; i<16; i++) {
        putchar_raw(board_id[i]);
    }
    stdio_flush();
}