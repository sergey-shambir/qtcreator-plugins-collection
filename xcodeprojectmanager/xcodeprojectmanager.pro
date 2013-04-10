TARGET = XCodeProjectManager
TEMPLATE = lib

DEFINES += XCODEPROJECTMANAGER_LIBRARY

# XCodeProjectManager files

SOURCES += xcodeprojectmanagerplugin.cpp \
    pbxproj/pbclasses.cpp \
    pbxproj/pblexer.cpp \
    pbxproj/pbparser.cpp \
    pbxproj/pbprojectmodel.cpp \
    pbxproj/pbenums.cpp \
    xcodeprojectbuildoptionspage.cpp \
    xcodemanager.cpp \
    xcodeproject.cpp \
    xcodeprojectnode.cpp \
    xcodeprojectfile.cpp

HEADERS += xcodeprojectmanagerplugin.h \
        xcodeprojectmanager_global.h \
        xcodeprojectmanagerconstants.h \
    pbxproj/pbclasses.h \
    pbxproj/pblexer.h \
    pbxproj/pbparser.h \
    pbxproj/pbprojectmodel.h \
    pbxproj/pbenums.h \
    xcodeprojectbuildoptionspage.h \
    xcodemanager.h \
    xcodeproject.h \
    xcodeprojectnode.h \
    xcodeprojectfile.h

# Qt Creator linking

## set the QTC_SOURCE environment variable to override the setting here
QTCREATOR_SOURCES = $$(QTC_SOURCE)
isEmpty(QTCREATOR_SOURCES):QTCREATOR_SOURCES=/home/sergey/Applications/Opensource/ubuntu-qtc/qt-creator-2.7.0

## set the QTC_BUILD environment variable to override the setting here
IDE_BUILD_TREE = $$(QTC_BUILD)
isEmpty(IDE_BUILD_TREE):IDE_BUILD_TREE=/home/sergey/Applications/Opensource/ubuntu-qtc/build-qtcreator-qt5_clang_x64-Release

## uncomment to build plugin into user config directory
## <localappdata>/plugins/<ideversion>
##    where <localappdata> is e.g.
##    "%LOCALAPPDATA%\QtProject\qtcreator" on Windows Vista and later
##    "$XDG_DATA_HOME/data/QtProject/qtcreator" or "~/.local/share/data/QtProject/qtcreator" on Linux
##    "~/Library/Application Support/QtProject/Qt Creator" on Mac
USE_USER_DESTDIR = yes

PROVIDER = DigiaPlc

include($$QTCREATOR_SOURCES/src/qtcreatorplugin.pri)
include($$QTCREATOR_SOURCES/src/plugins/coreplugin/coreplugin.pri)
include($$QTCREATOR_SOURCES/src/plugins/projectexplorer/projectexplorer.pri)
include($$QTCREATOR_SOURCES/src/plugins/cpptools/cpptools.pri)

LIBS += -L$$IDE_PLUGIN_PATH/QtProject

OTHER_FILES += \
    pbxproj/pbparser_test_1.pbxproj \
    XCodeProject.mimetypes.xml \
    XCodeProjectManager.json

RESOURCES += \
    xcodeproject.qrc

