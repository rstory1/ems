#-------------------------------------------------
#
# Project created by QtCreator 2012-05-18T09:42:36
#
#-------------------------------------------------

QT       += core gui

TARGET = EngineMonitor
TEMPLATE = app

DESTDIR = ./bin

SOURCES += main.cpp\
        enginemonitor.cpp \
    bargraph.cpp \
    rpmindicator.cpp \
    cylinderheadtemperature.cpp \
    exhaustgastemperature.cpp

HEADERS  += enginemonitor.h \
    bargraph.h \
    rpmindicator.h \
    cylinderheadtemperature.h \
    exhaustgastemperature.h
