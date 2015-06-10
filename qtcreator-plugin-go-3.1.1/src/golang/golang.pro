include(../plugin.pri)

DEFINES += GO_LIBRARY
CONFIG += c++11

# Go files

SOURCES += \
    golangplugin.cpp \
    goproject.cpp \
    goprojectfile.cpp \
    goprojectmanager.cpp \
    goprojectnode.cpp \
    filefilteritems.cpp \
    goprojectfileformat.cpp \
    goprojectitem.cpp \
    toolchain.cpp \
    toolchainmanager.cpp \
    toolchainoptionspage.cpp \
    toolchainconfigwidget.cpp \
    gotoolchain.cpp \
    gobuildconfiguration.cpp \
    gokitinformation.cpp \
    gorunconfigurationfactory.cpp \
    gorunconfiguration.cpp \
    goapplicationwizard.cpp \
    goparser.cpp

HEADERS += \
    golangplugin.h \
    golangconstants.h \
    golang_global.h \
    goproject.h \
    goprojectfile.h \
    goprojectmanager.h \
    goprojectnode.h \
    filefilteritems.h \
    goprojectfileformat.h \
    goprojectitem.h \
    toolchain.h \
    toolchainmanager.h \
    toolchainoptionspage.h \
    toolchainconfigwidget.h \
    gotoolchain.h \
    gobuildconfiguration.h \
    gokitinformation.h \
    gorunconfigurationfactory.h \
    gorunconfiguration.h \
    goapplicationwizard.h \
    goparser.h

RESOURCES += \
    golangplugin.qrc
