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

#include "goprojectnode.h"
#include "goprojectitem.h"
#include "filefilteritems.h"

using namespace GoLang::Internal;

GoProjectNode::GoProjectNode(GoProject *project, Core::IDocument *projectFile)
    : ProjectExplorer::ProjectNode(QFileInfo(projectFile->filePath()).absoluteFilePath()),
      m_project(project),
      m_projectFile(projectFile) {
    setDisplayName(QFileInfo(projectFile->filePath()).completeBaseName());
    refresh();
}

Core::IDocument *GoProjectNode::projectFile() const {
    return m_projectFile;
}

QString GoProjectNode::projectFilePath() const {
    return m_projectFile->filePath();
}

void GoProjectNode::refresh() {
    using namespace ProjectExplorer;

    this->removeFileNodes(fileNodes());
    this->removeFolderNodes(subFolderNodes());

    QPointer<GoProjectItem> proItem = m_project->m_projectItem;
    if(proItem) {

        FileNode *projectFilesNode = new FileNode(m_project->filesFileName(),
                                                  ProjectFileType,
                                                  /* generated = */ false);
        this->addFileNodes(QList<FileNode *>()
                           << projectFilesNode);

        foreach(GoBaseTargetItem* item, proItem->commands()) {
            QStringList files = item->files();
            files.removeAll(m_project->filesFileName());

            QHash<QString, QStringList> filesInDirectory;

            foreach (const QString &fileName, files) {
                QFileInfo fileInfo(fileName);

                QString absoluteFilePath;
                QString relativeDirectory;

                if (fileInfo.isAbsolute()) {
                    absoluteFilePath = fileInfo.filePath();
                    relativeDirectory = m_project->projectDir().relativeFilePath(fileInfo.path());
                } else {
                    absoluteFilePath = m_project->projectDir().absoluteFilePath(fileInfo.filePath());
                    relativeDirectory = fileInfo.path();
                    if (relativeDirectory == QLatin1String("."))
                        relativeDirectory.clear();
                }

                filesInDirectory[relativeDirectory].append(absoluteFilePath);
            }

            VirtualFolderNode* vFolder = new VirtualFolderNode(item->name(),1);
            this->addFolderNodes(QList<FolderNode*>() << vFolder);

            const QHash<QString, QStringList>::ConstIterator cend = filesInDirectory.constEnd();
            for (QHash<QString, QStringList>::ConstIterator it = filesInDirectory.constBegin(); it != cend; ++it) {
                FolderNode *folder = findOrCreateFolderByName(it.key(),vFolder);

                QList<FileNode *> fileNodes;
                foreach (const QString &file, it.value()) {
                    FileType fileType = SourceType; // ### FIXME
                    FileNode *fileNode = new FileNode(file, fileType, false);
                    fileNodes.append(fileNode);
                }

                folder->addFileNodes(fileNodes);
            }
            m_folderByName.clear();
        }
    }

    //QStringList files = m_project->files(Project::AllFiles);
    //files.removeAll(m_project->filesFileName());
    m_folderByName.clear();
}

ProjectExplorer::FolderNode *GoProjectNode::findOrCreateFolderByName(const QStringList &components, int end, ProjectExplorer::VirtualFolderNode *virtualRoot) {
    if (! end)
        return 0;

    QString baseDir = QFileInfo(path()).path();

    QString folderName;
    for (int i = 0; i < end; ++i) {
        folderName.append(components.at(i));
        folderName += QLatin1Char('/');
    }

    const QString component = components.at(end - 1);

    if (component.isEmpty())
        return virtualRoot;

    else if (FolderNode *folder = m_folderByName.value(folderName))
        return folder;

    FolderNode *folder = new FolderNode(baseDir + QLatin1Char('/') + folderName);
    folder->setDisplayName(component);

    m_folderByName.insert(folderName, folder);

    FolderNode *parent = findOrCreateFolderByName(components, end - 1, virtualRoot);
    if (! parent)
        parent = virtualRoot;

    parent->addFolderNodes(QList<FolderNode*>() << folder);

    return folder;
}

ProjectExplorer::FolderNode *GoProjectNode::findOrCreateFolderByName(const QString &filePath, ProjectExplorer::VirtualFolderNode *parent) {
    QStringList components = filePath.split(QLatin1Char('/'));
    return findOrCreateFolderByName(components, components.length(),parent);
}

QList<ProjectExplorer::ProjectAction> GoProjectNode::supportedActions(Node *node) const {
    Q_UNUSED(node);
    QList<ProjectExplorer::ProjectAction> actions;
    actions.append(ProjectExplorer::AddNewFile);
    actions.append(ProjectExplorer::EraseFile);
    actions.append(ProjectExplorer::Rename);
    return actions;
}

bool GoProjectNode::canAddSubProject(const QString &proFilePath) const {
    Q_UNUSED(proFilePath)
    return false;
}

bool GoProjectNode::addSubProjects(const QStringList &proFilePaths) {
    Q_UNUSED(proFilePaths)
    return false;
}

bool GoProjectNode::removeSubProjects(const QStringList &proFilePaths) {
    Q_UNUSED(proFilePaths)
    return false;
}

bool GoProjectNode::addFiles(const QStringList &, QStringList *) {
    return false;
}

bool GoProjectNode::removeFiles(const QStringList &, QStringList *) {
    return false;
}

bool GoProjectNode::deleteFiles(const QStringList &) {
    return true;
}

bool GoProjectNode::renameFile(const QString &, const QString &) {
    return true;
}

QList<ProjectExplorer::RunConfiguration *> GoProjectNode::runConfigurationsFor(Node *node) {
    Q_UNUSED(node)
    return QList<ProjectExplorer::RunConfiguration *>();
}
