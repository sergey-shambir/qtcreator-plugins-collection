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

#ifndef KEYWORDS_H
#define KEYWORDS_H

#include "clang_global.h"

#include <clang-c/Index.h>

#include <QtCore/QScopedPointer>

namespace clang {
class IdentifierTable;
class LangOptions;
}

namespace ClangCodeModel {

/*
 * When lexing in raw mode the identifier table is not looked up. This works as a replacement
 * for keywords in this case and for any other in which there's no parsing context.
 */
class CLANG_EXPORT Keywords
{
public:
    Keywords();
    ~Keywords();

    void load(const clang::LangOptions &options);
    bool contains(const char *buffer, size_t length) const;

private:
    QScopedPointer<clang::IdentifierTable> m_table;
};

} // Clang

#endif // KEYWORDS_H
