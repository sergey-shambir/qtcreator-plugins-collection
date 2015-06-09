#ifndef GOLANG_INTERNAL_GORUNCONFIGURATIONFACTORY_H
#define GOLANG_INTERNAL_GORUNCONFIGURATIONFACTORY_H

#include <projectexplorer/runconfiguration.h>

namespace GoLang {
namespace Internal {

class GoRunConfigurationFactory : public ProjectExplorer::IRunConfigurationFactory
{
public:
    GoRunConfigurationFactory(QObject *parent = 0);
    QList<Core::Id> availableCreationIds(ProjectExplorer::Target *parent) const override;
    QString displayNameForId(const Core::Id id) const override;
    bool canHandle(ProjectExplorer::Target *parent) const;
    bool canCreate(ProjectExplorer::Target *parent, const Core::Id id) const override;
    ProjectExplorer::RunConfiguration *doRestore(ProjectExplorer::Target *parent, const QVariantMap &map) override;
    bool canClone(ProjectExplorer::Target *parent, ProjectExplorer::RunConfiguration *product) const override;
    ProjectExplorer::RunConfiguration *clone(ProjectExplorer::Target *parent, ProjectExplorer::RunConfiguration *source) override;
    bool canRestore(ProjectExplorer::Target *parent, const QVariantMap &map) const override;
    ProjectExplorer::RunConfiguration *doCreate(ProjectExplorer::Target *parent, const Core::Id id) override;
};

} // namespace Internal
} // namespace GoLang

#endif // GOLANG_INTERNAL_GORUNCONFIGURATIONFACTORY_H
