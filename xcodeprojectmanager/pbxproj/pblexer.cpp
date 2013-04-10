/*
 * Copyright (c) 2013 Sergey Shambir
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "pblexer.h"
#include <cctype>

namespace XCodeProjectManager {

static bool isNameOrNumberChar(char ch)
{
    return std::isalnum(ch) || strchr("._/", ch);
}

Lexer::Lexer(const QByteArray &source)
    : m_source(source)
    , m_position(0)
    , m_line(1)
    , m_column(1)
{
}

Lexer::~Lexer()
{
}

Token::Kind Lexer::readNext()
{
    m_token.kind = Token::EndOfFile;
    m_token.maybeMD5 = false;
    m_token.text.clear();

    bool repeat = true;
    while (repeat) {
        skipWhitespace();
        if (m_position >= m_source.size())
            return Token::EndOfFile;

        repeat = false;
        const char ch = peek(0);
        if (ch == '/' && strchr("/*", peek(1))) {
            readComment();
        } else if (isNameOrNumberChar(ch)) {
            readNameOrNumber();
        } else if (ch == '"') {
            readString();
        } else if (ch == '{') {
            skip();
            m_token.kind = Token::LeftBrace;
        } else if (ch == '}') {
            skip();
            m_token.kind = Token::RightBrace;
        } else if (ch == '(') {
            skip();
            m_token.kind = Token::LeftParen;
        } else if (ch == ')') {
            skip();
            m_token.kind = Token::RightParen;
        } else if (ch == ';') {
            skip();
            m_token.kind = Token::Semicolon;
        } else if (ch == ',') {
            skip();
            m_token.kind = Token::Comma;
        } else if (ch == '=') {
            skip();
            m_token.kind = Token::Equal;
        } else {
            // just skip
            skip();
            repeat = true;
        }
    }
    return m_token.kind;
}

const Token &Lexer::token() const
{
    return m_token;
}

Token::Kind Lexer::tokenKind() const
{
    return m_token.kind;
}

int Lexer::line() const
{
    return m_line;
}

int Lexer::column() const
{
    return m_column;
}

void Lexer::skipWhitespace()
{
    while (m_position < m_source.size()) {
        const char ch = m_source[m_position];
        if (!std::isspace(ch))
            break;
        skip();
    }
}

void Lexer::readNameOrNumber()
{
    const int start = m_position;
    while (isNameOrNumberChar(peek()))
        skip();
    const int length = m_position - start;
    const QByteArray text = m_source.mid(start, length);
    m_token.text = QString::fromUtf8(text);
    m_token.kind = Token::Name;

    bool maybeMD5 = m_token.text.size() == 4 * 6;
    for (int i = 0; maybeMD5 && i < m_token.text.size(); ++i) {
        const char ch = text[i];
        maybeMD5 = std::isdigit(ch) || (ch >= 'A' && ch <= 'F');
    }
    m_token.maybeMD5 = maybeMD5;
}

void Lexer::readComment()
{
    m_token.kind = Token::Comment;
    // distinct single-line and multi-line
    if (peek(1) == '/') {
        m_position += 2;
        int start = m_position;
        forever {
            char ch = peek();
            if (ch == '\n') {
                skip();
                break;
            }
            if (ch == '\0')
                break;
            skip();
        }
        int length = m_position - start;
        if (length)
            m_token.text = QString::fromUtf8(m_source.mid(start, length).trimmed());
    } else {
        m_position += 2;
        int start = m_position;
        forever {
            char ch = peek();
            if (ch == '\0' || (ch == '*' && peek(1) == '/'))
                break;
            skip();
        }
        m_position += 2;
        int length = m_position - start - 2; // without '*/'
        if (length)
            m_token.text = QString::fromUtf8(m_source.mid(start, length).trimmed());
    }
}

void Lexer::readString()
{
    m_token.kind = Token::String;
    int start = m_position;
    skip();
    forever {
        char ch = peek();
        if (ch == '\\' && peek(1) == '\"') {
            skip();
            skip();
            continue;
        }
        if (ch == '\n' || ch == '\0' || ch == '\"')
            break;
        skip();
    }
    m_position += 1;
    int length = m_position - start;
    if (length)
        m_token.text = QString::fromUtf8(m_source.mid(start, length));
}

char Lexer::peek(int step) const
{
    int index = m_position + step;
    if (index < m_source.size())
        return m_source[index];
    return '\0';
}

void Lexer::skip()
{
#ifdef DEBUG_PBPROJ
    char ch = peek();
    if (ch == '\n') {
        ++m_line;
        m_column = 1;
    } else {
        ++m_column;
    }
#endif
    ++m_position;
}

} // namespace XCodeProjectManager
