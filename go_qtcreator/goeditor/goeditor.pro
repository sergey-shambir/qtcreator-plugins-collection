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
    tools/goautocompleter.cpp \
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
    tools/goautocompleter.h \
    tools/lexical/sourcecodestream.h \
    tools/highlighttask.h

# Qt Creator linking

## uncomment to build plugin into user config directory
## <localappdata>/plugins/<ideversion>
##    where <localappdata> is e.g.
##    "%LOCALAPPDATA%\QtProject\qtcreator" on Windows Vista and later
##    "$XDG_DATA_HOME/data/QtProject/qtcreator" or "~/.local/share/data/QtProject/qtcreator" on Linux
##    "~/Library/Application Support/QtProject/Qt Creator" on Mac
USE_USER_DESTDIR = yes

PROVIDER = QtProject

LIBS += -L"/usr/lib/x86_64-linux-gnu/qtcreator/" -L"/usr/lib/x86_64-linux-gnu/qtcreator/plugins/QtProject"

include(/usr/src/qtcreator/src/qtcreatorplugin.pri)

RESOURCES += \
    goeditorplugin.qrc

