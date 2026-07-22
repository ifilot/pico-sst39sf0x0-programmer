#ifndef SERIAL_TRANSPORT_H
#define SERIAL_TRANSPORT_H

#include <QByteArray>

#include <memory>
#include <string>

/**
 * @brief Minimal transport abstraction used by SerialInterface.
 *
 * Production code uses a QSerialPort-backed implementation while tests can
 * provide an in-memory emulator that speaks the same command protocol.
 */
class SerialTransport {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~SerialTransport() = default;

    /**
     * @brief Open the transport.
     * @return true on success
     */
    virtual bool open() = 0;

    /**
     * @brief Close the transport.
     */
    virtual void close() = 0;

    /**
     * @brief Check whether the transport is open.
     * @return true when open
     */
    virtual bool isOpen() const = 0;

    /**
     * @brief Set the data terminal ready signal.
     * @param ready DTR state
     */
    virtual void setDataTerminalReady(bool ready) = 0;

    /**
     * @brief Return the last transport error string.
     * @return error string
     */
    virtual std::string errorString() const = 0;

    /**
     * @brief Write raw bytes to the transport.
     * @param data bytes to write
     * @param maxSize number of bytes
     * @return number of bytes written
     */
    virtual qint64 write(const char* data, qint64 maxSize) = 0;

    /**
     * @brief Write a QByteArray to the transport.
     * @param data bytes to write
     * @return number of bytes written
     */
    virtual qint64 write(const QByteArray& data) = 0;

    /**
     * @brief Wait for pending bytes to be written.
     * @param msecs timeout in milliseconds
     * @return true when new progress was observed
     */
    virtual bool waitForBytesWritten(int msecs) = 0;

    /**
     * @brief Wait for new bytes to arrive.
     * @param msecs timeout in milliseconds
     * @return true when new bytes are available
     */
    virtual bool waitForReadyRead(int msecs) = 0;

    /**
     * @brief Read all currently available bytes.
     * @return buffered bytes
     */
    virtual QByteArray readAll() = 0;

    /**
     * @brief Return the number of buffered bytes available for reading.
     * @return available byte count
     */
    virtual qint64 bytesAvailable() const = 0;
};

#endif // SERIAL_TRANSPORT_H
