#include "golangplugin.h"
#include "golangconstants.h"
#include "goprojectmanager.h"
#include "goproject.h"
#include "toolchainmanager.h"
#include "toolchainoptionspage.h"
#include "gotoolchain.h"
#include "gobuildconfiguration.h"
#include "gokitinformation.h"
#include "gorunconfigurationfactory.h"
#include "goapplicationwizard.h"

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/fileiconprovider.h>
#include <coreplugin/icore.h>
#include <coreplugin/mimedatabase.h>
#include <projectexplorer/toolchainmanager.h>

#include <qtsupport/qtsupportconstants.h>

#include <QtPlugin>

#include <QApplication>
#include <QMessageBox>
#include <QPushButton>

using namespace GoLang::Internal;

class GoLangPluginFeatureProvider : public Core::IFeatureProvider
{
    Core::FeatureSet availableFeatures(const QString & /* platform */) const {
        return Core::FeatureSet(Core::Id(GoLang::Constants::GO_SUPPORT_FEATURE));
    }

    QStringList availablePlatforms() const { return QStringList(); }
    QString displayNameForPlatform(const QString & /* platform */) const { return QString(); }
};

GoLangPlugin::GoLangPlugin()
{
    // Create your members
}

GoLangPlugin::~GoLangPlugin()
{
    // Unregister objects from the plugin manager's object pool
    // Delete members
}

bool GoLangPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    // Register objects in the plugin manager's object pool
    // Load settings
    // Add actions to menus
    // Connect to other plugins' signals
    // In the initialize function, a plugin can be sure that the plugins it
    // depends on have initialized their members.

    Q_UNUSED(arguments)

    using namespace Core;

    const QLatin1String mimetypesXml(":/goproject/GoProjectManager.mimetypes.xml");

    if (!MimeDatabase::addMimeTypes(mimetypesXml, errorString))
        return false;

    addAutoReleasedObject(new Internal::Manager);
    addAutoReleasedObject(new ToolChainManager);
    addAutoReleasedObject(new Internal::ToolChainOptionsPage);
    addAutoReleasedObject(new GoToolChainFactory);
    addAutoReleasedObject(new GoBuildConfigurationFactory);
    addAutoReleasedObject(new GoBuildStepFactory);
    addAutoReleasedObject(new GoRunConfigurationFactory);
    addAutoReleasedObject(new GoLangPluginFeatureProvider);

    addAutoReleasedObject(new ProjectExplorer::CustomWizardFactory<GoApplicationWizard>
                              (QLatin1String("goapp-project"), Core::IWizard::ProjectWizard));

    ProjectExplorer::KitManager::registerKitInformation(new GoToolChainKitInformation);

    connect(ProjectExplorer::ToolChainManager::instance(),SIGNAL(toolChainsLoaded()),this,SLOT(restoreToolChains()));    
    return true;
}

void GoLangPlugin::extensionsInitialized()
{
    // Retrieve objects from the plugin manager's object pool
    // In the extensionsInitialized function, a plugin can be sure that all
    // plugins that depend on it are completely initialized.
}

ExtensionSystem::IPlugin::ShutdownFlag GoLangPlugin::aboutToShutdown()
{
    // Save settings
    // Disconnect from signals that are not needed during shutdown
    // Hide UI (if you add UI that is not in the main window directly)
    return SynchronousShutdown;
}

void GoLangPlugin::restoreToolChains()
{
    //this has to be called after Projectexplorer has initialized the toolchains, but before the Kits are loaded
    ToolChainManager::restoreToolChains();
}

Q_EXPORT_PLUGIN2(Go, GoPlugin)

