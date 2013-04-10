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

#include "xcodemanager.h"
#include "xcodeprojectmanagerconstants.h"
#include "xcodeproject.h"

#include <QFileInfo>
#include <QDir>

namespace XCodeProjectManager {

XCodeManager::XCodeManager()
//    : m_optionsPage(optionsPage)
{
}

QString XCodeManager::mimeType() const
{
    return QLatin1String(Constants::PBXPROJ_MIMETYPE);
}

ProjectExplorer::Project *XCodeManager::openProject(const QString &fileName, QString *errorString)
{
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();

    // Check whether the project file exists.
    if (canonicalFilePath.isEmpty()) {
        if (errorString)
            *errorString = tr("Failed opening project '%1': Project file does not exist")
                           .arg(QDir::toNativeSeparators(fileName));
        return 0;
    }

    return new XCodeProject(this, canonicalFilePath);
}

//XCodeProjectBuildOptionsPage *XCodeManager::buildOptionsPage()
//{
//    return m_optionsPage;
//}

} // namespace XCodeProjectManager
