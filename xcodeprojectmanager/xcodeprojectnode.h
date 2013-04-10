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

#ifndef XCODEPROJECTMANAGER_XCODEPROJECTNODE_H
#define XCODEPROJECTMANAGER_XCODEPROJECTNODE_H

#include <projectexplorer/projectnodes.h>
#include "pbxproj/pbprojectmodel.h"

#include <QDir>

namespace XCodeProjectManager {

class XCodeProjectNode : public ProjectExplorer::ProjectNode
{
public:
    XCodeProjectNode(const QString &projectFilePath);

    bool hasBuildTargets() const;
    QList<ProjectAction> supportedActions(Node *node) const;
    bool canAddSubProject(const QString &proFilePath) const;
    bool addSubProjects(const QStringList &proFilePaths);
    bool removeSubProjects(const QStringList &proFilePaths);
    bool addFiles(const ProjectExplorer::FileType fileType,
                  const QStringList &filePaths,
                  QStringList *notAdded);
    bool removeFiles(const ProjectExplorer::FileType fileType,
                     const QStringList &filePaths,
                     QStringList *notRemoved);
    bool deleteFiles(const ProjectExplorer::FileType fileType,
                     const QStringList &filePaths);
    bool renameFile(const ProjectExplorer::FileType fileType,
                     const QString &filePath,
                     const QString &newFilePath);
    QList<ProjectExplorer::RunConfiguration *> runConfigurationsFor(Node *node);

    void refresh(const PBProjectModel::Ptr &model);
    QStringList files() const;

private:
    void addGroupChild(const PBValue &child, FolderNode *to);
    void addGroupChildren(const PBValue &group, FolderNode *to);
    void addFileRef(const PBValue &fileRef, ProjectExplorer::FolderNode *to);
    void addGroup(const PBValue &group, ProjectExplorer::FolderNode *to);

    static ProjectExplorer::FileNode *createFileNode(const QFileInfo &fileInfo);
    static ProjectExplorer::FileType typeForFileName(const Core::MimeDatabase *db, const QFileInfo &file);

    PBValue m_objects;
    QStringList m_files;
    QString m_projectFilePath;
    QDir m_projectFileDir;
};

} // namespace XCodeProjectManager

#endif // XCODEPROJECTMANAGER_XCODEPROJECTNODE_H
