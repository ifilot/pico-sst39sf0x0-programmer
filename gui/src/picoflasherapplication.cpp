#include "picoflasherapplication.h"

/**
 * @brief Default constructor
 * @param argc number of command line argument
 * @param argv command line arguments
 */
PicoFlasherApplication::PicoFlasherApplication(int& argc, char** argv) :
QApplication(argc, argv) {}

/**
 * @brief notify
 * @param receiver
 * @param event
 * @return
 */
bool PicoFlasherApplication::notify(QObject* receiver, QEvent* event) {
    bool done = true;
    try {
        done = QApplication::notify(receiver, event);
    } catch (const std::exception& e) {
        this->throw_message_window(tr("Uncaught exception was thrown."),
                                   tr("An uncaught exception was thrown by the program. This is undiserable behavior and "
                                      "the program will be terminated. Please carefully read the error message "
                                      "and consult the manual or contact one of the developers.\n\nError:\n") + e.what());
        throw(e);
    }
    return done;
}


void PicoFlasherApplication::throw_message_window(const QString& title, const QString& message) {
    QMessageBox message_box;
    message_box.setText(message);
    message_box.setIcon(QMessageBox::Critical);
    message_box.setWindowTitle(title);
    message_box.setWindowIcon(QIcon(":/assets/icon/eeprom_icon.ico"));
    message_box.exec();
}
