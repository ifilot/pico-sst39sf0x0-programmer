#include <QApplication>
#include <QSerialPort>
#include <QSslSocket>
#include <QWidget>

#include <cstdio>

#include "applicationstyle.h"

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    const QString styleName = configureApplicationStyle(application);
    QWidget widget;
    QSerialPort serialPort;
    widget.ensurePolished();

#ifdef Q_OS_WIN
    if (styleName.compare(QStringLiteral("windowsvista"), Qt::CaseInsensitive) != 0) {
        std::fprintf(stderr,
                     "Windows Vista style deployment check failed; active style is %s.\n",
                     styleName.toLocal8Bit().constData());
        return 1;
    }
#endif

    if (!serialPort.isSequential()) {
        std::fprintf(stderr, "Serial-port deployment check failed.\n");
        return 1;
    }

    if (!QSslSocket::supportsSsl()) {
        std::fprintf(stderr,
                     "TLS deployment check failed: Qt was built with %s, but no compatible "
                     "OpenSSL runtime could be loaded.\n",
                     QSslSocket::sslLibraryBuildVersionString().toLocal8Bit().constData());
        return 1;
    }

    std::printf("TLS deployment check passed: %s\n",
                QSslSocket::sslLibraryVersionString().toLocal8Bit().constData());
    return 0;
}
