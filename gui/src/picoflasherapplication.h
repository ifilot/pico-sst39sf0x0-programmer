#ifndef PICOFLASHERAPPLICATION_H
#define PICOFLASHERAPPLICATION_H

#include <QApplication>
#include <QMessageBox>
#include <QIcon>

#include "config.h"

class PicoFlasherApplication : public QApplication
{
public:
    /**
     * @brief Default constructor
     * @param argc number of command line argument
     * @param argv command line arguments
     */
    PicoFlasherApplication(int& argc, char** argv);

    /**
     * @brief notify
     * @param receiver
     * @param event
     * @return
     */
    bool notify(QObject* receiver, QEvent* event);

private:
    void throw_message_window(const QString& title, const QString& message);
};

#endif // PICOFLASHERAPPLICATION_H
