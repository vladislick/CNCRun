#-------------------------------------------------
#
# Project created by QtCreator 2018-12-22T20:29:19
# Author: Vdovin Vladislav @vladislick
#
#-------------------------------------------------

QT       += core gui serialport widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CNCRun
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        src/main.cpp \
        src/mainwindow.cpp \
        src/aboutapp.cpp \
        src/settings.cpp \
        src/gcode.cpp \
        src/config.cpp

HEADERS += \
        lib/mainwindow.h \
        lib/aboutapp.h \
        lib/settings.h \
        lib/gcode.h \
        lib/config.h

FORMS += \
        ui/mainwindow.ui \
        ui/aboutapp.ui \
        ui/settings.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    settings.conf
