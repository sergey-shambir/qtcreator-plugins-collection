/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "xcodeproject.h"
#include "xcodeprojectmanagerconstants.h"
#include "xcodeprojectnode.h"
#include "xcodemanager.h"
#include "xcodeprojectfile.h"
#include "pbxproj/pbparser.h"

#include <coreplugin/icontext.h>
#include <coreplugin/icore.h>
#include <projectexplorer/projectexplorerconstants.h>

#include <QtConcurrentRun>
#include <QFutureWatcher>

#include <QFileSystemWatcher>
#include <QFuture>

namespace XCodeProjectManager {

class PrivateXCodeProject
{
public:
    XCodeManager *m_projectManager;
    QString m_projectFilePath;
    XCodeProjectNode *m_rootNode;
    QFileSystemWatcher *m_projectFileWatcher;
    PBProjectModel::Ptr m_model;
    QFutureWatcher<PBProjectModel::Ptr> m_modelReloadWatcher;
    QScopedPointer<XCodeProjectFile> m_projectFile;
};

XCodeProject::XCodeProject(XCodeManager *projectManager, const QString &projectFilePath) :
    d(new PrivateXCodeProject())
{
    d->m_projectManager = projectManager;
    d->m_projectFilePath = projectFilePath;
    d->m_projectFileWatcher = new QFileSystemWatcher(this);
    d->m_rootNode = new XCodeProjectNode(projectFilePath);
    d->m_projectFile.reset(new XCodeProjectFile(projectFilePath));

    setProjectContext(Core::Context(Constants::XCODE_PROJECT_CONTEXT));
    setProjectLanguages(Core::Context(ProjectExplorer::Constants::LANG_CXX));

    connect(&d->m_modelReloadWatcher, SIGNAL(finished()), this, SLOT(parsingFinished()));
    startReparsing();

    d->m_projectFileWatcher->addPath(projectFilePath);
    connect(d->m_projectFileWatcher, SIGNAL(fileChanged(QString)), this, SLOT(startReparsing()));
}

XCodeProject::~XCodeProject()
{
    delete d;
}

QString XCodeProject::displayName() const
{
    return d->m_rootNode->displayName();
}

Core::Id XCodeProject::id() const
{
    return Core::Id(Constants::XCODE_PROJECT_ID);
}

Core::IDocument *XCodeProject::document() const
{
    return d->m_projectFile.data();
}

ProjectExplorer::IProjectManager *XCodeProject::projectManager() const
{
    return d->m_projectManager;
}

ProjectExplorer::ProjectNode *XCodeProject::rootProjectNode() const
{
    return d->m_rootNode;
}

QStringList XCodeProject::files(ProjectExplorer::Project::FilesMode fileMode) const
{
    Q_UNUSED(fileMode);
    // TODO: respect the mode
    return d->m_rootNode->files();
}

void XCodeProject::startReparsing()
{
    if (!d->m_modelReloadWatcher.isFinished()) {
        d->m_modelReloadWatcher.cancel();
        d->m_modelReloadWatcher.waitForFinished();
    }

    d->m_modelReloadWatcher.setFuture(
                QtConcurrent::run(loadXCodeProjectModel, d->m_projectFilePath));
}

void XCodeProject::parsingFinished()
{
    PBProjectModel::Ptr replacement = d->m_modelReloadWatcher.result();
    if (replacement.isNull())
        return;
    d->m_model = replacement;

    d->m_rootNode->refresh(d->m_model);
    updateCodeModel();

    // TODO: can we (and is is important to) detect when the list really changed?
    emit fileListChanged();
}

/**
 * @brief Update editor Code Models
 *
 * Because only language with Code Model in QtCreator and support in XCode is C++/ObjC,
 * this method updates C++/ObjC code model.
 * XCode doesn't support Qt.
 */
void XCodeProject::updateCodeModel()
{
}

} // namespace XCodeProjectManager
