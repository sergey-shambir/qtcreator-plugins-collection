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

#ifndef XCODEPROJECTMANAGER_OPENSTEPLEXER_H
#define XCODEPROJECTMANAGER_OPENSTEPLEXER_H

#include <QString>

namespace XCodeProjectManager {


class Token
{
public:
    enum Kind {
        EndOfFile = 0,
        Name,       // 1, MD5 checksum, name or number
        String,     // 2
        Comment,    // 3
        Equal,      // 4
        LeftBrace,  // 5
        RightBrace, // 6
        LeftParen,  // 7
        RightParen, // 8
        Semicolon,  // 9
        Comma       // 10
    };

    /**
     * @brief Clean text content, if applicable
     * Full name, full number, full string or stripped comment.
     */
    QString text;
    Kind kind;
    bool maybeMD5;
};

class Lexer
{
public:
    Lexer(const QByteArray &source);
    ~Lexer();

    /// @return false at end of file
    Token::Kind readNext();
    const Token &token() const;
    Token::Kind tokenKind() const;

    /// @return 1-based line number, or always 1 if DEBUG_PBPROJ macro undefined
    int line() const;
    /// @return 1-based column number, or always 1 if DEBUG_PBPROJ macro undefined
    int column() const;

protected:
    void skipWhitespace();
    void readNameOrNumber();
    void readComment();
    void readString();

private:
    char peek(int step = 0) const;
    void skip();

    Token m_token;
    QByteArray m_source;
    int m_position;
    int m_line;
    int m_column;
};

} // namespace XCodeProjectManager

#endif // XCODEPROJECTMANAGER_OPENSTEPLEXER_H
