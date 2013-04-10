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

#ifndef XCODEPROJECTMANAGER_PBENUMS_H
#define XCODEPROJECTMANAGER_PBENUMS_H

#include "pbclasses.h"

namespace XCodeProjectManager {

enum PBObjectClass {
    PBUnknown = 0,
    PBXGroup,
    PBXBuildFile,
    PBXFileReference,
    PBXSourcesBuildPhase,
    PBXFrameworksBuildPhase,
    PBXHeadersBuildPhase,
    PBXResourcesBuildPhase,
    PBXNativeTarget,
    XCConfigurationList,
    XCBuildConfiguration,
    PBXVariantGroup,
    PBXShellScriptBuildPhase,
    PBXCopyFilesBuildPhase,
    PBXProject
};

PBObjectClass pbObjectClass(const QString &className);
QString pbObjectClassName(PBObjectClass objectClass);

/// @brief Iterates \a PBDictionary entries matching by 'isa' field
/// Dictionary should exists while iterating
class PBByClassIterator
{
public:
    PBByClassIterator(const PBValue &dictionary, const PBObjectClass &isaClass);

    /// @return PBKey pointer
    const PBKey &objectKey() const;

    /// @return PBDictionary pointer
    const PBValue &object() const;

    bool toNext();

private:
    bool skipUntilMatches();

    PBKey m_isaKey;
    QString m_isaClass;

    bool m_invalid;
    bool m_isStarted;

    PBDictionary::iterator m_it;
    PBDictionary::iterator m_end;
};

}

#endif // XCODEPROJECTMANAGER_PBENUMS_H
