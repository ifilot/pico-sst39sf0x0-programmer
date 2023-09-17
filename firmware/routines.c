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
    gpio_put(LED_RD, true);

    write_byte(0x5555, 0xAA);
    write_byte(0x2AAA, 0x55);
    write_byte(0x5555, 0x90);

    uint8_t id1 = read_byte(0x0000);
    uint8_t id2 = read_byte(0x0001);

    write_byte(0x5555, 0xAA);
    write_byte(0x2AAA, 0x55);
    write_byte(0x5555, 0xF0);

    gpio_put(LED_RD, false);

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
    gpio_put(LED_RD, true);

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
    gpio_put(LED_RD, false);
}

/*
 * @brief Read block of 0x1000 bytes
 **/
void read_p2k_cartridge_block(uint8_t block_id) {
    gpio_put(LED_RD, true);

    // set pins 0-7 to input
    gpio_set_dir_in_masked(0x000000FF);
    
    // set pins based on block_id
    gpio_put(CE, block_id >= 2);  // CARSEL 1
    gpio_put(OE, block_id < 2);   // CARSEL 2
    unsigned int pin12 = (block_id & 1) ? 0 : (1 << 4);

    // set address to read from
    for(unsigned int j=0; j<16; j++) {
        set_address_high((j + pin12) << 8);
        for(unsigned int i=0; i<256; i++) {
            // set lower address
            set_address_low(i);
        
            // small delay to properly receive response
            sleep_us(DELAY_READ);

            // read from lower 8 pins, discard rest
            uint8_t val = gpio_get_all();

            // send to UART
            putchar_raw(val);
        }
    }

    gpio_put(CE, true);
    gpio_put(OE, true);
    gpio_put(LED_RD, false);
}

/*
 * @brief Read bank of 0x4000 bytes
 **/
void read_bank(uint8_t bank_id) {
    gpio_put(LED_RD, true);

    // set pins 0-7 to input
    gpio_set_dir_in_masked(0x000000FF);
    
    // chip enable
    gpio_put(CE, false);
    gpio_put(OE, false);

    // set address to read from
    for(uint32_t j=0; j<0x40; j++) {
        set_address_high((j + 0x40 * bank_id) << 8);
        for(uint32_t i=0; i<0x100; i++) {
            set_address_low(i);
        
            sleep_us(DELAY_READ);

            // read from lower 8 pins, discard rest
            uint8_t val = gpio_get_all();

            // send to UART
            putchar_raw(val);
        }
    }

    gpio_put(CE, true);
    gpio_put(OE, true);
    gpio_put(LED_RD, false);
}

/*
 * @brief Read sector of 0x1000 bytes
 **/
void read_sector(uint32_t sector_id) {
    gpio_put(LED_RD, true);

    // set pins 0-7 to input
    gpio_set_dir_in_masked(0x000000FF);
    
    // chip enable
    gpio_put(CE, false);
    gpio_put(OE, false);

    uint32_t base_addr = sector_id * 0x1000;

    // set address to read from
    for(uint32_t j=0; j<0x10; j++) {
        set_address_high(base_addr | (j << 8));
        for(uint32_t i=0; i<0x100; i++) {
            set_address_low(i);
        
            sleep_us(DELAY_READ);

            // read from lower 8 pins, discard rest
            uint8_t val = gpio_get_all();

            // send to UART
            putchar_raw(val);
        }
    }

    gpio_put(CE, true);
    gpio_put(OE, true);
    gpio_put(LED_RD, false);
}

/*
 * @brief Write bank of 0x1000 bytes
 **/
void write_sector(uint32_t sector_id) {
    // first collect all the data
    uint16_t bitsread = 0;
    uint8_t buffer[0x1000];
    while(bitsread < 0x1000) {
        int uartc = getchar_timeout_us(DELAY_WRITE);
        if(uartc != PICO_ERROR_TIMEOUT ) {
            uint8_t c = uartc & 0xFF;
            buffer[bitsread] = c;
            bitsread++;
        }
    }

    // then write the data
    gpio_put(LED_WR, true);

    uint32_t addr = sector_id * 0x1000;
    uint8_t checksum = 0;
    for(uint16_t i=0; i<0x1000; i++) {
        uint8_t c = buffer[i];
        write_byte(0x5555, 0xAA);
        write_byte(0x2AAA, 0x55);
        write_byte(0x5555, 0xA0);
        write_byte(addr, c);
        checksum += c;
        addr++;

        sleep_us(DELAY_ADDR);
    }
    
    gpio_put(LED_WR, false);

    // return crc16 checksum
    uint16_t crc16checksum = crc16_xmodem(buffer, 0x1000);
    putchar_raw(crc16checksum & 0xFF);
    putchar_raw((crc16checksum >> 8) & 0xFF);
}

/*
 * @brief Wait until MSB bit is one
 **/
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
    gpio_put(LED_WR, true);

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
    
    gpio_put(LED_WR, false);

    // return checksum
    putchar_raw(checksum);
}

/*
 * @brief Erase sector (4kb)
 */
void erase_sector(uint32_t block_id) {
    gpio_put(LED_WR, true);
    uint32_t erase_addr = block_id << 8;
    
    write_byte(0x5555, 0xAA);
    write_byte(0x2AAA, 0x55);
    write_byte(0x5555, 0x80);
    write_byte(0x5555, 0xAA);
    write_byte(0x2AAA, 0x55);
    write_byte(erase_addr, 0x30);

    // get number of iterations in pollbyte routine
    uint16_t cnts = pollbyte(erase_addr);
    
    gpio_put(LED_WR, false);

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

/**
 * Generates CRC16 XMODEM checksum for data
*/
uint16_t crc16_xmodem(uint8_t *data, uint16_t length) {
    uint32_t crc = 0;
    static const uint16_t poly = 0x1021;

    for(uint16_t i=0; i<length; i++) {
      crc = crc ^ (data[i] << 8);
      for (uint8_t j=0; j<8; j++) {
        crc = crc << 1;
        if (crc & 0x10000) {
            crc = (crc ^ poly) & 0xFFFF;
        }
      }
    }

    return (uint16_t)crc;
}