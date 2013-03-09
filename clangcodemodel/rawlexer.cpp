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

#include "rawlexer.h"
#include "constants.h"

#include <clang/Lex/Lexer.h>

#include <QtCore/QDebug>

using namespace ClangCodeModel;

RawLexer::RawLexer()
    : m_state(Normal)
{
    m_langOptions.CPlusPlus = 1;
    m_langOptions.Bool = 1;
}

void RawLexer::init()
{
    m_keywords.load(m_langOptions);
}

void RawLexer::includeQt()
{
    // @TODO
}

void RawLexer::includeTrigraphs()
{
    m_langOptions.Trigraphs = 1;
}

void RawLexer::includeDigraphs()
{
    m_langOptions.Digraphs = 1;
}

void RawLexer::includeC99()
{
    m_langOptions.C99 = 1;
}

void RawLexer::includeCpp0x()
{
    m_langOptions.CPlusPlus0x = 1;
}

void RawLexer::includeCppOperators()
{
    m_langOptions.CXXOperatorNames = 1;
}

QList<Token> RawLexer::lex(const QString &code, int *state)
{
    if (code.isEmpty())
        return QList<Token>();

    if (state) {
        if (*state == -1)
            *state = Normal;
        m_state = State(*state);
    } else {
        m_state = Normal;
    }

    unsigned artificialOffset = 0;
    QString lexedCode = code;
    if (m_state == InComment) {
        lexedCode.prepend(QLatin1String("/*"));
        artificialOffset = 2;
    } else if (m_state == InDoxygenComment) {
        lexedCode.prepend(QLatin1String("/*!"));
        artificialOffset = 3;
    } else if (m_state == InString) {
        lexedCode.prepend(QLatin1Char('"'));
        artificialOffset = 1;
    }

    // Encoding?
    QByteArray rawCode = lexedCode.toLatin1();

    const char *begin = rawCode.constData();
    const char *end = begin + rawCode.length();
    clang::Lexer lexer(clang::SourceLocation(), m_langOptions, begin, begin, end);
    lexer.SetCommentRetentionState(true); // We want to identify comments.
    lexer.SetKeepWhitespaceMode(true); // For broken comments.

    QList<Token> tokens;
    clang::Token resultToken;
    do {
        lexer.LexFromRawLexer(resultToken);
        if (resultToken.is(clang::tok::eof))
            break;

        Token token;
        token.m_begin = lexer.getBufferLocation() - resultToken.getLength() - begin;
        token.m_length = resultToken.getLength();

        if (resultToken.isLiteral()) {
            token.m_flags.m_kind = Token::Literal;
            if (!lexedCode.at(token.m_begin).isDigit()) {
                if (m_state == InString)
                    m_state = Normal;
            }
        } else if (resultToken.is(clang::tok::raw_identifier)) {
            // Raw lexing doesn't lookup keywords, so we use the artificial table.
            if (m_keywords.contains(resultToken.getRawIdentifierData(), resultToken.getLength()))
                token.m_flags.m_kind = Token::Keyword;
            else
                token.m_flags.m_kind = Token::Identifier;
        } else if (resultToken.is(clang::tok::comment)) {
            token.m_flags.m_kind = Token::Comment;
            checkDoxygenComment(lexedCode, &token);
            if (m_state == InComment || m_state == InDoxygenComment) {
                m_state = Normal;
            }
        } else if (resultToken.is(clang::tok::unknown)) {
            // Check for a broken comment or a broken string.
            const QChar &c = lexedCode.at(token.m_begin);
            if (c == Constants::kSlash) {
                if (token.m_begin + 1 < static_cast<unsigned>(lexedCode.length())) {
                    if (lexedCode.at(token.m_begin + 1) == Constants::kStar) {
                        token.m_flags.m_kind = Token::Comment;
                        checkDoxygenComment(lexedCode, &token);
                        if (token.m_flags.m_doxygenComment)
                            m_state = InDoxygenComment;
                        else
                            m_state = InComment;
                    }
                }
            } else if (c == Constants::kDoubleQuote) {
                token.m_flags.m_kind = Token::Literal;
                m_state = InString;
            }
        } else {
            token.m_flags.m_kind = Token::Punctuation;
        }

        if (artificialOffset) {
            token.m_begin = token.m_begin < artificialOffset ? 0u : token.m_begin - artificialOffset;
            if (tokens.isEmpty()) {
                Q_ASSERT(token.m_length >= artificialOffset);
                token.m_length -= artificialOffset;
            }
        }

        if (!token.m_length)
            break;

        tokens.push_back(token);
    } while (lexer.getBufferLocation() < end);

    if (state)
        *state = m_state;

    return tokens;
}

void RawLexer::checkDoxygenComment(const QString &lexedCode, Token *token)
{
    if (token->m_begin + 2 < static_cast<unsigned>(lexedCode.length())) {
        if ((lexedCode.at(token->m_begin + 1) == Constants::kStar
                && lexedCode.at(token->m_begin + 2) == Constants::kExclamation)
                || (token->m_begin + 3 < static_cast<unsigned>(lexedCode.length())
                    && lexedCode.at(token->m_begin + 1) == Constants::kSlash
                    && lexedCode.at(token->m_begin + 2) == Constants::kSlash
                    && lexedCode.at(token->m_begin + 3) == Constants::kSpace)) {
            token->m_flags.m_doxygenComment = 1;
        }
    }
}
