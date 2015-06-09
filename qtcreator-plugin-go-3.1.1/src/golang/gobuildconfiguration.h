#ifndef GOLANG_INTERNAL_GOBUILDCONFIGURATION_H
#define GOLANG_INTERNAL_GOBUILDCONFIGURATION_H

#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/abi.h>
#include <projectexplorer/abstractprocessstep.h>
#include <projectexplorer/task.h>
#include <utils/qtcprocess.h>

namespace GoLang {

class GoProject;

class GoBuildConfiguration : public ProjectExplorer::BuildConfiguration
{
    Q_OBJECT
public:
    GoBuildConfiguration(ProjectExplorer::Target *target);
    GoBuildConfiguration(ProjectExplorer::Target *target, GoBuildConfiguration *other);

    // BuildConfiguration interface
    virtual ProjectExplorer::NamedWidget *createConfigWidget() override;
    virtual QList<ProjectExplorer::NamedWidget *> createSubConfigWidgets() override;
    virtual bool isEnabled() const override;
    virtual QString disabledReason() const override;
    virtual BuildType buildType() const override;

protected:
    virtual void setBuildDirectory(const Utils::FileName &dir) override;

    // ProjectConfiguration interface
public:
    virtual bool fromMap(const QVariantMap &map) override;
    virtual QVariantMap toMap() const override;

    friend class GoBuildConfigurationFactory;
};

class GoBuildConfigurationFactory : public ProjectExplorer::IBuildConfigurationFactory
{
    Q_OBJECT
public:
    GoBuildConfigurationFactory(QObject *parent = 0);
    ~GoBuildConfigurationFactory();

    // IBuildConfigurationFactory interface
    virtual int priority(const ProjectExplorer::Target *parent) const override;
    virtual QList<ProjectExplorer::BuildInfo *> availableBuilds(const ProjectExplorer::Target *parent) const override;
    virtual int priority(const ProjectExplorer::Kit *k, const QString &projectPath) const override;
    virtual QList<ProjectExplorer::BuildInfo *> availableSetups(const ProjectExplorer::Kit *k, const QString &projectPath) const override;
    virtual GoBuildConfiguration *create(ProjectExplorer::Target *parent, const ProjectExplorer::BuildInfo *info) const override;
    virtual bool canRestore(const ProjectExplorer::Target *parent, const QVariantMap &map) const override;
    virtual GoBuildConfiguration *restore(ProjectExplorer::Target *parent, const QVariantMap &map);
    virtual bool canClone(const ProjectExplorer::Target *parent, ProjectExplorer::BuildConfiguration *product) const override;
    virtual GoBuildConfiguration*clone(ProjectExplorer::Target *parent, ProjectExplorer::BuildConfiguration *product) override;
private:
    virtual QList<ProjectExplorer::BuildInfo *> availableBuilds(const ProjectExplorer::Kit *kit, const QString &projectFilePath) const;
    bool canHandle(const ProjectExplorer::Target *t) const;

};

class GoBuildStepFactory : public ProjectExplorer::IBuildStepFactory
{
    Q_OBJECT

public:
    // IBuildStepFactory interface
    virtual QList<Core::Id> availableCreationIds(ProjectExplorer::BuildStepList *parent) const override;
    virtual QString displayNameForId(const Core::Id id) const override;
    virtual bool canCreate(ProjectExplorer::BuildStepList *parent, const Core::Id id) const override;
    virtual ProjectExplorer::BuildStep *create(ProjectExplorer::BuildStepList *parent, const Core::Id id) override;
    virtual bool canRestore(ProjectExplorer::BuildStepList *parent, const QVariantMap &map) const override;
    virtual ProjectExplorer::BuildStep *restore(ProjectExplorer::BuildStepList *parent, const QVariantMap &map) override;
    virtual bool canClone(ProjectExplorer::BuildStepList *parent, ProjectExplorer::BuildStep *product) const override;
    virtual ProjectExplorer::BuildStep *clone(ProjectExplorer::BuildStepList *parent, ProjectExplorer::BuildStep *product) override;

private:
    bool canHandle(const ProjectExplorer::Target *t) const;
};

class GoBuildStep : public ProjectExplorer::BuildStep
{
    Q_OBJECT
public:

    enum State {
        Init,
        GoGet,
        GoBuild,
        Finished
    };

    GoBuildStep(ProjectExplorer::BuildStepList *bsl);
    GoBuildStep(ProjectExplorer::BuildStepList *bsl, GoBuildStep *bs);
    ~GoBuildStep();

    void setIsCleanStep (const bool set = true);
    bool isCleanStep () const;

    // BuildStep interface
    virtual bool init() override;
    virtual void run(QFutureInterface<bool> &fi) override;
    virtual ProjectExplorer::BuildStepConfigWidget *createConfigWidget() override;
    virtual bool runInGuiThread() const override { return true; }
    virtual void cancel() override;

    // ProjectConfiguration interface
    virtual bool fromMap(const QVariantMap &map) override;
    virtual QVariantMap toMap() const override;

    GoProject *goProject () const;

protected slots:
    void onProcessFinished  ();
    void onProcessStdOut ();
    void onProcessStdErr ();
    void outputAdded(const QString &string, ProjectExplorer::BuildStep::OutputFormat format);

protected:
    void startNextStep ();
    void handleFinished (bool result);
    void startProcess  (const ProjectExplorer::ProcessParameters &params);
    void stopProcess ();
    void setOutputParser (ProjectExplorer::IOutputParser *parser);
    bool processSucceeded () const;


private:
    QList<ProjectExplorer::Task> m_tasks;
    QFutureInterface<bool> *m_future;
    Utils::QtcProcess *m_process;

    ProjectExplorer::IOutputParser *m_outputParserChain;

    bool m_clean;
    State m_state;

    friend class GoBuildStepFactory;
};

} // namespace GoLang

#endif // GOLANG_INTERNAL_GOBUILDCONFIGURATION_H
