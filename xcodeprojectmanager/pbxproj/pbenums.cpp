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

#include "pbenums.h"

namespace XCodeProjectManager {

PBObjectClass pbObjectClass(const QString &className)
{
    if (className == QLatin1String("PBXBuildFile"))
        return PBXBuildFile;
    if (className == QLatin1String("PBXFileReference"))
        return PBXFileReference;
    if (className == QLatin1String("PBXGroup"))
        return PBXGroup;
    if (className == QLatin1String("PBXSourcesBuildPhase"))
        return PBXSourcesBuildPhase;
    if (className == QLatin1String("PBXFrameworksBuildPhase"))
        return PBXFrameworksBuildPhase;
    if (className == QLatin1String("PBXHeadersBuildPhase"))
        return PBXHeadersBuildPhase;
    if (className == QLatin1String("PBXResourcesBuildPhase"))
        return PBXResourcesBuildPhase;
    if (className == QLatin1String("PBXCopyFilesBuildPhase"))
        return PBXCopyFilesBuildPhase;
    if (className == QLatin1String("PBXShellScriptBuildPhase"))
        return PBXShellScriptBuildPhase;
    if (className == QLatin1String("PBXNativeTarget"))
        return PBXNativeTarget;
    if (className == QLatin1String("XCConfigurationList"))
        return XCConfigurationList;
    if (className == QLatin1String("XCBuildConfiguration"))
        return XCBuildConfiguration;
    if (className == QLatin1String("PBXVariantGroup"))
        return PBXVariantGroup;
    if (className == QLatin1String("PBXProject"))
        return PBXProject;
    return PBUnknown;
}

QString pbObjectClassName(PBObjectClass objectClass)
{
    switch (objectClass) {
    case PBXBuildFile:
        return QLatin1String("PBXBuildFile");
    case PBXCopyFilesBuildPhase:
        return QLatin1String("PBXCopyFilesBuildPhase");
    case PBXFileReference:
        return QLatin1String("PBXFileReference");
    case PBXFrameworksBuildPhase:
        return QLatin1String("PBXFrameworksBuildPhase");
    case PBXGroup:
        return QLatin1String("PBXGroup");
    case PBXHeadersBuildPhase:
        return QLatin1String("PBXHeadersBuildPhase");
    case PBXNativeTarget:
        return QLatin1String("PBXNativeTarget");
    case PBXProject:
        return QLatin1String("PBXProject");
    case PBXResourcesBuildPhase:
        return QLatin1String("PBXResourcesBuildPhase");
    case PBXShellScriptBuildPhase:
        return QLatin1String("PBXShellScriptBuildPhase");
    case PBXSourcesBuildPhase:
        return QLatin1String("PBXSourcesBuildPhase");
    case PBXVariantGroup:
        return QLatin1String("PBXVariantGroup");
    case XCBuildConfiguration:
        return QLatin1String("XCBuildConfiguration");
    case XCConfigurationList:
        return QLatin1String("XCConfigurationList");
    default:
        return QLatin1String("PBUnknown");
    }
}

PBByClassIterator::PBByClassIterator(const PBValue &dictionary, const PBObjectClass &isaClass)
    : m_isaClass(pbObjectClassName(isaClass))
    , m_invalid(dictionary.asDict() == NULL)
    , m_isStarted(false)
{
    if (!m_invalid) {
        m_it = dictionary.asDict()->begin();
        m_end = dictionary.asDict()->end();
        m_isaKey.key = QLatin1String("isa");
    }
}

const PBKey &PBByClassIterator::objectKey() const
{
    return m_it.key();
}

const PBValue &PBByClassIterator::object() const
{
    return m_it.value();
}

bool PBByClassIterator::toNext()
{
    if (m_invalid)
        return false;

    if (!m_isStarted)
        ++m_it;
    m_isStarted = true;
    return skipUntilMatches();
}

bool PBByClassIterator::skipUntilMatches()
{
    while (m_it != m_end && m_it.value().valueForKey(m_isaKey).repr() != m_isaClass)
        ++m_it;
    return m_it != m_end;
}

}
