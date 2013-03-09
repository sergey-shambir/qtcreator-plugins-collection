/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
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

#ifndef INCLUDETRACKER_H
#define INCLUDETRACKER_H

#include "clang_global.h"

#include <QtCore/QStringList>
#include <QtCore/QHash>

namespace ClangCodeModel {
namespace Internal {

class CLANG_EXPORT IncludeTracker
{
public:
    IncludeTracker();

    enum ResolutionMode
    {
        // This is the usual form: Given the search order, whenever we find an include path
        // that matches a particular include spelling, we resolve to this one.
        FirstMatchResolution,

        // In this mode we gather every possible match of a particular include spelling
        // for the available include paths. Ideally this should not exist actually, but this
        // is particularly useful for computing the dependency graph used to compare against
        // the units processed by clang. Since we don't (yet) reliably reproduce clang's search
        // order, we have more chances of identifying the "right" file when restoring the index.
        EveryMatchResolution
    };

    void setResolutionMode(ResolutionMode mode);
    ResolutionMode resolutionMode() const;

    QStringList directIncludes(const QString &fileName,
                               const QStringList &compilationOptions) const;

private:
    struct OptionsCache
    {
        QStringList m_searchPaths;
    };

    typedef QHash<QStringList, OptionsCache> CacheSet;
    typedef CacheSet::iterator CacheSetIt;

    enum IncludeKind
    {
        QuotedInclude,
        AngleBracketInclude
    };

    QStringList parseIncludes(const QString &contents,
                              const QString &basePath,
                              CacheSetIt cacheIt) const;
    QSet<QString> resolveInclude(const QString &basePath,
                                 const QString &includeSpelling,
                                 IncludeKind includeKind,
                                 CacheSetIt cacheIt) const;

    static bool isStartOfLine(const QString &content, int current);

    mutable CacheSet m_cache;
    ResolutionMode m_mode;
};

} // Internal
} // ClangCodeModel

#endif // INCLUDETRACKER_H
