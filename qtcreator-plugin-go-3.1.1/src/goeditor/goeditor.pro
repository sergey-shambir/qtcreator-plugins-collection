include(../plugin.pri)

DEFINES += GOEDITOR_LIBRARY
CONFIG += c++11

# GoEditor files

SOURCES += goeditorplugin.cpp \
    goeditorfactory.cpp \
    goeditorwidget.cpp \
    goeditor.cpp \
    tools/gocodetask.cpp \
    tools/gohighlighter.cpp \
    tools/goindenter.cpp \
    tools/lexical/goscanner.cpp \
    tools/gocompletionassist.cpp \
    tools/highlighttask.cpp

HEADERS += goeditorplugin.h \
    goeditor_global.h \
    goeditorconstants.h \
    goeditorfactory.h \
    goeditorwidget.h \
    goeditor.h \
    tools/gocodetask.h \
    tools/gohighlighter.h \
    tools/goindenter.h \
    tools/lexical/goscanner.h \
    tools/gocompletionassist.h \
    tools/lexical/sourcecodestream.h \
    tools/highlighttask.h

RESOURCES += \
    goeditorplugin.qrc
