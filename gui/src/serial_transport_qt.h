#ifndef SERIAL_TRANSPORT_QT_H
#define SERIAL_TRANSPORT_QT_H

#include "serial_transport.h"

#include <QSerialPort>

#include <memory>
#include <string>

/**
 * @brief QSerialPort-backed implementation of SerialTransport.
 */
class QtSerialTransport : public SerialTransport {
private:
    std::unique_ptr<QSerialPort> port;

public:
    /**
     * @brief Construct a new QSerialPort-backed transport.
     * @param portname serial port name
     */
    explicit QtSerialTransport(const std::string& portname);

    /**
     * @brief Open the serial port.
     * @return true on success
     */
    bool open() override;

    /**
     * @brief Close the serial port.
     */
    void close() override;

    /**
     * @brief Check whether the port is open.
     * @return true when open
     */
    bool isOpen() const override;

    /**
     * @brief Set the data terminal ready line.
     * @param ready DTR state
     */
    void setDataTerminalReady(bool ready) override;

    /**
     * @brief Get the current error string from QSerialPort.
     * @return error string
     */
    std::string errorString() const override;

    /**
     * @brief Write raw bytes to the serial port.
     * @param data bytes to write
     * @param maxSize number of bytes
     * @return number of bytes written
     */
    qint64 write(const char* data, qint64 maxSize) override;

    /**
     * @brief Write a QByteArray to the serial port.
     * @param data bytes to write
     * @return number of bytes written
     */
    qint64 write(const QByteArray& data) override;

    /**
     * @brief Wait for bytes to be written to the serial port.
     * @param msecs timeout in milliseconds
     * @return true when progress was observed
     */
    bool waitForBytesWritten(int msecs) override;

    /**
     * @brief Wait for data to become available on the serial port.
     * @param msecs timeout in milliseconds
     * @return true when new bytes are available
     */
    bool waitForReadyRead(int msecs) override;

    /**
     * @brief Read all currently available serial data.
     * @return buffered bytes
     */
    QByteArray readAll() override;

    /**
     * @brief Return the number of bytes available for reading.
     * @return available byte count
     */
    qint64 bytesAvailable() const override;
};

#endif // SERIAL_TRANSPORT_QT_H
