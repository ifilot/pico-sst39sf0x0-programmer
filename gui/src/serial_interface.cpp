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

#include "serial_interface.h"

/**
 * @brief SerialInterface
 * @param _portname address of the com port
 */
SerialInterface::SerialInterface(const std::string& _portname) {
    this->portname = _portname;
}

/**
 * @brief Create a new QSerialPort object and specify
 *        communication settings
 */
void SerialInterface::open_port() {
    if(this->portname.size() == 0) {
        throw std::runtime_error("No port has been set");
    }

    this->port = std::make_unique<QSerialPort>(this->portname.c_str());
    this->port->setBaudRate(QSerialPort::Baud19200);
    this->port->setDataBits(QSerialPort::Data8);
    this->port->setStopBits(QSerialPort::OneStop);
    this->port->setParity(QSerialPort::NoParity);
    this->port->setFlowControl(QSerialPort::NoFlowControl);

    this->port->open(QIODevice::ReadWrite);
    this->port->setDataTerminalReady(true);

    qDebug() << QObject::tr("Opening COM port:") + QObject::tr(this->portname.c_str());
}

/**
 * @brief Close the communication port and destroy
 *        the QSerialPort object
 */
void SerialInterface::close_port() {
    this->port->close();
    this->port.reset();

    qDebug() << QObject::tr("Closing COM port:") + QObject::tr(this->portname.c_str());
}

/********************************************************
 *  Cardreader interfacing routines
 ********************************************************/

/**
 * @brief Get identifier string of the board
 * @return identifier string
 */
std::string SerialInterface::get_board_info() {
    try {
        this->flush_buffer();
        const std::string command = "READINFO";
        QByteArray response_data = this->send_command_capture_response(command, 16);

        // store firmware data
        QString firmware_string = QString(response_data);
        if(firmware_string.mid(0,4) != "PICO") {
            throw std::runtime_error("Unidentified chipset");
        }

        return response_data.toStdString();
    }  catch (std::exception& e) {
        std::cerr << "Caught error: " << e.what() << std::endl;
        throw e;
    }
}

/**
 * @brief Read a block (0x100 bytes) from cartridge at address location
 * @param address location
 * @return data at address
 */
QByteArray SerialInterface::read_block(unsigned int block_addr) {
    try {
        std::string command = QString("RDBK%1").arg(block_addr,4,16, QLatin1Char('0')).toUpper().toStdString();
        QByteArray response_data = this->send_command_capture_response(command, 256);

        return response_data;
    }  catch (std::exception& e) {
        std::cerr << "Caught error: " << e.what() << std::endl;
        throw e;
    }
}

/**
 * @brief Read a segment (0x1000 bytes) from a cartridge at segment location
 * @param address location
 * @return data at address
 */
QByteArray SerialInterface::read_segment_cartridge(unsigned int segment_addr) {
    try {
        std::string command = QString("RP2KCR%1").arg(segment_addr,2,16, QLatin1Char('0')).toUpper().toStdString();
        QByteArray response_data = this->send_command_capture_response(command, 0x1000);

        return response_data;
    }  catch (std::exception& e) {
        std::cerr << "Caught error: " << e.what() << std::endl;
        throw e;
    }
}

/**
 * @brief Read a bank (0x4000 bytes)
 * @param bank_id
 * @return data at address
 */
QByteArray SerialInterface::read_bank(unsigned int bank_id) {
    try {
        std::string command = QString("RDBANK%1").arg(bank_id,2,16, QLatin1Char('0')).toUpper().toStdString();
        QByteArray response_data = this->send_command_capture_response(command, 0x4000);

        return response_data;
    }  catch (std::exception& e) {
        std::cerr << "Caught error: " << e.what() << std::endl;
        throw e;
    }
}

/**
 * @brief Erase sector (4096 bytes) on SST39SF0x0 chip
 * @param start address
 */
void SerialInterface::erase_sector(unsigned int sector_id) {
    try {
        std::string command = QString("ESST%1").arg(sector_id,4,16,QLatin1Char('0')).toUpper().toStdString();
        auto response = this->send_command_capture_response(command, 2);
        uint16_t nrcycles = 0;
        memcpy((void*)&nrcycles, (void*)&response.data()[0], 2);
        qDebug() << "Succesfully erased sector #" << sector_id << " in " << nrcycles << " cyles.";
    }  catch (std::exception& e) {
        std::cerr << "Caught error: " << e.what() << std::endl;
        throw e;
    }
}

/**
 * @brief Burn block (256 bytes) to SST39SF0x0 chip
 * @param start address
 * @param data (256 bytes)
 */
void SerialInterface::burn_block(unsigned int sector_addr, const QByteArray& data) {
    try {
        qDebug() << "Burning block.";

        // calculate checksum
        uint8_t checksum = 0;
        for(int i=0; i<data.size(); i++) {
            checksum += data[i];
        }

        // display which checksum to expect
        qDebug() << QString("Expecting checksum: 0x%1").arg(checksum, 2, 16, QLatin1Char('0')).toUpper().toStdString().c_str();

        // construct command
        std::string command = QString("WRBK%1").arg(sector_addr,4,16,QLatin1Char('0')).toUpper().toStdString();

        // send command to serial interface
        this->send_command(command);

        this->port->write(data, 256);
        while(this->port->waitForBytesWritten(SERIAL_TIMEOUT_BLOCK)){}

        this->wait_for_response(1);
        auto response = this->port->readAll();

        if((uint8_t)response.data()[0] != checksum) {
            qCritical() << "Invalid checksum received: " << checksum << " versus " << response[0];
            throw std::runtime_error("Invalid checksum received");
        } else {
            qDebug() << QString("Valid checksum received: 0x%1").arg(checksum, 2, 16, QLatin1Char('0')).toUpper().toStdString().c_str();
        }

        // discard any contents still left in read buffer
        this->flush_buffer();
    }  catch (std::exception& e) {
        std::cerr << "Caught error: " << e.what() << std::endl;
        throw e;
    }
}

/**
 * @brief get_chip_id check to verify this is a SST39SF0x0 chip
 * @return chip id
 */
uint16_t SerialInterface::get_chip_id() {
    try {
        std::string command = "DEVIDSST";
        auto response = this->send_command_capture_response(command, 2);
        uint16_t chip_id = (uint16_t)(response[0]+1) * 256 + (uint16_t)response[1];
        return chip_id;
    }  catch (std::exception& e) {
        std::cerr << "Caught error: " << e.what() << std::endl;
        throw e;
    }

}

/********************************************************
 *  PRIVATE ROUTINES
 ********************************************************/

/**
 * @brief Write single byte to address
 * @param address to write at
 * @param byte to write
 */
void SerialInterface::write_address(uint16_t address, uint8_t value) {
    try {
        std::string command = QString("WR%1%2").arg(address,4,16, QLatin1Char('0')).arg(value,2,16,QLatin1Char('0')).toUpper().toStdString();
        this->send_command(command);

    }  catch (std::exception& e) {
        std::cerr << "Caught error: " << e.what() << std::endl;
        throw e;
    }
}

/**
 * @brief send a single command and capture the echo
 * @param command to send
 */
void SerialInterface::send_command(const std::string& command) {
    // send the command
    qDebug() << "Send command: " << command.c_str();
    this->port->write(command.c_str(), 8);
    while(this->port->waitForBytesWritten(SERIAL_TIMEOUT)){}

    // capture command response
    this->wait_for_response(8);
    auto response = this->port->readAll();

    // check that response is identifical to command,
    // else throw an error
    if(strcmp(response.toStdString().c_str(), command.c_str()) != 0) {
        throw std::runtime_error("Invalid response received (" + response.toStdString() + ") from command " + command);
    } else {
        qDebug() << "Response succesfully received: " << response;
    }
}

/**
 * @brief send a single command and capture the echo
 * @param command to send
 */
QByteArray SerialInterface::send_command_capture_response(const std::string& command, int nrbytes) {
    // send the command
    qDebug() << "Send command: " << command.c_str();
    this->port->write(command.c_str(), 8);
    while(this->port->waitForBytesWritten(SERIAL_TIMEOUT)){}

    // capture command response
    this->wait_for_response(8 + nrbytes);

    // read bytes from port
    auto response = this->port->readAll();

    // separate command and response
    auto cmdres = response.mid(0,8);
    response = response.mid(8,nrbytes);

    // verify response
    if(strcmp(cmdres.toStdString().c_str(), command.c_str()) != 0) {
        throw std::runtime_error("Invalid response received (" + response.toStdString() + ") from command " + command);
    } else {
        qDebug() << "Response succesfully received: " << cmdres;
    }

    qDebug() << "Done, returning " << response.size() << " bytes.";
    return response;
}

/**
 * @brief Capture any bytes left in read buffer and destroy them
 */
void SerialInterface::flush_buffer() {
    QByteArray response;

    if(this->port->waitForReadyRead(100)) {
        response += this->port->readAll(); //discard bytes
    }

    if(response.size() > 0) {
        qDebug() << "Flushing buffer, discarding the following bytes: " << response;
    }
}

/**
 * @brief Convenience function waiting for response
 */
void SerialInterface::wait_for_response(int nrbytes) {
    size_t ctr = 0;
    int bytes_available = 0;
    while(this->port->waitForReadyRead(10) || this->port->bytesAvailable() < nrbytes){
        // check if number of bytes available is increasing, if not, increment counter
        if(this->port->bytesAvailable() == bytes_available) {
            ctr++;
        }
        bytes_available = this->port->bytesAvailable();

        // if counter reaches a maximum number of tries, terminate the procedure
        if(ctr > 100) {
            qDebug() << "Failed to capture response, outputting buffer:";
            qDebug() << this->port->readAll();
            throw std::runtime_error("Too many tries waiting for response to command, terminating.");
        }
    }
}

bool SerialInterface::firmware_version_greater_than(int major, int minor, int patch) {
    if(this->firmware_major > major) {
        return true;
    } else if(this->firmware_major < major) {
        return false;
    }

    // major is equal, so check minor
    if(this->firmware_minor > minor) {
        return true;
    } else if(this->firmware_minor < minor) {
        return false;
    }

    // major and minor are both equal, so check patch
    if(this->firmware_patch > patch) {
        return true;
    }

    // either patch is also equal or smaller, so return false
    return false;
}

/**
 * @brief build_command
 * @param header
 * @param vallength: number of characters hex value has to hold
 * @param value: value to represent
 * @return command string
 */
std::string SerialInterface::build_command(const QString& header, int vallength, uint16_t value) {
    return QString(header + QString("%1").arg(value,vallength,16, QLatin1Char('0'))).toUpper().toStdString();
}
