include(../plugin.pri)

DEFINES += GOEDITOR_LIBRARY
CONFIG += c++11

# GoEditor files

SOURCES += goeditorplugin.cpp \
    goeditorfactory.cpp \
    goeditorwidget.cpp \
    goeditor.cpp \
    tools/gohighlighter.cpp \
    tools/goindenter.cpp \
    tools/lexical/goscanner.cpp \
    tools/gocompletionassist.cpp \
    tools/highlighttask.cpp \
    goeditordocument.cpp \
    tools/gosemanticinfo.cpp \
    tools/gohoverhandler.cpp \
    tools/gotoolprocess.cpp \
    tools/gocodeprocess.cpp \
    tools/gofmtprocess.cpp \
    tools/gosemkiprocess.cpp

HEADERS += goeditorplugin.h \
    goeditor_global.h \
    goeditorconstants.h \
    goeditorfactory.h \
    goeditorwidget.h \
    goeditor.h \
    tools/gohighlighter.h \
    tools/goindenter.h \
    tools/lexical/goscanner.h \
    tools/gocompletionassist.h \
    tools/lexical/sourcecodestream.h \
    tools/highlighttask.h \
    goeditordocument.h \
    tools/gosemanticinfo.h \
    tools/gohoverhandler.h \
    tools/gotoolprocess.h \
    tools/gocodeprocess.h \
    tools/gofmtprocess.h \
    tools/gosemkiprocess.h

RESOURCES += \
    goeditorplugin.qrc

