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

#include "xcodeprojectnode.h"
#include "pbxproj/pbenums.h"
#include "xcodeprojectmanagerconstants.h"

#include <coreplugin/icore.h>
#include <coreplugin/mimedatabase.h>
#include <projectexplorer/projectexplorerconstants.h>

#include <QFileInfo>
#include <QDebug> // FIXME: remove

using namespace ProjectExplorer;

namespace XCodeProjectManager {

XCodeProjectNode::XCodeProjectNode(const QString &projectFilePath)
    : ProjectExplorer::ProjectNode(projectFilePath)
    , m_projectFilePath(projectFilePath)
    , m_projectFileDir(QFileInfo(projectFilePath).dir())
{
    setPath(m_projectFileDir.absoluteFilePath(QLatin1String("..")));
}

bool XCodeProjectNode::hasBuildTargets() const
{
    return true;
}

QList<ProjectExplorer::ProjectNode::ProjectAction> XCodeProjectNode::supportedActions(ProjectExplorer::Node *node) const
{
    Q_UNUSED(node);
    // TODO: add actions
    return QList<ProjectNode::ProjectAction>();
}

bool XCodeProjectNode::canAddSubProject(const QString &proFilePath) const
{
    Q_UNUSED(proFilePath)
    return false;
}

bool XCodeProjectNode::addSubProjects(const QStringList &proFilePaths)
{
    Q_UNUSED(proFilePaths);
    return false;
}

bool XCodeProjectNode::removeSubProjects(const QStringList &proFilePaths)
{
    Q_UNUSED(proFilePaths);
    return false;
}

bool XCodeProjectNode::addFiles(const ProjectExplorer::FileType fileType, const QStringList &filePaths, QStringList *notAdded)
{
    Q_UNUSED(fileType);
    Q_UNUSED(filePaths);
    Q_UNUSED(notAdded);
    // TODO: obvious
    return false;
}

bool XCodeProjectNode::removeFiles(const ProjectExplorer::FileType fileType, const QStringList &filePaths, QStringList *notRemoved)
{
    Q_UNUSED(fileType);
    Q_UNUSED(filePaths);
    Q_UNUSED(notRemoved);
    // TODO: obvious
    return false;
}

bool XCodeProjectNode::deleteFiles(const ProjectExplorer::FileType fileType, const QStringList &filePaths)
{
    Q_UNUSED(fileType);
    Q_UNUSED(filePaths);
    // TODO: obvious
    return false;
}

bool XCodeProjectNode::renameFile(const ProjectExplorer::FileType fileType, const QString &filePath, const QString &newFilePath)
{
    Q_UNUSED(fileType);
    Q_UNUSED(filePath);
    Q_UNUSED(newFilePath);
    // TODO: obvious
    return false;
}

QList<ProjectExplorer::RunConfiguration *> XCodeProjectNode::runConfigurationsFor(ProjectExplorer::Node *node)
{
    Q_UNUSED(node);
    // TODO: what's this for
    return QList<ProjectExplorer::RunConfiguration *>();
}

bool sortToGroupByFolder(const QString &fileA, const QString &fileB)
{
    const int size = qMin(fileA.size(), fileB.size());
    for (int i = 0; i < size; ++i)
        if (fileA[i] != fileB[i]) {
            if (fileA[i] == QLatin1Char('/'))
                return false;
            if (fileB[i] == QLatin1Char('/'))
                return true;
            return fileA[i] < fileB[i];
        }
    return fileA.size() <= fileB.size();
}

void XCodeProjectNode::refresh(const PBProjectModel::Ptr &modelReference)
{
    qDebug() << "XCodeProjectNode::refresh";
    PBProjectModel::Ptr model(modelReference);
    removeFileNodes(fileNodes(), this);
    removeFolderNodes(subFolderNodes(), this);
    m_files.clear();
    m_objects = model->objects();

    addGroupChildren(model->rootGroup(), this);
}

QStringList XCodeProjectNode::files() const
{
    return m_files;
}

void XCodeProjectNode::addGroupChild(const PBValue &child, FolderNode *to)
{
    qDebug() << "XCodeProjectNode::addGroupChild: " << child.repr();
    const PBObjectClass clazz = pbObjectClass(child.valueForKey(QLatin1String("isa")).repr());
    if (clazz == PBXFileReference)
        addFileRef(child, to);
    else if (clazz == PBXGroup)
        addGroup(child, to);
}

void XCodeProjectNode::addGroupChildren(const PBValue &group, FolderNode *to)
{
    PBValue children = group.valueForKey(QLatin1String("children"));
    qDebug() << "XCodeProjectNode::addGroupChildren: " << children.repr();
    for (int i = 0, n = children.count(); i < n; ++i) {
        QString key = children.valueAtIndex(i).key();
        if (!key.isEmpty())
            addGroupChild(m_objects.valueForKey(key), to);
    }
}

void XCodeProjectNode::addFileRef(const PBValue &fileRef, FolderNode *to)
{
    qDebug() << "XCodeProjectNode::addFileRef";
    if (fileRef.isNull())
        return;
    const QDir dir(to->path());

    const QString varPath = fileRef.valueForKey(QLatin1String("path")).repr();
    qDebug() << "XCodeProjectNode::addFileRef varPath =" << varPath;
    QFileInfo finfo(dir.absoluteFilePath(varPath));
    if (finfo.exists()) {
        QList<FileNode *> files;
        files << createFileNode(finfo);
        addFileNodes(files, to);
    }
}

void XCodeProjectNode::addGroup(const PBValue &group, FolderNode *to)
{
    qDebug() << "XCodeProjectNode::addGroup";
    if (group.isNull())
        return;
    const QDir dir(to->path());

    const QString varPath = group.valueForKey(QLatin1String("path")).repr();
    const QString varName = group.valueForKey(QLatin1String("name")).repr();
    qDebug() << "XCodeProjectNode::addGroup varPath =" << varPath;
    FolderNode *node = new FolderNode(dir.absoluteFilePath(varPath));
    if (varName.isEmpty())
        node->setDisplayName(varName);
    else
        node->setDisplayName(varPath);

    addGroupChildren(group, node);

    QList<FolderNode *> list;
    list << node;
    addFolderNodes(list, to);
}

FileNode *XCodeProjectNode::createFileNode(const QFileInfo &fileInfo)
{
    FileType fileType = typeForFileName(Core::ICore::mimeDatabase(), fileInfo);
    return new FileNode(fileInfo.canonicalFilePath(), fileType, false);
}

FileType XCodeProjectNode::typeForFileName(const Core::MimeDatabase *db, const QFileInfo &file)
{
    const Core::MimeType mt = db->findByFile(file);
    if (!mt)
        return UnknownFileType;

    const QString typeName = mt.type();
    if (typeName == QLatin1String(Constants::PBXPROJ_MIMETYPE))
        return ProjectFileType;
    if (typeName == QLatin1String(ProjectExplorer::Constants::CPP_SOURCE_MIMETYPE)
        || typeName == QLatin1String(ProjectExplorer::Constants::C_SOURCE_MIMETYPE))
        return SourceType;
    if (typeName == QLatin1String(ProjectExplorer::Constants::CPP_HEADER_MIMETYPE)
        || typeName == QLatin1String(ProjectExplorer::Constants::C_HEADER_MIMETYPE))
        return HeaderType;
    if (typeName == QLatin1String(ProjectExplorer::Constants::RESOURCE_MIMETYPE))
        return ResourceType;
    if (typeName == QLatin1String(ProjectExplorer::Constants::FORM_MIMETYPE))
        return FormType;
    if (typeName == QLatin1String(ProjectExplorer::Constants::QML_MIMETYPE))
        return QMLType;
    return UnknownFileType;
}

} // namespace XCodeProjectManager
