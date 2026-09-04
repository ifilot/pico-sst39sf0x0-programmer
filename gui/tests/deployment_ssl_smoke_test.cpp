#include <QCoreApplication>
#include <QSslSocket>

#include <cstdio>

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);

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
