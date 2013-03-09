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

#include "includetracker.h"
#include "rawlexer.h"
#include "constants.h"
#include "utils_p.h"

#include <utils/fileutils.h>

#include <QtCore/QLatin1String>
#include <QtCore/QSet>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtCore/QStringList>
#include <QtCore/QDebug>

using namespace ClangCodeModel;
using namespace ClangCodeModel::Internal;
using namespace ClangCodeModel::Constants;

inline uint qHash(const QStringList &all)
{
    uint h = 0;
    foreach (const QString &s, all)
        h ^= qHash(s);
    return h;
}

IncludeTracker::IncludeTracker()
    : m_mode(FirstMatchResolution)
{}

void IncludeTracker::setResolutionMode(ResolutionMode mode)
{
    m_mode = mode;
}

IncludeTracker::ResolutionMode IncludeTracker::resolutionMode() const
{
    return m_mode;
}

QStringList IncludeTracker::directIncludes(const QString &fileName,
                                           const QStringList &compilationOptions) const
{
    QHash<QStringList, OptionsCache>::iterator cacheIt = m_cache.find(compilationOptions);
    if (cacheIt == m_cache.end()) {
        OptionsCache cache;
        foreach (const QString &option, compilationOptions) {
            if (option.startsWith(QLatin1String("-I"))) {
                QString searchPath = QDir::cleanPath(option.right(option.length() - 2));
                if (!searchPath.endsWith(QLatin1Char('/')))
                    searchPath.append(QLatin1Char('/'));
                cache.m_searchPaths.append(searchPath);
            }
        }
        cacheIt = m_cache.insert(compilationOptions, cache);
    }

    QFileInfo fileInfo(fileName);
    if (!fileInfo.isAbsolute())
        return QStringList();

    Utils::FileReader reader;
    if (!reader.fetch(fileName, QIODevice::Text))
        return QStringList();
    QTextStream stream(reader.data());

    QString basePath = fileInfo.absolutePath();
    if (!basePath.endsWith(QLatin1Char('/')))
        basePath.append(QLatin1Char('/'));

    return parseIncludes(stream.readAll(), basePath, cacheIt);
}

QStringList IncludeTracker::parseIncludes(const QString &contents,
                                          const QString &basePath,
                                          ClangCodeModel::Internal::IncludeTracker::CacheSetIt cacheIt) const
{
    // The list of includes computed might not be totally correct. This is because no
    // preprocessing action is performed here, but instead we simply track valid #include
    // lines. For instance, includes inside an #ifdefed out block will be considered (which
    // shouldn't be problem). On the other hand, includes which result from macro expansion
    // will be missing.

    enum Status {
        Waiting,
        Preprocessing,
        PreprocessingInclude,
        ReadingAngleBracketInclude
    };
    Status status = Waiting;

    RawLexer lexer;
    lexer.init();
    const QList<Token> &tokens = lexer.lex(contents);

    QString bracketInclude;
    QSet<QString> uniqueIncludes;
    foreach (const Token &tk, tokens) {
        switch (status) {
        case Waiting:
            if (tk.is(Token::Punctuation)
                    && Token::isPunctuationPound(tk, contents)
                    && isStartOfLine(contents, tk.begin())) {
                status = Preprocessing;
            }
            break;
        case Preprocessing:
            if (tk.is(Token::Identifier)
                    && contents.midRef(tk.begin(), tk.length()) == QLatin1String("include")) {
                status = PreprocessingInclude;
            } else if (!(tk.is(Token::Punctuation) && tk.isPunctuationSpace(tk, contents))) {
                status = Waiting;
            }
            break;
        case PreprocessingInclude:
            if (tk.is(Token::Literal)) {
                QString quotedInclude = contents.mid(tk.begin(), tk.length());
                if (quotedInclude.length() >= 2) {
                    const QSet<QString> &resolved =
                            resolveInclude(basePath,
                                           quotedInclude.mid(1, quotedInclude.length() - 2),
                                           QuotedInclude,
                                           cacheIt);
                    if (!resolved.isEmpty())
                        uniqueIncludes.unite(resolved);
                }
                status = Waiting;
            } else if (tk.is(Token::Punctuation)
                       && Token::isPunctuationLABracket(tk, contents)) {
                // In raw lexing mode we don't get a nice angle string literal, so we
                // manually construct the file name.
                status = ReadingAngleBracketInclude;
            } else if (!(tk.is(Token::Punctuation) && tk.isPunctuationSpace(tk, contents))) {
                status = Waiting;
            }
            break;
        case ReadingAngleBracketInclude:
            if (tk.is(Token::Identifier)
                    || (tk.is(Token::Punctuation)
                        && !Token::isPunctuationRABracket(tk, contents)
                        && !Token::isPunctuationNewLine(tk, contents))) {
                bracketInclude.append(contents.midRef(tk.begin(), tk.length()));
            } else {
                if (tk.is(Token::Punctuation)
                       && Token::isPunctuationRABracket(tk, contents)
                       && !bracketInclude.isEmpty()) {
                    const QSet<QString> &resolved =
                            resolveInclude(basePath,
                                           bracketInclude,
                                           AngleBracketInclude,
                                           cacheIt);
                    if (!resolved.isEmpty())
                        uniqueIncludes.unite(resolved);
                }
                bracketInclude.clear();
                status = Waiting;
            }
            break;
        default:
            bracketInclude.clear();
            status = Waiting;
            break;
        }
    }

    return uniqueIncludes.toList();
}

QSet<QString> IncludeTracker::resolveInclude(const QString &basePath,
                                             const QString &includeSpelling,
                                             IncludeKind includeKind,
                                             CacheSetIt cacheIt) const
{
    // @TODO: Cache data...

    QSet<QString> resolutions;

    QFileInfo fileInfo(includeSpelling);
    if (fileInfo.isAbsolute()) {
        if (fileInfo.isFile())
            resolutions.insert(normalizeFileName(includeSpelling));
        if (m_mode == FirstMatchResolution)
            return resolutions;
    }

    if (includeKind == QuotedInclude) {
        fileInfo.setFile(basePath + includeSpelling);
        if (fileInfo.isFile()) {
            resolutions.insert(normalizeFileName(fileInfo));
            if (m_mode == FirstMatchResolution)
                return resolutions;
        }
    }

    foreach (const QString &searchPath, cacheIt->m_searchPaths) {
        fileInfo.setFile(searchPath + includeSpelling);
        if (fileInfo.isFile()) {
            resolutions.insert(normalizeFileName(fileInfo));
            if (m_mode == FirstMatchResolution)
                return resolutions;
        }
    }

    return resolutions;
}

bool IncludeTracker::isStartOfLine(const QString &contents, int offset)
{
    Q_ASSERT(offset >= 0 && offset < contents.length());

    while (offset > 0) {
         const QChar &c = contents.at(offset - 1);
         if (c == kSpace || c == kHorizontalTab)
            --offset;
         else
             break;
    }
    if (offset == 0 || contents.at(offset - 1) == kNewLine)
        return true;

    return false;
}

