#include "gorunconfiguration.h"
#include "golangconstants.h"
#include "goproject.h"
#include "goprojectitem.h"

#include <qtsupport/qtkitinformation.h>
#include <projectexplorer/target.h>
#include <projectexplorer/kit.h>
#include <utils/pathchooser.h>
#include <utils/detailswidget.h>
#include <utils/stringutils.h>
#include <projectexplorer/environmentaspect.h>
#include <projectexplorer/localenvironmentaspect.h>

#include <QFormLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QToolButton>

namespace GoLang {

const char USER_WORKING_DIRECTORY_KEYC[] = "GoLang.GoRunConfiguration.UserWorkingDirectory";
const char USE_TERMINAL_KEYC[] = "GoLang.GoRunConfiguration.UseTerminal";
const char COMMAND_KEYC[] = "GoLang.GoRunConfiguration.Command";
const char ARGUMENTS_KEYC[] = "GoLang.GoRunConfiguration.Arguments";

// Configuration widget
GoRunConfigurationWidget::GoRunConfigurationWidget(GoRunConfiguration *goRC, QWidget *parent)
    : QWidget(parent), m_ignoreChange(false), m_goRc(goRC)
{
    QFormLayout *fl = new QFormLayout();
    fl->setMargin(0);
    fl->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    QLineEdit *argumentsLineEdit = new QLineEdit();
    argumentsLineEdit->setText(goRC->commandLineArguments());
    connect(argumentsLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(setArguments(QString)));
    fl->addRow(tr("Arguments:"), argumentsLineEdit);

    m_workingDirectoryEdit = new Utils::PathChooser();
    m_workingDirectoryEdit->setExpectedKind(Utils::PathChooser::Directory);
    m_workingDirectoryEdit->setBaseDirectory(goRC->target()->project()->projectDirectory());
    m_workingDirectoryEdit->setPath(m_goRc->workingDirectory());

    ProjectExplorer::EnvironmentAspect *aspect
            = goRC->extraAspect<ProjectExplorer::EnvironmentAspect>();
    if (aspect) {
        connect(aspect, SIGNAL(environmentChanged()), this, SLOT(environmentWasChanged()));
        environmentWasChanged();
    }
    m_workingDirectoryEdit->setPromptDialogTitle(tr("Select Working Directory"));

    QToolButton *resetButton = new QToolButton();
    resetButton->setToolTip(tr("Reset to default"));
    resetButton->setIcon(QIcon(QLatin1String(Core::Constants::ICON_RESET)));

    QHBoxLayout *boxlayout = new QHBoxLayout();
    boxlayout->addWidget(m_workingDirectoryEdit);
    boxlayout->addWidget(resetButton);

    fl->addRow(tr("Working directory:"), boxlayout);

    QCheckBox *runInTerminal = new QCheckBox;
    fl->addRow(tr("Run in Terminal"), runInTerminal);

    m_detailsContainer = new Utils::DetailsWidget(this);
    m_detailsContainer->setState(Utils::DetailsWidget::NoSummary);

    QWidget *m_details = new QWidget(m_detailsContainer);
    m_detailsContainer->setWidget(m_details);
    m_details->setLayout(fl);

    QVBoxLayout *vbx = new QVBoxLayout(this);
    vbx->setMargin(0);
    vbx->addWidget(m_detailsContainer);

    connect(m_workingDirectoryEdit, SIGNAL(changed(QString)),
            this, SLOT(setWorkingDirectory()));

    connect(resetButton, SIGNAL(clicked()),
            this, SLOT(resetWorkingDirectory()));

    connect(runInTerminal, SIGNAL(toggled(bool)),
            this, SLOT(runInTerminalToggled(bool)));

    connect(m_goRc, SIGNAL(workingDirectoryChanged()),
            this, SLOT(workingDirectoryChanged()));

    connect(m_goRc,SIGNAL(enabledChanged()),this,SLOT(onRcEnabledChanged()));
    onRcEnabledChanged();
}

void GoRunConfigurationWidget::setWorkingDirectory()
{
    if (m_ignoreChange)
        return;
    m_ignoreChange = true;
    m_goRc->setWorkingDirectory(m_workingDirectoryEdit->rawPath());
    m_ignoreChange = false;
}

void GoRunConfigurationWidget::workingDirectoryChanged()
{
    if (!m_ignoreChange) {
        m_ignoreChange = true;
        m_workingDirectoryEdit->setPath(m_goRc->workingDirectory());
        m_ignoreChange = false;
    }
}

void GoRunConfigurationWidget::onRcEnabledChanged()
{
    setEnabled(m_goRc->isEnabled());
}

void GoRunConfigurationWidget::resetWorkingDirectory()
{
    // This emits a signal connected to workingDirectoryChanged()
    // that sets the m_workingDirectoryEdit
    m_goRc->setWorkingDirectory(QString());
}

void GoRunConfigurationWidget::runInTerminalToggled(bool toggled)
{
    m_goRc->setRunMode(toggled ? ProjectExplorer::LocalApplicationRunConfiguration::Console
                               : ProjectExplorer::LocalApplicationRunConfiguration::Gui);
}

void GoRunConfigurationWidget::environmentWasChanged()
{
    ProjectExplorer::EnvironmentAspect *aspect
            = m_goRc->extraAspect<ProjectExplorer::EnvironmentAspect>();
    QTC_ASSERT(aspect, return);
    m_workingDirectoryEdit->setEnvironment(aspect->environment());
}
void GoRunConfigurationWidget::setArguments(const QString &args)
{
    m_goRc->setCommandLineArguments(args);
}

GoRunConfiguration::GoRunConfiguration(ProjectExplorer::Target *t, const Core::Id &id)
    : LocalApplicationRunConfiguration(t,id),
      m_defaultWorkingDirectory(project()->projectDirectory()),
      m_commandName(id.suffixAfter(Constants::GO_RUNCONFIG_ID)),
      m_runMode(Gui)
{
    setDefaultDisplayName(m_commandName);
    setDisplayName(m_commandName);
    addExtraAspect(new ProjectExplorer::LocalEnvironmentAspect(this));
}

GoRunConfiguration::GoRunConfiguration(ProjectExplorer::Target *t, GoRunConfiguration *other)
    : LocalApplicationRunConfiguration(t,other),
      m_commandName(other->m_commandName),
      m_args(other->m_args)
{

}

QString GoRunConfiguration::executable() const
{
    foreach(const GoBaseTargetItem *t,project()->buildTargets()) {
        const GoApplicationItem *app = qobject_cast<const GoApplicationItem*>(t);
        if(t->name() == m_commandName) {
            return project()->projectDirectory()+QStringLiteral("/bin/")+m_commandName;
        }
    }
    return QString();
}

ProjectExplorer::LocalApplicationRunConfiguration::RunMode GoRunConfiguration::runMode() const
{
    return m_runMode;
}

QString GoRunConfiguration::workingDirectory() const
{
    QString wD = m_userWorkingDirectory.isEmpty() ? m_defaultWorkingDirectory : m_userWorkingDirectory;
    ProjectExplorer::EnvironmentAspect *aspect = extraAspect<ProjectExplorer::EnvironmentAspect>();
    QTC_ASSERT(aspect, return QString());
    return QDir::cleanPath(aspect->environment().expandVariables(
                Utils::expandMacros(wD, macroExpander())));
}

QString GoRunConfiguration::commandLineArguments() const
{
    return m_args;
}

void GoRunConfiguration::addToBaseEnvironment(Utils::Environment &env) const
{

}

QWidget *GoRunConfiguration::createConfigurationWidget()
{
    return new GoRunConfigurationWidget(this);
}

bool GoRunConfiguration::fromMap(const QVariantMap &map)
{
    if(!LocalApplicationRunConfiguration::fromMap(map))
        return false;

    m_runMode = map.value(QLatin1String(USE_TERMINAL_KEYC)).toBool() ? Console : Gui;
    m_commandName = map.value(QLatin1String(COMMAND_KEYC)).toString();
    m_args = map.value(QLatin1String(ARGUMENTS_KEYC)).toString();
    setWorkingDirectory(map.value(QLatin1String(USER_WORKING_DIRECTORY_KEYC)).toString());

    return true;
}

QVariantMap GoRunConfiguration::toMap() const
{
    QVariantMap map(ProjectExplorer::LocalApplicationRunConfiguration::toMap());
    if(map.isEmpty())
        return map;

    map.insert(QLatin1String(USER_WORKING_DIRECTORY_KEYC), m_userWorkingDirectory);
    map.insert(QLatin1String(USE_TERMINAL_KEYC), m_runMode == Console);
    map.insert(QLatin1String(COMMAND_KEYC), m_commandName);
    map.insert(QLatin1String(ARGUMENTS_KEYC), m_args);

    return map;
}

GoProject *GoRunConfiguration::project() const
{
    GoProject *pro = qobject_cast<GoProject*>(target()->project());
    Q_ASSERT_X(pro,Q_FUNC_INFO,"Go Project can not be NULL");
    return pro;
}

void GoRunConfiguration::setCommandName(const QString &cmdName)
{
    m_commandName = cmdName;
}

void GoRunConfiguration::setWorkingDirectory(const QString &dir)
{
    if( dir == workingDirectory() )
        return;

    m_userWorkingDirectory = dir;
    emit workingDirectoryChanged();
}

void GoRunConfiguration::setRunMode(ProjectExplorer::LocalApplicationRunConfiguration::RunMode mode)
{
    if( mode == m_runMode )
        return;

    m_runMode = mode;
}

void GoRunConfiguration::setCommandLineArguments(const QString &args)
{
    m_args = args;
}

} // namespace GoLang
