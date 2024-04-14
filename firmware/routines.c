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

    tud_cdc_write_char(id1);
    tud_cdc_write_char(id2);
    tud_cdc_write_flush();
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
    set_address(sector_addr);

    // data buffer
    uint8_t data[0x100];

    // enable chip
    gpio_put(CE, false);

    // set address to read from
    for(uint32_t i=0; i<256; i++) {
        set_address_low(i);
    
        gpio_put(OE, false);
        sleep_us(DELAY_READ);

        // read from lower 8 pins, discard rest
        data[i] = gpio_get_all();

        gpio_put(OE, true);
    }

    // disable chip
    gpio_put(CE, true);

    tud_cdc_write(data, 0x100);
    tud_cdc_write_flush();

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

    // data buffer
    uint8_t data[0x100];

    // set address to read from
    for(unsigned int j=0; j<16; j++) {
        set_address((j + pin12) << 8);
        for(unsigned int i=0; i<0x100; i++) {
            // set lower address
            set_address_low(i);
        
            // small delay to properly receive response
            sleep_us(DELAY_READ);

            // read from lower 8 pins, discard rest
            data[i] = gpio_get_all();
        }

        tud_cdc_write(data, 0x100);
        tud_cdc_write_flush();
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

    // data buffer
    uint8_t data[0x100];

    // set address to read from
    for(uint32_t j=0; j<0x40; j++) {
        set_address((j + 0x40 * bank_id) << 8);
        for(uint32_t i=0; i<0x100; i++) {
            set_address_low(i);
        
            sleep_us(DELAY_READ);

            // read from lower 8 pins, discard rest
            data[i] = gpio_get_all();
        }

        tud_cdc_write(data, 0x100);
        tud_cdc_write_flush();
    }

    gpio_put(CE, true);
    gpio_put(OE, true);
    gpio_put(LED_RD, false);
}

/*
 * @brief Read sector of 0x1000 bytes
 **/
void read_sector(uint8_t sector_id) {
    gpio_put(LED_RD, true);

    // set pins 0-7 to input
    gpio_set_dir_in_masked(0x000000FF);
    
    // chip enable
    gpio_put(CE, false);
    gpio_put(OE, false);

    // data buffer
    uint8_t data[0x100];

    // set address to read from
    for(uint8_t j=0; j<0x10; j++) {
        set_address_high((j + 0x10 * sector_id) << 8);
        for(uint32_t i=0; i<0x100; i++) {
            set_address_low(i);
        
            sleep_us(DELAY_READ);

            // read from lower 8 pins, discard rest
            data[i] = gpio_get_all();
        }

        tud_cdc_write(data, 0x100);
        tud_cdc_write_flush();
    }

    gpio_put(CE, true);
    gpio_put(OE, true);
    gpio_put(LED_RD, false);
}

/*
 * @brief Write block of 256 bytes
 *
 * sector_addr: upper bytes
 */
void write_block(uint32_t block_id) {
    // first collect all the data
    uint32_t bitsread = 0;
    uint8_t buffer[BLOCK_SIZE];
    while(bitsread < BLOCK_SIZE) {
        if(tud_cdc_available()) {
            bitsread += tud_cdc_read(&buffer[bitsread], BLOCK_SIZE - bitsread);
        }
        tud_task();
    }

    write_data(block_id << 8, buffer, BLOCK_SIZE);

    // return checksum
    uint8_t checksum = 0;
    for(uint32_t i=0; i<BLOCK_SIZE; i++) {
        checksum += buffer[i];
    }
    tud_cdc_write_char(checksum);
    tud_cdc_write_flush();
}

/*
 * @brief Write bank of 0x1000 bytes
 **/
void write_sector(uint8_t sector_id) {
    // first collect all the data
    uint32_t bitsread = 0;
    uint8_t buffer[SECTOR_SIZE];
    while(bitsread < SECTOR_SIZE) {
        if(tud_cdc_available()) {
            bitsread += tud_cdc_read(&buffer[bitsread], SECTOR_SIZE - bitsread);
        }
        tud_task();
    }

    // write data
    write_data(sector_id * SECTOR_SIZE, buffer, SECTOR_SIZE);

    // return crc16 checksum
    uint16_t crc16checksum = crc16_xmodem(buffer, SECTOR_SIZE);
    tud_cdc_write_char(crc16checksum & 0xFF);
    tud_cdc_write_char((crc16checksum >> 8) & 0xFF);
    tud_cdc_write_flush();
}

/*
 * @brief Write fixed set of data to address
 **/
void write_data(uint32_t addr, uint8_t *data, uint32_t nrbytes) {
    // turn on data LED
    gpio_put(LED_WR, true);
    
    // set upper bytes to low
    set_address_upper(0x00);

    uint8_t checkbyte = 0;
    for(uint32_t i=0; i<nrbytes; i++) {
        // chip is enabled and data pins are set to output
        gpio_put(CE, false);
        gpio_set_dir_out_masked(0x000000FF);

        write_byte_ignore_upper_fast(0x5555, 0xAA);
        write_byte_ignore_upper_fast(0x2AAA, 0x55);
        write_byte_ignore_upper_fast(0x5555, 0xA0);

        // write a byte to address; at the end of this function, CE is set
        // HIGH (disabled) and the pins are in input mode
        write_byte(addr, data[i]);

        checkbyte = ~data[i];
        do {
            gpio_put(CE, false);
            gpio_put(OE, false);
            sleep_us(DELAY_READ_VERIFY);

            checkbyte = gpio_get_all();   // read from lower 8 pins, discard rest

            gpio_put(CE, true);
            gpio_put(OE, true);
        } while(checkbyte != data[i]);

        addr++;
    }

    gpio_put(LED_WR, false);
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

/**
 * @brief      Erase the complete chip
 */
void erase_chip() {
    gpio_put(LED_WR, true);
    
    // chip-erase sequence
    write_byte(0x5555, 0xAA);
    write_byte(0x2AAA, 0x55);
    write_byte(0x5555, 0x80);
    write_byte(0x5555, 0xAA);
    write_byte(0x2AAA, 0x55);
    write_byte(0x5555, 0x10);

    // get number of iterations in pollbyte routine
    uint16_t cnts = pollbyte(0x0000);
    
    gpio_put(LED_WR, false);

    // return result
    tud_cdc_write_char(cnts >> 8);
    tud_cdc_write_char(cnts & 0xFF);
    tud_cdc_write_flush();
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
    tud_cdc_write_char(cnts >> 8);
    tud_cdc_write_char(cnts & 0xFF);    
    tud_cdc_write_flush();
}

/**
 * Return board identification string
 **/
void write_board_id() {
    static const char* board_id = BOARD_ID;
    tud_cdc_write(board_id, 16);
    tud_cdc_write_flush();

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