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

#ifndef GOLANGPROJECT_H
#define GOLANGPROJECT_H

#include "goprojectmanager.h"
#include "goprojectfile.h"
#include "goprojectnode.h"
#include "golangconstants.h"
#include "goprojectitem.h"

#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/icontext.h>
#include <coreplugin/mimedatabase.h>
#include <coreplugin/messagemanager.h>
#include <coreplugin/icore.h>
#include <coreplugin/idocument.h>
#include <coreplugin/documentmanager.h>
#include <projectexplorer/iprojectmanager.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/projectnodes.h>
#include <projectexplorer/project.h>
#include <projectexplorer/kitmanager.h>
#include <projectexplorer/target.h>
#include <projectexplorer/session.h>
#include <projectexplorer/runconfiguration.h>
#include <projectexplorer/applicationlauncher.h>
#include <qmljs/qmljsmodelmanagerinterface.h>

#include <QFlags>

namespace GoLang {

class GoProjectItem;

namespace Internal {
class Manager;
class GoProjectFile;
class GoProjectNode;
} // namespace Internal

class GoProject : public ProjectExplorer::Project
{
    Q_OBJECT

    friend class Internal::GoProjectNode;

public:
    GoProject(Internal::Manager *manager, const QString &filename);
    virtual ~GoProject();

    QString filesFileName() const;

    QString displayName() const override;
    Core::IDocument *document() const override;
    ProjectExplorer::IProjectManager *projectManager() const override;

    bool supportsKit(ProjectExplorer::Kit *k, QString *errorMessage) const override;

    ProjectExplorer::ProjectNode *rootProjectNode() const override;
    QStringList files(FilesMode fileMode) const override;

    Q_INVOKABLE QString applicationNames () const;

    bool validProjectFile() const;

    enum RefreshOption {
        ProjectFile   = 0x01,
        Files         = 0x02,
        Configuration = 0x04,
        Everything    = ProjectFile | Files | Configuration
    };
    Q_DECLARE_FLAGS(RefreshOptions,RefreshOption)

    void refresh(RefreshOptions options);

    QDir projectDir() const;
    QStringList files() const;
    QStringList customImportPaths() const;

    bool addFiles(const QStringList &filePaths);

    void refreshProjectFile();

    enum QmlImport { UnknownImport, QtQuick1Import, QtQuick2Import };
    QmlImport defaultImport() const;

    QList<GoBaseTargetItem *> buildTargets() const;

    // Project interface
    virtual bool supportsNoTargetPanel() const;
    virtual ProjectExplorer::KitMatcher *createRequiredKitMatcher() const;
    virtual ProjectExplorer::KitMatcher *createPreferredKitMatcher() const;
    virtual bool needsConfiguration() const;



private slots:
    void refreshFiles(const QSet<QString> &added, const QSet<QString> &removed);
    void addedTarget(ProjectExplorer::Target *target);
    void onActiveTargetChanged(ProjectExplorer::Target *target);
    void onKitChanged();
    void addedRunConfiguration(ProjectExplorer::RunConfiguration *);

protected:
    bool fromMap(const QVariantMap &map);

    void updateConfigurations(ProjectExplorer::Target *t);
    void updateConfigurations();

    // Project interface
    virtual bool setupTarget(ProjectExplorer::Target *t) override;
private:
    // plain format
    void parseProject(RefreshOptions options);

    Internal::Manager *m_manager;
    QString m_fileName;
    Internal::GoProjectFile *m_file;
    QString m_projectName;
    QmlImport m_defaultImport;
    QmlJS::ModelManagerInterface *m_modelManager;
    ProjectExplorer::Target *m_activeTarget;

    // plain format
    QStringList m_files;

    QPointer<GoProjectItem> m_projectItem;

    Internal::GoProjectNode *m_rootNode;
};

}

#endif // GOLANGPROJECT_H
