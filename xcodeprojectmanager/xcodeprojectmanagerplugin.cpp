#include "xcodeprojectmanagerplugin.h"
#include "xcodeprojectmanagerconstants.h"
#include "xcodeprojectbuildoptionspage.h"
#include "xcodemanager.h"

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/mimedatabase.h>

#include <QAction>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenu>

#include <QtPlugin>

using namespace XCodeProjectManager::Internal;

XCodeProjectManagerPlugin::XCodeProjectManagerPlugin()
{
}

XCodeProjectManagerPlugin::~XCodeProjectManagerPlugin()
{
}

bool XCodeProjectManagerPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

    if (!Core::ICore::mimeDatabase()->addMimeTypes(QLatin1String(":vcproject/VcProject.mimetypes.xml"), errorString))
        return false;

//    XCodeProjectBuildOptionsPage *page = new XCodeProjectBuildOptionsPage();
//    addAutoReleasedObject(page);
    addAutoReleasedObject(new XCodeManager(/*page*/));
    
    return true;
}

void XCodeProjectManagerPlugin::extensionsInitialized()
{
}

ExtensionSystem::IPlugin::ShutdownFlag XCodeProjectManagerPlugin::aboutToShutdown()
{
    return SynchronousShutdown;
}

void XCodeProjectManagerPlugin::triggerAction()
{
    QMessageBox::information(Core::ICore::mainWindow(),
                             tr("Action triggered"),
                             tr("This is an action from XCodeProjectManager."));
}

Q_EXPORT_PLUGIN2(XCodeProjectManager, XCodeProjectManagerPlugin)

