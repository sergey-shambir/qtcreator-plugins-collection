#include "gobuildconfiguration.h"
#include "golangconstants.h"
#include "gotoolchain.h"
#include "gokitinformation.h"
#include "goproject.h"
#include "goprojectitem.h"

#include <projectexplorer/namedwidget.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/task.h>
#include <projectexplorer/target.h>
#include <projectexplorer/kitinformation.h>
#include <projectexplorer/kit.h>
#include <projectexplorer/buildinfo.h>
#include <projectexplorer/project.h>
#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/ioutputparser.h>
#include <projectexplorer/ansifilterparser.h>
#include <utils/qtcassert.h>
#include <utils/qtcprocess.h>
#include <utils/osspecificaspects.h>
#include <coreplugin/mimedatabase.h>

namespace GoLang {

const char GO_BUILDSTEP_ISCLEAN_KEYC[] = "GoLang.GoBuildStep.IsCleanStep";

namespace {
//forked from projectexplorer because AnsiParser is not exported
enum AnsiState {
    PLAIN,
    ANSI_START,
    ANSI_CSI,
    ANSI_SEQUENCE,
    ANSI_WAITING_FOR_ST,
    ANSI_ST_STARTED
};

QString filterLine(const QString &line)
{
    QString result;
    result.reserve(line.count());

    static AnsiState state = PLAIN;
    foreach (const QChar c, line) {
        unsigned int val = c.unicode();
        switch (state) {
            case PLAIN:
                if (val == 27) // 'ESC'
                    state = ANSI_START;
                else if (val == 155) // equivalent to 'ESC'-'['
                    state = ANSI_CSI;
                else
                    result.append(c);
                break;
            case ANSI_START:
                if (val == 91) // [
                    state = ANSI_CSI;
                else if (val == 80 || val == 93 || val == 94 || val == 95) // 'P', ']', '^' and '_'
                    state = ANSI_WAITING_FOR_ST;
                else if (val >= 64 && val <= 95)
                    state = PLAIN;
                else
                    state = ANSI_SEQUENCE;
                break;
            case ANSI_CSI:
                if (val >= 64 && val <= 126) // Anything between '@' and '~'
                    state = PLAIN;
                break;
            case ANSI_SEQUENCE:
                if (val >= 64 && val <= 95) // Anything between '@' and '_'
                    state = PLAIN;
                break;
            case ANSI_WAITING_FOR_ST:
                if (val == 7) // 'BEL'
                    state = PLAIN;
                if (val == 27) // 'ESC'
                    state = ANSI_ST_STARTED;
                break;
            case ANSI_ST_STARTED:
                if (val == 92) // '\'
                    state = PLAIN;
                else
                    state = ANSI_WAITING_FOR_ST;
                break;
        }
    }
    return result;
}
}

GoBuildConfiguration::GoBuildConfiguration(ProjectExplorer::Target *target) :
    ProjectExplorer::BuildConfiguration(target,Constants::GO_BUILDCONFIGURATION_ID)
{
}

GoBuildConfiguration::GoBuildConfiguration(ProjectExplorer::Target *target, GoBuildConfiguration *other)
    : ProjectExplorer::BuildConfiguration(target,other)
{

}

ProjectExplorer::NamedWidget *GoBuildConfiguration::createConfigWidget()
{
    return new ProjectExplorer::NamedWidget();
}

QList<ProjectExplorer::NamedWidget *> GoBuildConfiguration::createSubConfigWidgets()
{
    return BuildConfiguration::createSubConfigWidgets();
}

bool GoBuildConfiguration::isEnabled() const
{
    return true;
}

QString GoBuildConfiguration::disabledReason() const
{
    return QString();
}

ProjectExplorer::BuildConfiguration::BuildType GoBuildConfiguration::buildType() const
{
    return ProjectExplorer::BuildConfiguration::Debug;
}

void GoBuildConfiguration::setBuildDirectory(const Utils::FileName &dir)
{
    ProjectExplorer::BuildConfiguration::setBuildDirectory(dir);
}

bool GoBuildConfiguration::fromMap(const QVariantMap &map)
{
    if(!ProjectExplorer::BuildConfiguration::fromMap(map))
        return false;

    return true;
}

QVariantMap GoBuildConfiguration::toMap() const
{
    QVariantMap map = ProjectExplorer::BuildConfiguration::toMap();
    if(map.isEmpty())
        return map;

    return map;
}

/*!
 * \class GoBuildConfigurationFactory
 * Factory class to create GoBuildConfiguration
 * instances.
 */
GoBuildConfigurationFactory::GoBuildConfigurationFactory(QObject *parent)
    : ProjectExplorer::IBuildConfigurationFactory(parent)
{
}

GoBuildConfigurationFactory::~GoBuildConfigurationFactory()
{
}

int GoBuildConfigurationFactory::priority(const ProjectExplorer::Target *parent) const
{
    if(canHandle(parent))
        return 1;
    return -1;
}

QList<ProjectExplorer::BuildInfo *> GoBuildConfigurationFactory::availableBuilds(const ProjectExplorer::Kit *kit, const QString &projectFilePath) const
{
    QList<ProjectExplorer::BuildInfo *> result;

    QFileInfo projFileInfo(projectFilePath);
    ProjectExplorer::BuildInfo *goBuild = new ProjectExplorer::BuildInfo(this);
    goBuild->displayName = tr("Default");
    goBuild->supportsShadowBuild = false;
    goBuild->buildDirectory = Utils::FileName::fromString(projFileInfo.absolutePath());
    goBuild->kitId = kit->id();
    goBuild->typeName = QStringLiteral("Default");

    result << goBuild;

    return result;
}

QList<ProjectExplorer::BuildInfo *> GoBuildConfigurationFactory::availableBuilds(const ProjectExplorer::Target *parent) const
{
    QList<ProjectExplorer::BuildInfo *> result;
    if(!canHandle(parent))
        return result;
    return availableBuilds(parent->kit(),parent->project()->projectFilePath());
}

int GoBuildConfigurationFactory::priority(const ProjectExplorer::Kit *k, const QString &projectPath) const
{
    if(!k)
        return -1;

    GoLang::ToolChain* tc = GoToolChainKitInformation::toolChain(k);
    if(!tc)
        return -1;

    return (Core::MimeDatabase::findByFile(QFileInfo(projectPath)).matchesType(QLatin1String(Constants::GO_PROJECT_MIMETYPE))) ? 10 : -1;
}

QList<ProjectExplorer::BuildInfo *> GoBuildConfigurationFactory::availableSetups(const ProjectExplorer::Kit *k, const QString &projectPath) const
{
    if(!GoToolChainKitInformation::toolChain(k))
        return QList<ProjectExplorer::BuildInfo *>();

    if(!Core::MimeDatabase::findByFile(QFileInfo(projectPath)).matchesType(QLatin1String(Constants::GO_PROJECT_MIMETYPE)))
        return QList<ProjectExplorer::BuildInfo *>();

    return availableBuilds(k,projectPath);
}

GoBuildConfiguration *GoBuildConfigurationFactory::create(ProjectExplorer::Target *parent, const ProjectExplorer::BuildInfo *info) const
{
    QTC_ASSERT(info->factory() == this, return 0);
    QTC_ASSERT(info->kitId == parent->kit()->id(), return 0);
    QTC_ASSERT(!info->displayName.isEmpty(), return 0);
    QTC_ASSERT(canHandle(parent),return 0);


    ProjectExplorer::BuildInfo copy(*info);
    GoProject *project = static_cast<GoProject *>(parent->project());

    if (copy.buildDirectory.isEmpty())
        copy.buildDirectory = Utils::FileName::fromString(project->projectDirectory());

    GoBuildConfiguration *bc = new GoBuildConfiguration(parent);
    bc->setDisplayName(copy.displayName);
    bc->setDefaultDisplayName(copy.displayName);

    ProjectExplorer::BuildStepList *buildSteps = bc->stepList(ProjectExplorer::Constants::BUILDSTEPS_BUILD);
    ProjectExplorer::BuildStepList *cleanSteps = bc->stepList(ProjectExplorer::Constants::BUILDSTEPS_CLEAN);

    GoBuildStep *build = new GoBuildStep(buildSteps);
    buildSteps->insertStep(0, build);

    GoBuildStep *cleanStep = new GoBuildStep(cleanSteps);
    cleanSteps->insertStep(0, cleanStep);
    cleanStep->setIsCleanStep(true);

    bc->setBuildDirectory(Utils::FileName::fromString(copy.buildDirectory.toString()));
    return bc;
}

bool GoBuildConfigurationFactory::canRestore(const ProjectExplorer::Target *parent, const QVariantMap &map) const
{
    if (!canHandle(parent))
        return false;
    return ProjectExplorer::idFromMap(map) == Constants::GO_BUILDCONFIGURATION_ID;
}

GoBuildConfiguration *GoBuildConfigurationFactory::restore(ProjectExplorer::Target *parent, const QVariantMap &map)
{
    if (!canRestore(parent, map))
        return 0;
    GoBuildConfiguration *bc = new GoBuildConfiguration(parent);
    if (bc->fromMap(map))
        return bc;

    delete bc;
    return 0;
}

bool GoBuildConfigurationFactory::canClone(const ProjectExplorer::Target *parent, ProjectExplorer::BuildConfiguration *product) const
{
    if (!canHandle(parent))
        return false;

    return product->id() == Constants::GO_BUILDCONFIGURATION_ID;
}

GoBuildConfiguration *GoBuildConfigurationFactory::clone(ProjectExplorer::Target *parent, ProjectExplorer::BuildConfiguration *product)
{
    if (!canClone(parent, product))
        return 0;
    GoBuildConfiguration *old = static_cast<GoBuildConfiguration *>(product);
    return new GoBuildConfiguration(parent, old);
}

bool GoBuildConfigurationFactory::canHandle(const ProjectExplorer::Target *t) const
{
    QTC_ASSERT(t, return false);
    if (!t->project()->supportsKit(t->kit()))
        return false;
    return qobject_cast<GoLang::GoProject *>(t->project());
}

GoBuildStep::GoBuildStep(ProjectExplorer::BuildStepList *bsl)
    : BuildStep(bsl,Constants::GO_GOSTEP_ID),
      m_future(0),
      m_process(0),
      m_outputParserChain(0),
      m_state(Init)
{
    setIsCleanStep(false);
}

GoBuildStep::GoBuildStep(ProjectExplorer::BuildStepList *bsl, GoBuildStep *bs)
    : BuildStep(bsl,bs),
      m_future(0),
      m_process(0),
      m_outputParserChain(0),
      m_state(Init)
{
    setIsCleanStep(bs->m_clean);
}

GoBuildStep::~GoBuildStep()
{
    stopProcess();

    if(m_outputParserChain)
        delete m_outputParserChain;
}

void GoBuildStep::setIsCleanStep(const bool set)
{
    if(m_clean == set)
        return;

    m_clean = set;
    if(m_clean)
        setDisplayName(tr("Run Go clean"));
    else
        setDisplayName(tr("Run Go install"));
}

bool GoBuildStep::isCleanStep() const
{
    return m_clean;
}

bool GoBuildStep::init()
{
    m_tasks.clear();

    ProjectExplorer::BuildConfiguration *bc = buildConfiguration();
    if (!bc){
        ProjectExplorer::Task t(ProjectExplorer::Task::Error
                                ,tr("No valid BuildConfiguration set for step: %1").arg(displayName())
                                ,Utils::FileName(),-1
                                ,ProjectExplorer::Constants::TASK_CATEGORY_BUILDSYSTEM);
        m_tasks.append(t);

        //GoBuildStep::run will stop if tasks exist
        return true;

    }

    ProjectExplorer::Kit* kit = bc->target()->kit();
    GoLang::ToolChain* tc = GoLang::GoToolChainKitInformation::toolChain(kit);
    if(!tc){
        ProjectExplorer::Task t(ProjectExplorer::Task::Error
                                ,tr("No valid Go Toolchain set for kit: %1").arg(kit->displayName())
                                ,Utils::FileName(),-1
                                ,ProjectExplorer::Constants::TASK_CATEGORY_BUILDSYSTEM);
        m_tasks.append(t);

        //GoBuildStep::run will stop if tasks exist
        return true;

    }

    ProjectExplorer::IOutputParser *parser = target()->kit()->createOutputParser();
    if (parser)
        setOutputParser(parser);

    return true;
}

void GoBuildStep::run(QFutureInterface<bool> &fi)
{
    m_future = &fi;

    if (m_tasks.size()) {
        foreach (const ProjectExplorer::Task& task, m_tasks) {
            addTask(task);
        }
        emit addOutput(tr("Configuration is invalid. Aborting build")
                       ,ProjectExplorer::BuildStep::ErrorMessageOutput);
        handleFinished(false);
        return;
    }

    m_state = Init;
    fi.setProgressRange(0,2);
    startNextStep();
}

ProjectExplorer::BuildStepConfigWidget *GoBuildStep::createConfigWidget()
{
    return new ProjectExplorer::SimpleBuildStepConfigWidget(this);
}

void GoBuildStep::cancel()
{
    stopProcess();
    m_future->reportCanceled();
    handleFinished(false);
}

bool GoBuildStep::fromMap(const QVariantMap &map)
{
    if(!BuildStep::fromMap(map))
        return false;

    setIsCleanStep(map.value(QLatin1String(GO_BUILDSTEP_ISCLEAN_KEYC),false).toBool());
    return true;
}

QVariantMap GoBuildStep::toMap() const
{
    QVariantMap map = BuildStep::toMap();
    map.insert(QLatin1String(GO_BUILDSTEP_ISCLEAN_KEYC),m_clean);
    return map;
}

GoProject *GoBuildStep::goProject() const
{
    return qobject_cast<GoProject*>(target()->project());
}

void GoBuildStep::onProcessFinished()
{
    //make sure all data was processed
    onProcessStdErr();
    onProcessStdOut();

    if (m_outputParserChain)
        m_outputParserChain->flush();

    if(m_process->exitCode() == 0 && m_process->exitStatus() == QProcess::NormalExit)
        emit addOutput(tr("The process %1 exited normally.").arg(m_process->program()),
                       ProjectExplorer::BuildStep::MessageOutput);
    else
        emit addOutput(tr("The process %1 exited with errors.").arg(m_process->program()),
                       ProjectExplorer::BuildStep::MessageOutput);

    startNextStep();
}

void GoBuildStep::onProcessStdOut()
{
    m_process->setReadChannel(QProcess::StandardOutput);
    while (m_process->canReadLine()) {
        QString line = QString::fromLocal8Bit(m_process->readLine());

        if (m_outputParserChain)
            m_outputParserChain->stdOutput(filterLine(line));
        emit addOutput(line, BuildStep::NormalOutput, BuildStep::DontAppendNewline);
    }
}

void GoBuildStep::onProcessStdErr()
{
    m_process->setReadChannel(QProcess::StandardError);
    while (m_process->canReadLine()) {
        QString line = QString::fromLocal8Bit(m_process->readLine());

        if (m_outputParserChain)
            m_outputParserChain->stdError(filterLine(line));
        emit addOutput(line, BuildStep::ErrorOutput, BuildStep::DontAppendNewline);
    }
}

void GoBuildStep::startNextStep()
{
    ProjectExplorer::BuildConfiguration *bc = buildConfiguration();
    if(!bc) {
        cancel();
        return;
    }

    Utils::Environment env = bc->environment();
    // Force output to english for the parsers. Do this here and not in the toolchain's
    // addToEnvironment() to not screw up the users run environment.
    env.set(QStringLiteral("LC_ALL"), QStringLiteral("C"));

    env.prependOrSet(QStringLiteral("GOPATH"),bc->target()->project()->projectDirectory(),
                     Utils::OsSpecificAspects(Utils::HostOsInfo::hostOs()).pathListSeparator());
    env.set(QStringLiteral("GOBIN") ,bc->target()->project()->projectDirectory()+QStringLiteral("/bin"));

    ProjectExplorer::Kit* kit = bc->target()->kit();
    GoLang::ToolChain* tc = GoLang::GoToolChainKitInformation::toolChain(kit);
    if(!tc) {
        cancel();
        return;
    }

    ProjectExplorer::ProcessParameters params;
    params.setMacroExpander(bc->macroExpander());

    //setup process parameters
    params.setWorkingDirectory(bc->buildDirectory().toString());
    params.setCommand(tc->compilerCommand().toString());
    params.setEnvironment(env);

    GoProject *pro = goProject();
    if(!pro) {
        cancel();
        return;
    }

    switch(m_state) {
        case Init: {
            m_state = GoGet;

            if(m_clean) {
                startNextStep();
                return;
            }

            QStringList packages;
            foreach(GoBaseTargetItem *target,pro->buildTargets()) {
                GoPackageItem* pck = qobject_cast<GoPackageItem*>(target);
                if(!pck)
                    continue;
                packages << pck->name();
            }

            if(!packages.size()) {
                m_state = GoGet;
                startNextStep();
                return;
            }

            QStringList arguments;
            arguments << QStringLiteral("get")
                      << QStringLiteral("-d") //only download
                      << QStringLiteral("-u") //update packages if possible
                      << packages;

            params.setArguments(Utils::QtcProcess::joinArgs(arguments));
            m_future->setProgressValueAndText(0,tr("Running go-get"));
            startProcess(params);
            emit addOutput(tr("Running command go %1").arg(Utils::QtcProcess::joinArgs(arguments)),
                           ProjectExplorer::BuildStep::MessageOutput);
            break;
        }
        case GoGet:{
            m_state = GoBuild;

            //only care about the last process output when not in clean mode
            if(!m_clean && !processSucceeded()) {
                handleFinished(false);
                return;
            }

            QStringList packages;
            foreach(GoBaseTargetItem *target,pro->buildTargets()) {
                GoApplicationItem* pck = qobject_cast<GoApplicationItem*>(target);
                if(!pck)
                    continue;
                packages << pck->name();
            }

            QStringList arguments;
            if(!m_clean) {
                arguments << QStringLiteral("install")
                          << QStringLiteral("-v")  //verbose output
                          << QStringLiteral("-x"); //show commands
                m_future->setProgressValueAndText(1,tr("Building"));
            } else {
                arguments << QStringLiteral("clean")
                          << QStringLiteral("-i")
                          << QStringLiteral("-r");
                m_future->setProgressValueAndText(1,tr("Cleaning"));
            }
            arguments << packages.join(QStringLiteral(" "));

            params.setArguments(Utils::QtcProcess::joinArgs(arguments));
            startProcess(params);
            emit addOutput(tr("Running command go %1").arg(Utils::QtcProcess::joinArgs(arguments)),
                           ProjectExplorer::BuildStep::MessageOutput);
            break;
        }
        case GoBuild:{
            //m_future->setProgressValueAndText(2,tr("Finished"));
            handleFinished(processSucceeded());
            break;
        }
        default:{
            return;
        }
    }
}

void GoBuildStep::handleFinished(bool result)
{
    stopProcess();
    setOutputParser(0);

    m_state = Finished;
    m_tasks.clear();
    m_future->reportResult(result);
    m_future = 0;

    emit finished();
}

void GoBuildStep::startProcess(const ProjectExplorer::ProcessParameters &params)
{
    stopProcess();

    qDebug()<<"Start "<<params.effectiveCommand()<<" with args: "<<params.effectiveArguments();

    if(m_outputParserChain)
        m_outputParserChain->setWorkingDirectory(params.effectiveWorkingDirectory());

    m_process = new Utils::QtcProcess;
    connect(m_process,SIGNAL(readyReadStandardError()),this,SLOT(onProcessStdErr()));
    connect(m_process,SIGNAL(readyReadStandardOutput()),this,SLOT(onProcessStdOut()));
    connect(m_process,SIGNAL(finished(int)),this,SLOT(onProcessFinished()));

#ifdef Q_OS_WIN
    m_process->setUseCtrlCStub(true);
#endif
    m_process->setEnvironment(params.environment());
    m_process->setWorkingDirectory(params.workingDirectory());
    m_process->setCommand(params.effectiveCommand(), params.effectiveArguments());
    m_process->start();
    if (!m_process->waitForStarted()) {
        handleFinished(false);
    }
}

void GoBuildStep::stopProcess()
{
    if (m_process) {
        m_process->disconnect(this);

        if(m_process->state() != QProcess::NotRunning) {
            m_process->terminate();
            if(!m_process->waitForFinished(500))
                m_process->kill();
        }

        m_process->deleteLater();
        m_process = 0;
    }
}

void GoBuildStep::setOutputParser(ProjectExplorer::IOutputParser *parser)
{
    delete m_outputParserChain;
    m_outputParserChain = parser;

    if (m_outputParserChain) {
        connect(m_outputParserChain, SIGNAL(addOutput(QString,ProjectExplorer::BuildStep::OutputFormat)),
                this, SLOT(outputAdded(QString,ProjectExplorer::BuildStep::OutputFormat)));
        //connect(m_outputParserChain, SIGNAL(addTask(ProjectExplorer::Task)),
        //        this, SLOT(taskAdded(ProjectExplorer::Task)));
    }
}

void GoBuildStep::outputAdded(const QString &string, ProjectExplorer::BuildStep::OutputFormat format)
{
    emit addOutput(string, format, BuildStep::DontAppendNewline);
}

bool GoBuildStep::processSucceeded() const
{
    if (m_outputParserChain && m_outputParserChain->hasFatalErrors())
        return false;

    return m_process->exitCode() == 0 && m_process->exitStatus() == QProcess::NormalExit;
}

/*!
 * \class GoBuildStepFactory
 * Factory class to create Go buildsteps
 * build steps
 */

namespace {
enum {
    GOSTEP_BUILDSUFFIX,
    GOSTEP_CLEANSUFFIX
};
}

QList<Core::Id> GoBuildStepFactory::availableCreationIds(ProjectExplorer::BuildStepList *parent) const
{
    if(!canHandle(parent->target()))
        return QList<Core::Id>();

    if (parent->id() == ProjectExplorer::Constants::BUILDSTEPS_BUILD)
        return QList<Core::Id>() << Core::Id(Constants::GO_GOSTEP_ID).withSuffix(GOSTEP_BUILDSUFFIX);
    else if(parent->id() == ProjectExplorer::Constants::BUILDSTEPS_CLEAN)
        return QList<Core::Id>() << Core::Id(Constants::GO_GOSTEP_ID).withSuffix(GOSTEP_CLEANSUFFIX);

    return QList<Core::Id>();
}

/*!
 * \brief GoBuildConfigurationFactory::canHandle
 * checks if we can create buildconfigurations for the given target
 */
bool GoBuildStepFactory::canHandle(const ProjectExplorer::Target *t) const
{
    QTC_ASSERT(t, return false);
    if (!t->project()->supportsKit(t->kit()))
        return false;

    return t->project()->id() == Core::Id(GoLang::Constants::GO_PROJECT_ID);
}

QString GoBuildStepFactory::displayNameForId(const Core::Id id) const
{
    if (id == Core::Id(Constants::GO_GOSTEP_ID).withSuffix(GOSTEP_BUILDSUFFIX))
        return tr("Run Go get/install", "Display name for GoStep id.");
    else if (id == Core::Id(Constants::GO_GOSTEP_ID).withSuffix(GOSTEP_CLEANSUFFIX))
        return tr("Run Go clean", "Display name for GoStep clean id.");
    return QString();
}

bool GoBuildStepFactory::canCreate(ProjectExplorer::BuildStepList *parent, const Core::Id id) const
{
    if (canHandle(parent->target()))
        return availableCreationIds(parent).contains(id);
    return false;
}

ProjectExplorer::BuildStep *GoBuildStepFactory::create(ProjectExplorer::BuildStepList *parent, const Core::Id id)
{
    if (!canCreate(parent, id))
        return 0;

    if ( id == Core::Id(Constants::GO_GOSTEP_ID).withSuffix(GOSTEP_BUILDSUFFIX) ) {
        GoBuildStep *step = new GoBuildStep(parent);
        return step;
    } else if ( id == Core::Id(Constants::GO_GOSTEP_ID).withSuffix(GOSTEP_CLEANSUFFIX) ) {
        GoBuildStep *step = new GoBuildStep(parent);
        step->setIsCleanStep(true);
        return step;
    }
    return 0;
}

bool GoBuildStepFactory::canRestore(ProjectExplorer::BuildStepList *parent, const QVariantMap &map) const
{
    if(!canHandle(parent->target()))
        return false;
    return ProjectExplorer::idFromMap(map) == Core::Id(Constants::GO_GOSTEP_ID);
}

ProjectExplorer::BuildStep *GoBuildStepFactory::restore(ProjectExplorer::BuildStepList *parent, const QVariantMap &map)
{
    if (!canRestore(parent, map))
        return 0;

    ProjectExplorer::BuildStep* step = new GoBuildStep(parent);
    if(step->fromMap(map))
        return step;

    delete step;
    return 0;
}

bool GoBuildStepFactory::canClone(ProjectExplorer::BuildStepList *parent, ProjectExplorer::BuildStep *product) const
{
    return canCreate(parent, product->id());
}

ProjectExplorer::BuildStep *GoBuildStepFactory::clone(ProjectExplorer::BuildStepList *parent, ProjectExplorer::BuildStep *product)
{
    if (!canClone(parent, product))
        return 0;

    if(product->id() == Core::Id(Constants::GO_GOSTEP_ID))
        return new GoBuildStep(parent,static_cast<GoBuildStep *>(product));

    QTC_ASSERT(false,return 0);
}

} // namespace GoLang
