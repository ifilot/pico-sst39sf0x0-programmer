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

#ifndef _CMDS_H
#define _CMDS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pico/stdlib.h>
#include <pico/stdio.h>
#include <tusb.h>

uint8_t get_uint8(const char* instruction, uint8_t offset);
uint16_t get_uint16(const char* instruction, uint8_t offset);
void echo_command(const char* inst, uint8_t size);
bool check_command(const char* cmd, const char* ref, uint8_t offset, uint8_t length);

#endif // _CMDS_H