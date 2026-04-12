#include "fault_injecting_transport.h"

#include <cctype>

FaultInjectingTransport::FaultInjectingTransport(std::unique_ptr<SerialTransport> _inner,
                                                 std::string _target_command,
                                                 FaultMode _fault_mode,
                                                 int _trigger_count) :
    inner(std::move(_inner)),
    target_command(std::move(_target_command)),
    fault_mode(_fault_mode),
    remaining_triggers(_trigger_count) {}

bool FaultInjectingTransport::open() {
    return this->inner->open();
}

void FaultInjectingTransport::close() {
    this->active_fault = false;
    this->pending_payload_fault = false;
    this->last_command.clear();
    this->inner->close();
}

bool FaultInjectingTransport::isOpen() const {
    return this->inner->isOpen();
}

void FaultInjectingTransport::setDataTerminalReady(bool ready) {
    this->inner->setDataTerminalReady(ready);
}

std::string FaultInjectingTransport::errorString() const {
    return this->inner->errorString();
}

qint64 FaultInjectingTransport::write(const char* data, qint64 maxSize) {
    if(this->looks_like_command(data, maxSize)) {
        this->last_command.assign(data, static_cast<size_t>(maxSize));
        this->activate_fault_if_needed(this->last_command);
    } else if(this->pending_payload_fault && maxSize > 0) {
        this->active_fault = true;
        this->pending_payload_fault = false;
    }

    return this->inner->write(data, maxSize);
}

qint64 FaultInjectingTransport::write(const QByteArray& data) {
    return this->write(data.constData(), data.size());
}

bool FaultInjectingTransport::waitForBytesWritten(int msecs) {
    return this->inner->waitForBytesWritten(msecs);
}

bool FaultInjectingTransport::waitForReadyRead(int msecs) {
    if(this->active_fault && this->fault_mode == FaultMode::SuppressResponse) {
        return false;
    }

    return this->inner->waitForReadyRead(msecs);
}

QByteArray FaultInjectingTransport::readAll() {
    if(this->active_fault && this->fault_mode == FaultMode::SuppressResponse) {
        return QByteArray();
    }

    return this->mutate_read_data(this->inner->readAll());
}

qint64 FaultInjectingTransport::bytesAvailable() const {
    if(this->active_fault && this->fault_mode == FaultMode::SuppressResponse) {
        return 0;
    }

    return this->inner->bytesAvailable();
}

bool FaultInjectingTransport::looks_like_command(const char* data, qint64 maxSize) const {
    if(maxSize != 8) {
        return false;
    }

    for(qint64 i=0; i<maxSize; i++) {
        const unsigned char ch = static_cast<unsigned char>(data[i]);
        if(!std::isalnum(ch)) {
            return false;
        }
    }

    return true;
}

void FaultInjectingTransport::activate_fault_if_needed(const std::string& command) {
    if(this->remaining_triggers <= 0) {
        return;
    }

    if(command == this->target_command) {
        switch(this->fault_mode) {
            case FaultMode::CorruptBlockChecksum:
            case FaultMode::CorruptSectorChecksum:
                this->pending_payload_fault = true;
            break;
            case FaultMode::None:
            case FaultMode::CorruptEcho:
            case FaultMode::SuppressResponse:
                this->active_fault = true;
            break;
        }
        this->remaining_triggers--;
    }
}

QByteArray FaultInjectingTransport::mutate_read_data(const QByteArray& data) {
    if(!this->active_fault || data.isEmpty()) {
        return data;
    }

    QByteArray mutated = data;

    switch(this->fault_mode) {
        case FaultMode::CorruptEcho:
            mutated[0] = mutated[0] == 'X' ? 'Y' : 'X';
        break;
        case FaultMode::CorruptBlockChecksum:
            mutated[0] = static_cast<char>(static_cast<unsigned char>(mutated[0]) ^ 0xFF);
        break;
        case FaultMode::CorruptSectorChecksum:
            mutated[0] = static_cast<char>(static_cast<unsigned char>(mutated[0]) ^ 0xFF);
        break;
        case FaultMode::None:
        case FaultMode::SuppressResponse:
        break;
    }

    this->active_fault = false;
    return mutated;
}
