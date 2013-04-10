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

#include "xcodeprojectfile.h"
#include "xcodeprojectmanagerconstants.h"
#include <QFileInfo>

namespace XCodeProjectManager {

XCodeProjectFile::XCodeProjectFile(const QString &filePath) :
    m_filePath(filePath),
    m_path(QFileInfo(filePath).path())
{
}

bool XCodeProjectFile::save(QString *errorString, const QString &fileName, bool autoSave)
{
    Q_UNUSED(errorString)
    Q_UNUSED(fileName)
    Q_UNUSED(autoSave)
    // TODO: obvious
    return false;
}

QString XCodeProjectFile::fileName() const
{
    return QFileInfo(m_filePath).fileName();
}

QString XCodeProjectFile::defaultPath() const
{
    // TODO: what's this for?
    return QString();
}

QString XCodeProjectFile::suggestedFileName() const
{
    // TODO: what's this for?
    return QString();
}

QString XCodeProjectFile::mimeType() const
{
    return QLatin1String(Constants::PBXPROJ_MIMETYPE);
}

bool XCodeProjectFile::isModified() const
{
    // TODO: obvious
    return false;
}

bool XCodeProjectFile::isSaveAsAllowed() const
{
    return false;
}

bool XCodeProjectFile::reload(QString *errorString, Core::IDocument::ReloadFlag flag, Core::IDocument::ChangeType type)
{
    Q_UNUSED(errorString);
    Q_UNUSED(flag);
    Q_UNUSED(type);

    // TODO: what's this for?
    return false;
}

void XCodeProjectFile::rename(const QString &newName)
{
    Q_UNUSED(newName);

    // TODO: obvious
}

} // namespace XCodeProjectManager
