TEMPLATE = lib
TARGET = VcProjectManager
include(../build_settings.pri)
include(vcprojectmanager_dependencies.pri)
HEADERS = vcprojectmanagerplugin.h \
    vcprojectreader.h \
    vcprojectnodes.h \
    vcprojectmanagerconstants.h \
    vcprojectmanager_global.h \
    vcprojectmanager.h \
    vcprojectfile.h \
    vcproject.h \
    vcprojectbuildconfiguration.h \
    vcmakestep.h \
    vcprojectbuildoptionspage.h \
    msbuildoutputparser.h
SOURCES = vcprojectmanagerplugin.cpp \
    vcprojectreader.cpp \
    vcprojectnodes.cpp \
    vcprojectmanager.cpp \
    vcprojectfile.cpp \
    vcproject.cpp \
    vcprojectbuildconfiguration.cpp \
    vcmakestep.cpp \
    vcprojectbuildoptionspage.cpp \
    msbuildoutputparser.cpp

OTHER_FILES += \
    VcProject.mimetypes.xml

RESOURCES += \
    vcproject.qrc

OTHER_FILES += \
    VcProjectManagerPlugin.json
