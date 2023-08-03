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

#include "cmds.h"

/*
 * @brief convert 2 HEX bytes of instruction to 8 bit unsigned integer
 * @param command word
 * @param offset in command
 */
uint8_t get_uint8(const char* instruction, uint8_t offset) {
    char buffer[3];
    buffer[0] = instruction[offset];
    buffer[1] = instruction[offset+1];
    buffer[2] = '\0';
    return strtoul(buffer, NULL, 16);
}

/*
 * @brief convert 4 HEX bytes of instruction to 16 bit unsigned integer
 * @param command word
 * @param offset in command
 */
uint16_t get_uint16(const char* instruction, uint8_t offset) {
    char buffer[5];
    memcpy(buffer, &instruction[offset], 4);
    buffer[4] = '\0';
    return strtoul(buffer, NULL, 16);
}

/*
 * Return command back over serial
 */
void echo_command(const char* inst, uint8_t size) {
    for(unsigned int i=0; i<size; i++) {
        putchar_raw(inst[i]);
    }
    stdio_flush();
}

/*
 * @brief Check if part of two strings are equal
 * @param original string
 * @param reference string
 * @param offset in original string
 * @param length to read
 */
bool check_command(const char* cmd, const char* ref, uint8_t offset, uint8_t length) {
    for(uint8_t i=0; i<length; i++) {
        if(cmd[i+offset] != ref[i]) {
            return false;
        }
    }
    return true;
}