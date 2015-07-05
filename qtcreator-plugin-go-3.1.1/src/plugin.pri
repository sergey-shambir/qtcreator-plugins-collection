PROVIDER = Canonical

## Where the Qt Creator headers are located at
QTCREATOR_SOURCES = $$(QTC_SOURCE)
isEmpty(QTCREATOR_SOURCES):QTCREATOR_SOURCES=/usr/src/qtcreator

## Where our plugin will be compiled to
IDE_BUILD_TREE = $$(QTC_BUILD)
isEmpty(IDE_BUILD_TREE):IDE_BUILD_TREE=/usr/lib/x86_64-linux-gnu/qtcreator/

isEmpty(UBUNTU_LOCAL_BUILD):UBUNTU_LOCAL_BUILD = $$(UBUNTU_QTC_PLUGIN_LOCALBUILD)
!isEmpty(UBUNTU_LOCAL_BUILD) {
    USE_USER_DESTDIR = yes
    PATHSTR = '\\"$${PWD}/../share/qtcreator\\"'

    DEFINES += UBUNTU_RESOURCE_PATH_LOCAL=\"$${PATHSTR}\" UBUNTU_BUILD_LOCAL
}


include($$QTCREATOR_SOURCES/src/qtcreatorplugin.pri)

INCLUDEPATH += $$QTCREATOR_SOURCES/src/

## make sure the QtProject libs are available when building locally
!isEmpty(UBUNTU_LOCAL_BUILD) {
    LIBS += -L$$DESTDIRBASE/QtProject/$$DESTDIRAPPNAME/plugins/$$QTCREATOR_VERSION/QtProject
}
LIBS += -L"$$[QT_INSTALL_LIBS]/qtcreator"
LIBS += -L"$$[QT_INSTALL_LIBS]/qtcreator/plugins/QtProject"
