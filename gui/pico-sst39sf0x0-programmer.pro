QT       += core gui serialport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/cartridgereadthread.cpp \
    src/dialogslotselection.cpp \
    src/filedownloader.cpp \
    src/flashthread.cpp \
    src/hexviewwidget.cpp \
    src/ioworker.cpp \
    src/logwindow.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/picoflasherapplication.cpp \
    src/readthread.cpp \
    src/serial_interface.cpp \
    src/settingswidget.cpp

HEADERS += \
    src/colors.h \
    src/config.h \
    src/cartridgereadthread.h \
    src/dialogslotselection.h \
    src/filedownloader.h \
    src/flashthread.h \
    src/hexviewwidget.h \
    src/ioworker.h \
    src/logwindow.h \
    src/mainwindow.h \
    src/picoflasherapplication.h \
    src/readthread.h \
    src/romsizes.h \
    src/serial_interface.h \
    src/settingswidget.h

# store Git id
GIT_HASH="\\\"$$system(git -C \""$$_PRO_FILE_PWD_"\" rev-parse --short HEAD)\\\""
DEFINES += GIT_HASH=$$GIT_HASH

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

DISTFILES += \
    assets/fonts/Consolas.ttf
