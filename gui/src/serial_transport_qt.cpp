#include "serial_transport_qt.h"

/**
 * @brief Construct a new QSerialPort-backed transport.
 * @param portname serial port name
 */
QtSerialTransport::QtSerialTransport(const std::string& portname) :
    port(std::make_unique<QSerialPort>(portname.c_str())) {
    this->port->setBaudRate(QSerialPort::Baud19200);
    this->port->setDataBits(QSerialPort::Data8);
    this->port->setStopBits(QSerialPort::OneStop);
    this->port->setParity(QSerialPort::NoParity);
    this->port->setFlowControl(QSerialPort::NoFlowControl);
}

/**
 * @brief Open the serial port.
 * @return true on success
 */
bool QtSerialTransport::open() {
    return this->port->open(QIODevice::ReadWrite);
}

/**
 * @brief Close the serial port.
 */
void QtSerialTransport::close() {
    this->port->close();
}

/**
 * @brief Check whether the serial port is open.
 * @return true when open
 */
bool QtSerialTransport::isOpen() const {
    return this->port->isOpen();
}

/**
 * @brief Set the data terminal ready line.
 * @param ready DTR state
 */
void QtSerialTransport::setDataTerminalReady(bool ready) {
    this->port->setDataTerminalReady(ready);
}

/**
 * @brief Return the last serial port error string.
 * @return error string
 */
std::string QtSerialTransport::errorString() const {
    return this->port->errorString().toStdString();
}

/**
 * @brief Write raw bytes to the serial port.
 * @param data bytes to write
 * @param maxSize number of bytes
 * @return number of bytes written
 */
qint64 QtSerialTransport::write(const char* data, qint64 maxSize) {
    return this->port->write(data, maxSize);
}

/**
 * @brief Write a QByteArray to the serial port.
 * @param data bytes to write
 * @return number of bytes written
 */
qint64 QtSerialTransport::write(const QByteArray& data) {
    return this->port->write(data);
}

/**
 * @brief Wait for bytes to be written.
 * @param msecs timeout in milliseconds
 * @return true when progress was observed
 */
bool QtSerialTransport::waitForBytesWritten(int msecs) {
    return this->port->waitForBytesWritten(msecs);
}

/**
 * @brief Wait for new bytes to arrive.
 * @param msecs timeout in milliseconds
 * @return true when new bytes are available
 */
bool QtSerialTransport::waitForReadyRead(int msecs) {
    return this->port->waitForReadyRead(msecs);
}

/**
 * @brief Read all available bytes from the serial port.
 * @return buffered bytes
 */
QByteArray QtSerialTransport::readAll() {
    return this->port->readAll();
}

/**
 * @brief Return the number of available bytes.
 * @return available byte count
 */
qint64 QtSerialTransport::bytesAvailable() const {
    return this->port->bytesAvailable();
}
