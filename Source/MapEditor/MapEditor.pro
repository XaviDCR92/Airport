#-------------------------------------------------
#
# Project created by QtCreator 2016-10-29T23:39:18
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MapEditor
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    mygraphicsscene.cpp

HEADERS  += mainwindow.h \
    mygraphicsscene.h

FORMS    += mainwindow.ui

DISTFILES += \
    ../../Sprites/Tileset1.bmp
