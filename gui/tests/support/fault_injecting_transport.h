#ifndef FAULT_INJECTING_TRANSPORT_H
#define FAULT_INJECTING_TRANSPORT_H

#include "serial_transport.h"

#include <memory>
#include <string>

class FaultInjectingTransport : public SerialTransport {
public:
    enum class FaultMode {
        None,
        CorruptEcho,
        CorruptBlockChecksum,
        CorruptSectorChecksum,
        SuppressResponse
    };

private:
    std::unique_ptr<SerialTransport> inner;
    std::string target_command;
    FaultMode fault_mode = FaultMode::None;
    int remaining_triggers = 0;
    std::string last_command;
    bool active_fault = false;
    bool pending_payload_fault = false;

    bool looks_like_command(const char* data, qint64 maxSize) const;
    void activate_fault_if_needed(const std::string& command);
    QByteArray mutate_read_data(const QByteArray& data);

public:
    FaultInjectingTransport(std::unique_ptr<SerialTransport> _inner,
                            std::string _target_command,
                            FaultMode _fault_mode,
                            int _trigger_count = 1);

    bool open() override;
    void close() override;
    bool isOpen() const override;

    void setDataTerminalReady(bool ready) override;
    std::string errorString() const override;

    qint64 write(const char* data, qint64 maxSize) override;
    qint64 write(const QByteArray& data) override;

    bool waitForBytesWritten(int msecs) override;
    bool waitForReadyRead(int msecs) override;

    QByteArray readAll() override;
    qint64 bytesAvailable() const override;
};

#endif // FAULT_INJECTING_TRANSPORT_H
