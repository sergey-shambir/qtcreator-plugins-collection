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

#include "token.h"
#include "constants.h"

#include <QtCore/QLatin1String>

using namespace ClangCodeModel;

Token::Token()
    : m_begin(-1)
    , m_length(-1)
    , m_flagsControl(0)
{}

bool Token::isCharCheck(const Token &token, const QString &code, const QLatin1Char &other)
{
    if (token.length() == 1)
        return code.at(token.begin()) == other;
    return false;
}

bool Token::isPunctuationLParen(const Token &token, const QString &code)
{
    return isCharCheck(token, code, Constants::kLParen);
}

bool Token::isPunctuationRParen(const Token &token, const QString &code)
{
    return isCharCheck(token, code, Constants::kRParen);
}

bool Token::isPunctuationLBrace(const Token &token, const QString &code)
{
    return isCharCheck(token, code, Constants::kLBrace);
}

bool Token::isPunctuationRBrace(const Token &token, const QString &code)
{
    return isCharCheck(token, code, Constants::kRBrace);
}

bool Token::isPunctuationLBracket(const Token &token, const QString &code)
{
    return isCharCheck(token, code, Constants::kLBracket);
}

bool Token::isPunctuationRBracket(const Token &token, const QString &code)
{
    return isCharCheck(token, code, Constants::kRBracket);
}

bool Token::isPunctuationLABracket(const Token &token, const QString &code)
{
    return isCharCheck(token, code, Constants::kLABracket);
}

bool Token::isPunctuationRABracket(const Token &token, const QString &code)
{
    return isCharCheck(token, code, Constants::kRABracket);
}

bool Token::isPunctuationSemiColon(const Token &token, const QString &code)
{
    return isCharCheck(token, code, Constants::kSemiColon);
}

bool Token::isPunctuationPound(const Token &token, const QString &code)
{
    return isCharCheck(token, code, Constants::kPound);
}

bool Token::isPunctuationColon(const Token &token, const QString &code)
{
    return isCharCheck(token, code, Constants::kColon);
}

bool Token::isPunctuationSpace(const Token &token, const QString &code)
{
    return isCharCheck(token, code, Constants::kSpace);
}

bool Token::isPunctuationNewLine(const Token &token, const QString &code)
{
    return isCharCheck(token, code, Constants::kNewLine);
}

bool Token::isLiteralNumeric(const Token &token, const QString &code)
{
    return code.at(token.m_begin).isDigit();
}

bool Token::isLiteralText(const Token &token, const QString &code)
{
    return !isLiteralNumeric(token, code);
}

bool Token::isDoxygenComment() const
{
    if (m_flags.m_kind == Comment)
        return m_flags.m_doxygenComment;
    return false;
}
