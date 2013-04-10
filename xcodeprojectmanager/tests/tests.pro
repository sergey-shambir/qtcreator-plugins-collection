#-------------------------------------------------
#
# Project created by QtCreator 2013-04-09T22:39:22
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

TARGET = tst_pbprojectmodel
CONFIG   += console
CONFIG   -= app_bundle

DEFINES += DEBUG_PBPROJ

TEMPLATE = app


SOURCES += tst_pbprojectmodel.cpp \
    ../pbxproj/pbclasses.cpp \
    ../pbxproj/pblexer.cpp \
    ../pbxproj/pbparser.cpp \
    ../pbxproj/pbprojectmodel.cpp \
    ../pbxproj/pbenums.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    ../pbxproj/pbclasses.h \
    ../pbxproj/pbenums.h \
    ../pbxproj/pblexer.h \
    ../pbxproj/pbparser.h \
    ../pbxproj/pbprojectmodel.h

OTHER_FILES += \
    ../pbxproj/pbparser_test_1.pbxproj
