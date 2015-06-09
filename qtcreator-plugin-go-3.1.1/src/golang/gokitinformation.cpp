#include "gokitinformation.h"
#include "toolchain.h"
#include "toolchainmanager.h"
#include "golangconstants.h"

#include <projectexplorer/kitmanager.h>
#include <projectexplorer/kitinformation.h>
#include <projectexplorer/toolchain.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <utils/qtcassert.h>
#include <coreplugin/icore.h>
#include <qtsupport/baseqtversion.h>
#include <qtsupport/qtkitinformation.h>

namespace GoLang {

bool GoKitMatcher::matches(const ProjectExplorer::Kit *k) const
{
    if (!k->isValid())
        return false;

    if (!GoToolChainKitInformation::toolChain(k))
        return false;

    ProjectExplorer::IDevice::ConstPtr dev = ProjectExplorer::DeviceKitInformation::device(k);
    if (dev.isNull() || dev->type() != ProjectExplorer::Constants::DESKTOP_DEVICE_TYPE)
        return false;

    return true;
}

// --------------------------------------------------------------------------
// ToolChainInformation:
// --------------------------------------------------------------------------

GoToolChainKitInformation::GoToolChainKitInformation()
{
    setObjectName(QLatin1String("GoToolChainInformation"));
    setId(GoToolChainKitInformation::id());
    setPriority(30000);

    connect(ProjectExplorer::KitManager::instance(), SIGNAL(kitsLoaded()),
            this, SLOT(kitsWereLoaded()));
}

QVariant GoToolChainKitInformation::defaultValue(ProjectExplorer::Kit *k) const
{
    Q_UNUSED(k);
    QList<ToolChain *> tcList = ToolChainManager::toolChains();
    if (tcList.isEmpty())
        return QString();

    ProjectExplorer::ToolChain *gccTc = ProjectExplorer::ToolChainKitInformation::toolChain(k);
    if(!gccTc)
        return QString();

    ProjectExplorer::Abi abi = gccTc->targetAbi();

    foreach (ToolChain *tc, tcList) {
        if (tc->targetAbi() == abi)
            return tc->id();
    }

    return QString();
}

QList<ProjectExplorer::Task> GoToolChainKitInformation::validate(const ProjectExplorer::Kit *k) const
{
    QList<ProjectExplorer::Task> result;

    const ToolChain* toolchain = toolChain(k);
    if (toolchain) {
        result << toolchain->validateKit(k);
    }
    return result;
}

void GoToolChainKitInformation::fix(ProjectExplorer::Kit *k)
{
    QTC_ASSERT(ToolChainManager::isLoaded(), return);
    if (toolChain(k))
        return;

    qWarning("No go tool chain set from kit \"%s\".",
             qPrintable(k->displayName()));
    QString defaultTc = defaultValue(k).toString();
    setToolChain(k, (!defaultTc.isEmpty()) ? ToolChainManager::findToolChain(defaultTc) : 0 ); // make sure to clear out no longer known tool chains
}

void GoToolChainKitInformation::setup(ProjectExplorer::Kit *k)
{
    QTC_ASSERT(ToolChainManager::isLoaded(), return);
    const QString id = k->value(GoToolChainKitInformation::id()).toString();
    if (id.isEmpty())
        return;

    ToolChain *tc = ToolChainManager::findToolChain(id);
    if (tc)
        return;

    // ID is not found: Might be an ABI string...
    foreach (ToolChain *current, ToolChainManager::toolChains()) {
        if (current->targetAbi().toString() == id)
            return setToolChain(k, current);
    }
}

ProjectExplorer::KitConfigWidget *GoToolChainKitInformation::createConfigWidget(ProjectExplorer::Kit *k) const
{
    return new Internal::ToolChainInformationConfigWidget(k, this);
}

QString GoToolChainKitInformation::displayNamePostfix(const ProjectExplorer::Kit *k) const
{
    ToolChain *tc = toolChain(k);
    return tc ? tc->displayName() : QString();
}

ProjectExplorer::KitInformation::ItemList GoToolChainKitInformation::toUserOutput(const ProjectExplorer::Kit *k) const
{
    ToolChain *tc = toolChain(k);
    return ItemList() << qMakePair(tr("Go - Compiler"), tc ? tc->displayName() : tr("None"));
}

void GoToolChainKitInformation::addToEnvironment(const ProjectExplorer::Kit *k, Utils::Environment &env) const
{
    ToolChain *tc = toolChain(k);
    if (tc)
        tc->addToEnvironment(env);
}

ProjectExplorer::IOutputParser *GoToolChainKitInformation::createOutputParser(const ProjectExplorer::Kit *k) const
{
    ToolChain *tc = toolChain(k);
    if (tc)
        return tc->outputParser();
    return 0;
}

Core::Id GoToolChainKitInformation::id()
{
    return "GoLang.Profile.GoToolChain";
}

ToolChain *GoToolChainKitInformation::toolChain(const ProjectExplorer::Kit *k)
{
    QTC_ASSERT(ToolChainManager::isLoaded(), return 0);
    if (!k)
        return 0;
    return ToolChainManager::findToolChain(k->value(GoToolChainKitInformation::id()).toString());
}

void GoToolChainKitInformation::setToolChain(ProjectExplorer::Kit *k, ToolChain *tc)
{
    k->setValue(GoToolChainKitInformation::id(), tc ? tc->id() : QString());
}

void GoToolChainKitInformation::kitsWereLoaded()
{
    foreach (ProjectExplorer::Kit *k, ProjectExplorer::KitManager::kits())
        fix(k);

    connect(ToolChainManager::instance(), SIGNAL(toolChainRemoved(GoLang::ToolChain*)),
            this, SLOT(toolChainRemoved(GoLang::ToolChain*)));
    connect(ToolChainManager::instance(), SIGNAL(toolChainUpdated(GoLang::ToolChain*)),
            this, SLOT(toolChainUpdated(GoLang::ToolChain*)));
}

void GoToolChainKitInformation::toolChainUpdated(GoLang::ToolChain *tc)
{
    //foreach (ProjectExplorer::Kit *k, ProjectExplorer::KitManager::matchingKits(ToolChainMatcher(tc)))
    //    notifyAboutUpdate(k);
}

void GoToolChainKitInformation::toolChainRemoved(GoLang::ToolChain *tc)
{
    Q_UNUSED(tc);
    foreach (ProjectExplorer::Kit *k, ProjectExplorer::KitManager::kits())
        fix(k);
}

namespace Internal {
// --------------------------------------------------------------------------
// ToolChainInformationConfigWidget:
// --------------------------------------------------------------------------

ToolChainInformationConfigWidget::ToolChainInformationConfigWidget(ProjectExplorer::Kit *k, const ProjectExplorer::KitInformation *ki) :
    KitConfigWidget(k, ki)
{
    m_comboBox = new QComboBox;
    m_comboBox->setEnabled(false);
    m_comboBox->setToolTip(toolTip());

    foreach (ToolChain *tc, ToolChainManager::toolChains())
        toolChainAdded(tc);

    updateComboBox();

    refresh();
    connect(m_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(currentToolChainChanged(int)));

    m_manageButton = new QPushButton(tr("Manage..."));
    m_manageButton->setContentsMargins(0, 0, 0, 0);
    connect(m_manageButton, SIGNAL(clicked()), this, SLOT(manageToolChains()));

    QObject *tcm = ToolChainManager::instance();
    connect(tcm, SIGNAL(toolChainAdded(GoLang::ToolChain*)),
            this, SLOT(toolChainAdded(GoLang::ToolChain*)));
    connect(tcm, SIGNAL(toolChainRemoved(GoLang::ToolChain*)),
            this, SLOT(toolChainRemoved(GoLang::ToolChain*)));
    connect(tcm, SIGNAL(toolChainUpdated(GoLang::ToolChain*)),
            this, SLOT(toolChainUpdated(GoLang::ToolChain*)));
}

ToolChainInformationConfigWidget::~ToolChainInformationConfigWidget()
{
    delete m_comboBox;
    delete m_manageButton;
}

QString ToolChainInformationConfigWidget::displayName() const
{
    return tr("Compiler:");
}

QString ToolChainInformationConfigWidget::toolTip() const
{
    return tr("The compiler to use for building go projects.<br>"
              "Make sure the compiler will produce binaries compatible with the target device, "
              "Qt version and other libraries used.");
}

void ToolChainInformationConfigWidget::refresh()
{
    m_comboBox->setCurrentIndex(indexOf(GoToolChainKitInformation::toolChain(m_kit)));
}

void ToolChainInformationConfigWidget::makeReadOnly()
{
    m_comboBox->setEnabled(false);
}

QWidget *ToolChainInformationConfigWidget::mainWidget() const
{
    return m_comboBox;
}

QWidget *ToolChainInformationConfigWidget::buttonWidget() const
{
    return m_manageButton;
}

void ToolChainInformationConfigWidget::toolChainAdded(GoLang::ToolChain *tc)
{
    m_comboBox->addItem(tc->displayName(), tc->id());
    updateComboBox();
}

void ToolChainInformationConfigWidget::toolChainRemoved(GoLang::ToolChain *tc)
{
    const int pos = indexOf(tc);
    if (pos < 0)
        return;
    m_comboBox->removeItem(pos);
    updateComboBox();
}
void ToolChainInformationConfigWidget::toolChainUpdated(GoLang::ToolChain *tc)
{
    const int pos = indexOf(tc);
    if (pos < 0)
        return;
    m_comboBox->setItemText(pos, tc->displayName());
}

void ToolChainInformationConfigWidget::manageToolChains()
{
    Core::ICore::showOptionsDialog(ProjectExplorer::Constants::PROJECTEXPLORER_SETTINGS_CATEGORY,
                                   Constants::TOOLCHAIN_SETTINGS_PAGE_ID);
}

void ToolChainInformationConfigWidget::currentToolChainChanged(int idx)
{
    const QString id = m_comboBox->itemData(idx).toString();
    GoToolChainKitInformation::setToolChain(m_kit, ToolChainManager::findToolChain(id));
}

void ToolChainInformationConfigWidget::updateComboBox()
{
    // remove unavailable tool chain:
    int pos = indexOf(0);
    if (pos >= 0)
        m_comboBox->removeItem(pos);

    if (m_comboBox->count() == 0) {
        m_comboBox->addItem(tr("<No compiler available>"), QString());
        m_comboBox->setEnabled(false);
    } else {
        m_comboBox->setEnabled(true);
    }
}

int ToolChainInformationConfigWidget::indexOf(const GoLang::ToolChain *tc)
{
    const QString id = tc ? tc->id() : QString();
    for (int i = 0; i < m_comboBox->count(); ++i) {
        if (id == m_comboBox->itemData(i).toString())
            return i;
    }
    return -1;
}

} // namespace Internal
} // namespace GoLang
