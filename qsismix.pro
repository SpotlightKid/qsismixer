#-------------------------------------------------
#
# Project created by QtCreator 2015-09-14T19:31:09
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qsismix
TEMPLATE = app


SOURCES += src/main.cpp\
        src/mainwindow.cpp \
    src/mixsis.cpp \
    src/mixsisctrl.cpp \
    src/changewatcher.cpp

HEADERS  += include/mainwindow.h \
    include/mixsis.h \
    include/mixsisctrl.h \
    include/changewatcher.h

FORMS    += mainwindow.ui

CONFIG += c++11

QMAKE_CXXFLAGS += -std=gnu++11

LIBS += -lasound

INCLUDEPATH += include

bin.path = /usr/local/bin
bin.files = $${OUT_PWD}/qsismix

INSTALLS += bin
