#include "gorunconfigurationfactory.h"
#include "gokitinformation.h"
#include "goproject.h"
#include "gorunconfiguration.h"
#include "gotoolchain.h"

#include <projectexplorer/target.h>
#include <projectexplorer/kitinformation.h>
#include <projectexplorer/abi.h>

namespace GoLang {
namespace Internal {

GoRunConfigurationFactory::GoRunConfigurationFactory(QObject *parent)
    : IRunConfigurationFactory(parent)
{

}

QList<Core::Id> GoRunConfigurationFactory::availableCreationIds(ProjectExplorer::Target *parent) const {
    if (!canHandle(parent))
        return QList<Core::Id>();

    QList<Core::Id> list;

    //our tests already checked if the types match
    GoProject *pro = static_cast<GoProject*>(parent->project());
    foreach(const GoBaseTargetItem *t,pro->buildTargets()) {
        const GoApplicationItem *app = qobject_cast<const GoApplicationItem*>(t);
        if(!app)
            continue;

        Core::Id id(Constants::GO_RUNCONFIG_ID);
        list << id.withSuffix(app->name());
    }
    return list;
}

QString GoRunConfigurationFactory::displayNameForId(const Core::Id id) const {

    QString suffix = id.suffixAfter(Constants::GO_RUNCONFIG_ID);
    return suffix;
}

bool GoRunConfigurationFactory::canHandle(ProjectExplorer::Target *parent) const
{
    if (!parent->project())
        return false;

    if (parent->project()->id() != Constants::GO_PROJECT_ID)
        return false;

    if (!parent->project()->supportsKit(parent->kit()))
        return false;

    GoLang::ToolChain *tc = GoLang::GoToolChainKitInformation::toolChain(parent->kit());
    if (!tc ||  !ProjectExplorer::Abi::hostAbi().isCompatibleWith(tc->targetAbi()))
        return false;

    if(ProjectExplorer::DeviceKitInformation::deviceId(parent->kit()) != ProjectExplorer::Constants::DESKTOP_DEVICE_ID)
        return false;

    return true;
}

bool GoRunConfigurationFactory::canCreate(ProjectExplorer::Target *parent,
                                          const Core::Id id) const {
    if (!canHandle(parent))
        return false;

    return availableCreationIds(parent).contains(id);
}

bool GoRunConfigurationFactory::canRestore(ProjectExplorer::Target *parent, const QVariantMap &map) const {
    if (!parent)
        return false;

    if (!ProjectExplorer::idFromMap(map).toSetting().toByteArray().startsWith(Constants::GO_RUNCONFIG_ID ))
        return false;

    return true;
}

ProjectExplorer::RunConfiguration *GoRunConfigurationFactory::doCreate(ProjectExplorer::Target *parent, const Core::Id id) {
    if (!canCreate(parent, id))
        return 0;

    QString suffix = id.suffixAfter(Constants::GO_RUNCONFIG_ID);
    if(suffix.isEmpty())
        return 0;

    return new GoRunConfiguration(parent,id);
}

ProjectExplorer::RunConfiguration *GoRunConfigurationFactory::doRestore(ProjectExplorer::Target *parent, const QVariantMap &map) {
    if (!canRestore(parent, map))
        return NULL;

    ProjectExplorer::RunConfiguration *conf = new GoRunConfiguration(parent,ProjectExplorer::idFromMap(map));
    if(!conf)
        return NULL;
    if(!conf->fromMap(map)) {
        delete conf;
        return NULL;
    }
    return conf;
}

bool GoRunConfigurationFactory::canClone(ProjectExplorer::Target *parent, ProjectExplorer::RunConfiguration *product) const {
    return canCreate(parent,product->id());
}

ProjectExplorer::RunConfiguration *GoRunConfigurationFactory::clone(ProjectExplorer::Target *parent,
                                                                    ProjectExplorer::RunConfiguration *source) {
    if (!canClone(parent, source))
        return NULL;

    return new GoRunConfiguration(parent,static_cast<GoRunConfiguration*>(source));
}


} // namespace Internal
} // namespace GoLang
