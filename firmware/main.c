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

#include "constants.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pico/stdlib.h>
#include <tusb.h>

#include "routines.h"
#include "cmds.h"

// command storage
char instruction[9];    // stores single 8-byte instruction
uint8_t inptr = 0;      // instruction pointer
uint8_t buf[64];        // usb buffer

// forward declarations
void parse_instructions();

/**
 * Initialization routine that sets default pin configuration
 **/
void init() {
    // initialize standard I/O
    stdio_init_all();

    // initialize TinyUSB stack
    tusb_init();

    // initialize GPIO0-GPIO19 + 26,27
    gpio_init_mask(0x00CFFFFFF);

    // set GPIO8-GPIO23 + 26,27 to output and GPIO0-7 to input
    gpio_set_dir_out_masked(0x0CFFFF00);
    gpio_set_dir_in_masked(0x000000FF);

    // set output enable (~OE) line to high
    gpio_put(ROE, false);
    gpio_put(CE, true);
    gpio_put(OE, true);
    gpio_put(PGM, true);
    gpio_put(ALS, false);
    gpio_put(AHS, false);
    gpio_put(AUS, false);

    // set leds to output
    gpio_put(LED_RD, false);
    gpio_put(LED_WR, false);
}

/*
 * @brief Main routine
 */
int main() {
    init();

    while(true) {

        if (tud_cdc_connected()) {
            if (tud_cdc_available()) {
                char c = tud_cdc_read_char();
        
                // only capture alphanumerical data
                if((c >= 48 && c <= 57) || (c >= 65 && c <=90)) {
                    instruction[inptr] = c;
                    inptr++;
                }
                
                if(inptr == 8) {
                    parse_instructions();
                    inptr = 0;
                }
            }
        }

        tud_task();
    }
}

/*
 * @brief parse instructions received over serial
 */
void parse_instructions() {
    echo_command(instruction, 8);
    
    /*
    * Read identifier string from board
    */
    if(check_command(instruction, "READINFO", 0, 8)) {
        write_board_id();
        return;
    /*
    * Perform a simple test; reads the first 32 bytes of the ROM
    */
    } else if(check_command(instruction, "TESTTEST", 0, 8)) {
        printbytes32();
        return;
    /*
    * Read the device id from the SSTSF390x0 chip
    */
    } else if(check_command(instruction, "DEVIDSST", 0, 8)) {
        read_chip_id();
        return;
    /*
    * Completely erase the chip
    */
    } else if(check_command(instruction, "ERASEALL", 0, 8)) {
        erase_chip();
        return;
    /*
    * Read data from P2000 SLOT1 cartridge, requiring adapter board
    */        
    } else if(check_command(instruction, "RP2KCR", 0, 6)) {
        read_p2k_cartridge_block(get_uint8(instruction, 6));
        return;
    /*
    * Read a bank (16kb)
    */
    } else if(check_command(instruction, "RDBANK", 0, 6)) {
        read_bank(get_uint8(instruction, 6));
        return;
    /*
    * Read a block
    */
    } else if(check_command(instruction, "RDBK", 0, 4)) {
        read_block(get_uint16(instruction, 4));
        return;
    /*
    * Read a sector (4kb)
    */
    } else if(check_command(instruction, "RDSECT", 0, 6)) {
        read_sector(get_uint8(instruction, 6));
        return;
    /*
    * Write a sector (4kb)
    */
    } else if(check_command(instruction, "WRSECT", 0, 6)) {
        write_sector(get_uint8(instruction, 6));
        return;
    /*
    * Write a block (256 bytes)
    */
    } else if(check_command(instruction, "WRBK", 0, 4)) {
        write_block(get_uint16(instruction, 4));
        return;
    /*
    * Erase a sector (4kb)
    */
    } else if(check_command(instruction, "ESST", 0, 4)) {
        erase_sector(get_uint16(instruction, 4));
        return;
    }
}