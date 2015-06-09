/*
 * Copyright 2013 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 2.1.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Juhapekka Piiroinen <juhapekka.piiroinen@canonical.com>
 */

#include "goproject.h"
#include "goprojectfileformat.h"
#include "goprojectmanager.h"
#include "gokitinformation.h"
#include "gotoolchain.h"

#include <coreplugin/icore.h>
#include <coreplugin/messagemanager.h>
#include <coreplugin/documentmanager.h>
#include <qtsupport/baseqtversion.h>
#include <qtsupport/qtkitinformation.h>
#include <qmljstools/qmljsmodelmanager.h>
#include <projectexplorer/kitinformation.h>
#include <projectexplorer/kitmanager.h>
#include <projectexplorer/target.h>
#include <qtsupport/qtsupportconstants.h>
#include <qtsupport/customexecutablerunconfiguration.h>


namespace GoLang {

using namespace Core;
using namespace ProjectExplorer;

GoProject::GoProject(Internal::Manager *manager, const QString &fileName)
    : m_manager(manager),
      m_fileName(fileName),
      m_defaultImport(UnknownImport),
      m_modelManager(QmlJS::ModelManagerInterface::instance()),
      m_activeTarget(0)
{
    setId(Constants::GO_PROJECT_ID);
    setProjectContext(Context(Constants::GO_PROJECT_PROJECTCONTEXT));
    setProjectLanguages(Context(Constants::LANG_GO));

    QFileInfo fileInfo(m_fileName);
    m_projectName = fileInfo.completeBaseName();

    m_file = new Internal::GoProjectFile(this, fileName);
    m_rootNode = new Internal::GoProjectNode(this, m_file);

    DocumentManager::addDocument(m_file, true);

    m_manager->registerProject(this);
}

GoProject::~GoProject()
{
    m_manager->unregisterProject(this);

    DocumentManager::removeDocument(m_file);

    delete m_projectItem.data();
    delete m_rootNode;
}

void GoProject::addedTarget(ProjectExplorer::Target *target)
{
    connect(target, SIGNAL(addedRunConfiguration(ProjectExplorer::RunConfiguration*)),
            this, SLOT(addedRunConfiguration(ProjectExplorer::RunConfiguration*)));
    foreach (ProjectExplorer::RunConfiguration *rc, target->runConfigurations())
        addedRunConfiguration(rc);
}

void GoProject::onActiveTargetChanged(ProjectExplorer::Target *target)
{
    if (m_activeTarget)
        disconnect(m_activeTarget, SIGNAL(kitChanged()), this, SLOT(onKitChanged()));
    m_activeTarget = target;
    if (m_activeTarget)
        connect(target, SIGNAL(kitChanged()), this, SLOT(onKitChanged()));

    // make sure e.g. the default qml imports are adapted
    refresh(Configuration);
}

void GoProject::onKitChanged()
{
    // make sure e.g. the default qml imports are adapted
    refresh(Configuration);
}

void GoProject::addedRunConfiguration(ProjectExplorer::RunConfiguration *rc)
{
    Q_UNUSED(rc)
    // The enabled state of qml runconfigurations can only be decided after
    // they have been added to a project
    //QmlProjectRunConfiguration *qmlrc = qobject_cast<QmlProjectRunConfiguration *>(rc);
    //if (qmlrc)
    //    qmlrc->updateEnabled();
}

QDir GoProject::projectDir() const
{
    return QFileInfo(projectFilePath()).dir();
}

QString GoProject::filesFileName() const
{ return m_fileName; }

static GoProject::QmlImport detectImport(const QString &qml) {
    static QRegExp qtQuick1RegExp(QLatin1String("import\\s+QtQuick\\s+1"));
    static QRegExp qtQuick2RegExp(QLatin1String("import\\s+QtQuick\\s+2"));

    if (qml.contains(qtQuick1RegExp))
        return GoProject::QtQuick1Import;
    else if (qml.contains(qtQuick2RegExp))
        return GoProject::QtQuick2Import;
    else
        return GoProject::UnknownImport;
}

void GoProject::parseProject(RefreshOptions options)
{
    if (options & Files) {
        if (options & ProjectFile)
            delete m_projectItem.data();

        if (!m_projectItem) {
              QString errorMessage;
              m_projectItem = GoProjectFileFormat::parseProjectFile(m_fileName, &errorMessage);
              if (m_projectItem) {
                  connect(m_projectItem.data(), SIGNAL(filesChanged(QSet<QString>,QSet<QString>)),
                          this, SLOT(refreshFiles(QSet<QString>,QSet<QString>)));

              } else {
                  MessageManager::write(tr("Error while loading project file %1.").arg(m_fileName), MessageManager::NoModeSwitch);
                  MessageManager::write(errorMessage);
              }
        }
        if (m_projectItem) {
            m_projectItem.data()->setSourceDirectory(projectDir().path());
            m_modelManager->updateSourceFiles(m_projectItem.data()->files(), true);

            /*
            QString mainFilePath = m_projectItem.data()->mainFile();
            if (!mainFilePath.isEmpty()) {
                mainFilePath = projectDir().absoluteFilePath(mainFilePath);
                Utils::FileReader reader;
                QString errorMessage;
                if (!reader.fetch(mainFilePath, &errorMessage)) {
                    MessageManager::write(tr("Warning while loading project file %1.").arg(m_fileName));
                    MessageManager::write(errorMessage);
                } else {
                    m_defaultImport = detectImport(QString::fromUtf8(reader.data()));
                }
            }
            */
        }
        m_rootNode->refresh();
        updateConfigurations();
    }

    if (options & Configuration) {
        // update configuration
    }

    if (options & Files)
        emit fileListChanged();
}

void GoProject::refresh(RefreshOptions options)
{
    parseProject(options);

    if (options & Files)
        m_rootNode->refresh();

    QmlJS::ModelManagerInterface::ProjectInfo projectInfo =
            QmlJSTools::defaultProjectInfoForProject(this);
    projectInfo.importPaths = customImportPaths();

    m_modelManager->updateProjectInfo(projectInfo,this);
}

void GoProject::updateConfigurations()
{
    foreach (Target *t, targets())
        updateConfigurations(t);
}

bool GoProject::setupTarget(Target *t)
{
    t->updateDefaultBuildConfigurations();
    t->updateDefaultDeployConfigurations();
    t->updateDefaultRunConfigurations();
    return true;
}

void GoProject::updateConfigurations(Target *t)
{
    //t->updateDefaultDeployConfigurations();
    t->updateDefaultRunConfigurations();

    if (t->runConfigurations().isEmpty()) {
        // Oh no, no run configuration,
        // create a custom executable run configuration
        t->addRunConfiguration(new QtSupport::CustomExecutableRunConfiguration(t));
    }
}

QStringList GoProject::files() const
{
    QStringList files;
    if (m_projectItem)
        files = m_projectItem.data()->files();
    else
        files = m_files;
    return files;
}

bool GoProject::validProjectFile() const
{
    return !m_projectItem.isNull();
}

QStringList GoProject::customImportPaths() const
{
    QStringList importPaths;
    //if (m_projectItem)
    //    importPaths = m_projectItem.data()->importPaths();

    return importPaths;
}

bool GoProject::addFiles(const QStringList &filePaths)
{
    QStringList toAdd;
    foreach (const QString &filePath, filePaths) {
        if (!m_projectItem.data()->matchesFile(filePath))
            toAdd << filePaths;
    }
    return toAdd.isEmpty();
}

void GoProject::refreshProjectFile()
{
    RefreshOptions opts(GoProject::ProjectFile | GoProject::Files);
    refresh(opts);
}

GoProject::QmlImport GoProject::defaultImport() const
{
    return m_defaultImport;
}

QList<GoBaseTargetItem *> GoProject::buildTargets() const
{
    if(!m_projectItem)
        return QList<GoBaseTargetItem*>();

    return m_projectItem->commands();
}

bool GoProject::supportsNoTargetPanel() const
{
    return true;
}

KitMatcher *GoProject::createRequiredKitMatcher() const
{
    return new GoKitMatcher();
}

KitMatcher *GoProject::createPreferredKitMatcher() const
{
    Core::FeatureSet features = Core::FeatureSet(QtSupport::Constants::FEATURE_DESKTOP);
    return new QtSupport::QtVersionKitMatcher(features);
}

bool GoProject::needsConfiguration() const
{
    return targets().isEmpty();
}

void GoProject::refreshFiles(const QSet<QString> &/*added*/, const QSet<QString> &removed)
{
    refresh(Files);
    if (!removed.isEmpty())
        m_modelManager->removeFiles(removed.toList());
}

QString GoProject::displayName() const
{
    return m_projectName;
}

IDocument *GoProject::document() const
{
    return m_file;
}

ProjectExplorer::IProjectManager *GoProject::projectManager() const
{
    return m_manager;
}

bool GoProject::supportsKit(ProjectExplorer::Kit *k, QString *errorMessage) const
{
    Id deviceType = ProjectExplorer::DeviceTypeKitInformation::deviceTypeId(k);
    if (deviceType != ProjectExplorer::Constants::DESKTOP_DEVICE_TYPE) {
        if (errorMessage)
            *errorMessage = tr("Device type is not desktop.");
        return false;
    }

    QtSupport::BaseQtVersion *version = QtSupport::QtKitInformation::qtVersion(k);
    if (!version) {
        if (errorMessage)
            *errorMessage = tr("No Qt version set in kit.");
        return false;
    }

#if 0
    if (version->qtVersion() < QtSupport::QtVersionNumber(4, 7, 0)) {
        if (errorMessage)
            *errorMessage = tr("Qt version is too old.");
        return false;
    }

    if (version->qtVersion() < QtSupport::QtVersionNumber(5, 0, 0)
            && defaultImport() == QtQuick2Import) {
        if (errorMessage)
            *errorMessage = tr("Qt version is too old.");
        return false;
    }
#endif

    if (!GoToolChainKitInformation::toolChain(k)) {
        if (errorMessage)
            *errorMessage = tr("No Go toolchain is set in the Kit.");
        return false;
    }
    return true;
}

ProjectExplorer::ProjectNode *GoProject::rootProjectNode() const
{
    return m_rootNode;
}

QStringList GoProject::files(FilesMode) const
{
    return files();
}

QString GoProject::applicationNames() const
{
    QStringList apps;
    foreach(const GoBaseTargetItem* t,buildTargets()) {
        if(qobject_cast<const GoApplicationItem*>(t))
            apps.append(t->name());
    }
    return apps.join(QStringLiteral(" "));
}

bool GoProject::fromMap(const QVariantMap &map)
{
    if (!Project::fromMap(map))
        return false;

    // refresh first - project information is used e.g. to decide the default RC's
    refresh(Everything);

    // addedTarget calls updateEnabled on the runconfigurations
    // which needs to happen after refresh
    foreach (Target *t, targets())
        addedTarget(t);

    connect(this, SIGNAL(addedTarget(ProjectExplorer::Target*)),
            this, SLOT(addedTarget(ProjectExplorer::Target*)));

    connect(this, SIGNAL(activeTargetChanged(ProjectExplorer::Target*)),
            this, SLOT(onActiveTargetChanged(ProjectExplorer::Target*)));

    // make sure we get updates on kit changes
    m_activeTarget = activeTarget();
    if (m_activeTarget)
        connect(m_activeTarget, SIGNAL(kitChanged()), this, SLOT(onKitChanged()));

    return true;
}

}
