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

#include "codenavigator.h"
#include "symbol.h"
#include "indexer.h"
#include "unitsetup.h"
#include "utils_p.h"

using namespace ClangCodeModel;
using namespace Internal;

CodeNavigator::CodeNavigator()
{}

CodeNavigator::~CodeNavigator()
{}

void CodeNavigator::setup(const QString &fileName, Indexer *indexer)
{
    m_setup.reset(new UnitSetup(fileName, indexer));
}

SourceLocation CodeNavigator::followItem(unsigned line, unsigned column) const
{
    const CXCursor &cursor = getCursor(line, column);
    if (clang_equalCursors(cursor, clang_getNullCursor()))
        return SourceLocation();

    CXCursorKind cursorKind = clang_getCursorKind(cursor);

    // @TODO: Doesn't catch the include when it's in the bracket form and the position is
    // within the brackets. Expected?
    if (cursorKind == CXCursor_InclusionDirective)
        return findInclude(cursor);

    return findDefinition(cursor, cursorKind);
}

namespace {

struct FindDeclarationData
{
    FindDeclarationData()
        : m_visitNamespaces(false)
        , m_declarationCursor(clang_getNullCursor())
    {}

    bool m_visitNamespaces;
    CXCursor m_declarationCursor;
    CXCursor m_canonicalCursor;
};

CXChildVisitResult findDeclaration(CXCursor cursor,
                                   CXCursor parentCursor,
                                   CXClientData clientData)
{
    Q_UNUSED(parentCursor);

    FindDeclarationData *data = static_cast<FindDeclarationData *>(clientData);

    if (!clang_isCursorDefinition(cursor)
            && clang_equalCursors(cursor, data->m_canonicalCursor)) {
        data->m_declarationCursor = cursor;
        return CXChildVisit_Break;
    }

    if (clang_getCursorKind(cursor) == CXCursor_Namespace
            && data->m_visitNamespaces) {
        return CXChildVisit_Recurse;
    }

    return CXChildVisit_Continue;
}

} // Anonymous

SourceLocation CodeNavigator::switchDeclarationDefinition(unsigned line, unsigned column) const
{
    CXCursor cursor = getCursor(line, column);
    if (clang_equalCursors(cursor, clang_getNullCursor()))
        return SourceLocation();

    CXCursorKind cursorKind = clang_getCursorKind(cursor);
    if (clang_isReference(cursorKind)) {
        cursor = clang_getCursorReferenced(cursor);
        cursorKind = clang_getCursorKind(cursor);
    }

    if (!(cursorKind == CXCursor_Constructor
            || cursorKind == CXCursor_Destructor
            || cursorKind == CXCursor_CXXMethod
            || cursorKind == CXCursor_FunctionTemplate
            || cursorKind == CXCursor_FunctionDecl)) {
        return SourceLocation();
    }

    if (clang_isCursorDefinition(cursor)) {
        FindDeclarationData data;

        CXCursor parent = clang_getNullCursor();
        if (cursorKind == CXCursor_Constructor
                || cursorKind == CXCursor_Destructor
                || cursorKind == CXCursor_CXXMethod) {
            parent = clang_getCursorSemanticParent(cursor);
        } else if (cursorKind == CXCursor_FunctionTemplate) {
            const CXCursor &semaParent = clang_getCursorSemanticParent(cursor);
            CXCursorKind semaParentKind = clang_getCursorKind(semaParent);
            if (semaParentKind == CXCursor_ClassDecl
                    || semaParentKind == CXCursor_StructDecl
                    || semaParentKind == CXCursor_ClassTemplate
                    || semaParentKind == CXCursor_ClassTemplatePartialSpecialization) {
                parent = semaParent;
            }
        }

        if (clang_equalCursors(parent, clang_getNullCursor())) {
            const CXCursor &semaParent = clang_getCursorSemanticParent(cursor);
            CXCursorKind semaParentKind = clang_getCursorKind(semaParent);
            if (semaParentKind == CXCursor_Namespace)
                data.m_visitNamespaces = true;
            parent = m_setup->unit().getTranslationUnitCursor();
        }

        data.m_canonicalCursor = clang_getCanonicalCursor(cursor);
        clang_visitChildren(parent, findDeclaration, &data);

        if (clang_equalCursors(data.m_declarationCursor, clang_getNullCursor()))
            return SourceLocation();

        return Internal::getSpellingLocation(clang_getCursorLocation(data.m_declarationCursor));
    }

    return findDefinition(cursor, cursorKind);
}

SourceLocation CodeNavigator::findDefinition(const CXCursor &cursor,
                                             CXCursorKind cursorKind) const
{
    CXCursor cursorDefinition = clang_getNullCursor();
    if (clang_isCursorDefinition(cursor))
        cursorDefinition = cursor;
    else
        cursorDefinition = clang_getCursorDefinition(cursor);

    if (!clang_equalCursors(cursorDefinition, clang_getNullCursor()))
        return Internal::getSpellingLocation(clang_getCursorLocation(cursorDefinition));

    // Definition is not in the unit, use indexed data to look for it.
    if (clang_isDeclaration(cursorKind)
            || clang_isReference(cursorKind)) {
        QList<Symbol> indexedSymbol;
        if (cursorKind == CXCursor_ClassDecl
                || cursorKind == CXCursor_StructDecl
                || cursorKind == CXCursor_UnionDecl) {
            indexedSymbol = m_setup->indexer()->allClasses();
        } else if (cursorKind == CXCursor_FunctionDecl
                   || cursorKind == CXCursor_FunctionTemplate
                   || cursorKind == CXCursor_CXXMethod) {
            indexedSymbol.append(m_setup->indexer()->allFunctions());
            indexedSymbol.append(m_setup->indexer()->allMethods());
        } else if (cursorKind == CXCursor_Constructor) {
            indexedSymbol = m_setup->indexer()->allConstructors();
        } else if (cursorKind == CXCursor_Destructor) {
            indexedSymbol = m_setup->indexer()->allDestructors();
        }

        if (!indexedSymbol.isEmpty()) {
            const QString &spelling = Internal::getQString(clang_getCursorSpelling(cursor));
            // @TODO: Take qualification into consideration.
            foreach (const Symbol &symbol, indexedSymbol) {
                if (symbol.m_name == spelling) {
                    return symbol.m_location;
                }
            }
        }
    }

    return SourceLocation();
}

SourceLocation CodeNavigator::findInclude(const CXCursor &cursor) const
{
    CXFile includedFile = clang_getIncludedFile(cursor);
    return SourceLocation(getQString(clang_getFileName(includedFile)));
}

CXCursor CodeNavigator::getCursor(unsigned line, unsigned column) const
{
    m_setup->checkForNewerUnit();

    if (!m_setup->unit().isLoaded())
        return clang_getNullCursor();

    const CXFile &file = m_setup->unit().getFile();
    const CXSourceLocation &location = m_setup->unit().getLocation(file, line, column);
    if (clang_equalLocations(location, clang_getNullLocation()))
        return clang_getNullCursor();

    return m_setup->unit().getCursor(location);
}
