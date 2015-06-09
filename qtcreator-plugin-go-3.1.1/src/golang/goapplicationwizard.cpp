#include "goapplicationwizard.h"
#include "gokitinformation.h"
#include "goproject.h"
#include "goprojectmanager.h"

#include <utils/qtcassert.h>
#include <utils/pathchooser.h>
#include <qtsupport/qtkitinformation.h>
#include <qtsupport/qtsupportconstants.h>
#include <projectexplorer/kitmanager.h>
#include <extensionsystem/pluginmanager.h>

#include <QDir>
#include <QScrollArea>
#include <QLabel>
#include <QVBoxLayout>
#include <QDebug>
#include <QLabel>
#include <QFormLayout>
namespace GoLang {

using namespace Internal;

GoApplicationWizard::GoApplicationWizard()
{
}

QWizard *GoApplicationWizard::createWizardDialog (QWidget *parent, const Core::WizardDialogParameters &wizardDialogParameters) const
{
    QTC_ASSERT(!parameters().isNull(), return 0);
    GoApplicationWizardDialog *projectDialog = new GoApplicationWizardDialog(parent, wizardDialogParameters);
    projectDialog->addTargetSetupPage(1);

    initProjectWizardDialog(projectDialog,
                            wizardDialogParameters.defaultPath(),
                            wizardDialogParameters.extensionPages());

    projectDialog->setIntroDescription(tr("This wizard generates a Go Application project."));
    return projectDialog;
}

bool GoApplicationWizard::postGenerateFiles(const QWizard *w, const Core::GeneratedFiles &l, QString *errorMessage)
{
    const GoApplicationWizardDialog *dialog = qobject_cast<const GoApplicationWizardDialog *>(w);

    // Generate user settings
    foreach (const Core::GeneratedFile &file, l)
        if (file.attributes() & Core::GeneratedFile::OpenProjectAttribute) {
            dialog->writeUserFile(file.path());
            break;
        }

    // Post-Generate: Open the projects/editors
    return ProjectExplorer::CustomProjectWizard::postGenerateOpen(l ,errorMessage);
}

GoApplicationWizardDialog::GoApplicationWizardDialog(QWidget *parent, const Core::WizardDialogParameters &parameters)
    : ProjectExplorer::BaseProjectWizardDialog(parent,parameters)
    , m_targetSetupPage(0)
{
    init();
}

GoApplicationWizardDialog::~GoApplicationWizardDialog()
{
    if (m_targetSetupPage && !m_targetSetupPage->parent())
        delete m_targetSetupPage;
}

bool GoApplicationWizardDialog::writeUserFile(const QString &projectFileName) const
{
    if (!m_targetSetupPage)
        return false;

    Internal::Manager *manager = ExtensionSystem::PluginManager::getObject<Internal::Manager>();
    Q_ASSERT(manager);

    GoProject *pro = new GoProject(manager, projectFileName);
    bool success = m_targetSetupPage->setupProject(pro);
    if (success)
        pro->saveSettings();
    delete pro;
    return success;
}


void GoApplicationWizardDialog::init()
{
    connect(this, SIGNAL(projectParametersChanged(QString,QString)),
            this, SLOT(generateProfileName(QString,QString)));
}

int GoApplicationWizardDialog::addTargetSetupPage(int id)
{
    m_targetSetupPage = new ProjectExplorer::TargetSetupPage;
    const QString platform = selectedPlatform();

    //prefer Qt Desktop or Platform Kit
    Core::FeatureSet features = Core::FeatureSet(QtSupport::Constants::FEATURE_DESKTOP);
    if (platform.isEmpty())
        m_targetSetupPage->setPreferredKitMatcher(new QtSupport::QtVersionKitMatcher(features));
    else
        m_targetSetupPage->setPreferredKitMatcher(new QtSupport::QtPlatformKitMatcher(platform));

    //make sure only Go compatible Kits are shown
    m_targetSetupPage->setRequiredKitMatcher(new GoKitMatcher);

    resize(900, 450);
    if (id >= 0)
        setPage(id, m_targetSetupPage);
    else
        id = addPage(m_targetSetupPage);

    wizardProgress()->item(id)->setTitle(tr("Kits"));
    return id;
}

QList<Core::Id> GoApplicationWizardDialog::selectedKits() const
{
    if(m_targetSetupPage)
        return m_targetSetupPage->selectedKits();

    return QList<Core::Id>();
}

void GoApplicationWizardDialog::generateProfileName(const QString &name, const QString &path)
{
    if (!m_targetSetupPage)
        return;

    const QString proFile =
            QDir::cleanPath(path + QLatin1Char('/') + name + QLatin1Char('/') + name + QLatin1String(".goproject"));

    m_targetSetupPage->setProjectPath(proFile);
}

} // namespace GoLang
