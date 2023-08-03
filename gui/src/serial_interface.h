/****************************************************************************
 *                                                                          *
 *   PICO-SST39SF0x0-FLASHER                                                *
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

#ifndef SERIAL_INTERFACE_H
#define SERIAL_INTERFACE_H

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDateTime>
#include <QDebug>
#include <QString>

#include <string>
#include <iostream>
#include <exception>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <memory>

/**
 * @brief Interface class handling serial communication
 */
class SerialInterface {

private:
    static const unsigned int SERIAL_TIMEOUT = 100;             // timeout for regular serial communication
    static const unsigned int SERIAL_TIMEOUT_SECTOR = 0;        // timeout when reading sector data (0x1000 bytes)
    static const unsigned int SERIAL_TIMEOUT_BLOCK = 3000;      // timeout when reading sector data (0x1000 bytes)
    std::string portname;                                       // communication port address
    std::unique_ptr<QSerialPort> port;                          // pointer to QSerialPort object

    // variables to store cartridge firmware version
    int firmware_major = 0;
    int firmware_minor = 0;
    int firmware_patch = 0;
    std::string firmware_version;
    std::string chipset;

    // list of features that are supported
    bool support_checksum = true;

public:
    /**
     * @brief SerialInterface
     * @param _portname address of the com port
     */
    SerialInterface(const std::string& _portname);

    /**
     * @brief get_port
     * @return string with port address
     */
    inline const std::string& get_port() const {
        return this->portname;
    }

    /**
     * @brief Create a new QSerialPort object and specify
     *        communication settings
     */
    void open_port();

    /**
     * @brief Close the communication port and destroy
     *        the QSerialPort object
     */
    void close_port();

    /********************************************************
     *  Cardreader interfacing routines
     ********************************************************/

    /**
     * @brief Get identifier string of the board
     * @return identifier string
     */
    std::string get_board_info();

    /**
     * @brief Read a block (0x100 bytes) from cartridge at address location
     * @param address location
     * @return data at address
     */
    QByteArray read_block(unsigned int sector_addr);

    /**
     * @brief Erase sector (0x1000 bytes) on SST39SF0x0 chip
     * @param upper bytes of sector address
     */
    void erase_sector(unsigned int sector_id);

    /**
     * @brief Burn block (256 bytes) to SST39SF0x0 chip
     * @param start address
     * @param data (256 bytes)
     */
    void burn_block(unsigned int sector_id, const QByteArray& data);

    /**
     * @brief get_chip_id check to verify this is a SST39SF0x0 chip
     * @return chip id
     */
    uint16_t get_chip_id();

private:

    /**
     * @brief Write single byte to address
     * @param address to write at
     * @param byte to write
     */
    void write_address(uint16_t address, uint8_t value);

    /**
     * @brief send a single command and capture the echo
     * @param command to send
     */
    void send_command(const std::string& command);

    /**
     * @brief send a single command and capture the response
     * @param command
     * @return
     */
    QByteArray send_command_capture_response(const std::string& command, int nrbytes);

    /**
     * @brief Capture any bytes left in read buffer and destroy them
     */
    void flush_buffer();

    /**
     * @brief Convenience function waiting for response
     */
    void wait_for_response(int nrbytes);

    /**
     * @brief Convenience function for comparing two version numbers
     */
    bool firmware_version_greater_than(int major, int minor, int patch);

    /**
     * @brief build_command
     * @param header
     * @param vallength: number of characters hex value has to hold
     * @param value: value to represent
     * @return command string
     */
    std::string build_command(const QString& header, int vallength, uint16_t value);
};

#endif // SerialInterface_H
