/*******************************************************************************
The MIT License (MIT)

Copyright (c) 2015 Sergey Shambir

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*******************************************************************************/

#pragma once
#include "../../goeditor_global.h"
#include <QSet>
#include "sourcecodestream.h"

namespace GoEditor {
namespace Internal {

enum Format {
    Format_Number = 0,
    Format_Rune, // Rune is character constant, surrounded by ''.
    Format_String,
    Format_Keyword,
    Format_Comment,
    Format_Identifier,
    Format_Whitespace,
    Format_PredeclaratedType,
    Format_PredeclaratedConst,
    Format_PredeclaratedFunction,
    Format_Operator,

    Format_FormatsAmount,
    Format_EndOfBlock
};

class FormatToken
{
public:
    FormatToken() {}

    FormatToken(Format format, size_t position, size_t length)
        : m_format(format)
        , m_position(position)
        , m_length(length)
    {}

    inline Format format() const { return m_format; }
    inline int begin() const { return m_position; }
    inline int end() const { return m_position + m_length; }
    inline int length() const { return m_length; }

private:
    Format m_format;
    int m_position;
    int m_length;
};

class GoScanner
{
    Q_DISABLE_COPY(GoScanner)
public:
    enum State {
        State_Default = 0,
        State_MultiLineComment,
        State_MultiLineString
    };

    GoScanner(const QChar *text, const int length);
    ~GoScanner();

    void setState(int state);
    int state() const;
    FormatToken read();
    QString value(const FormatToken& tk) const;

private:
    FormatToken onDefaultState();

    FormatToken readStringLiteral(QChar quoteChar);
    FormatToken readMultiLineComment();
    FormatToken readMultiLineString();
    FormatToken readIdentifier();
    FormatToken readNumber();
    FormatToken readFloatNumber();
    FormatToken readComment();
    FormatToken readWhiteSpace();
    FormatToken readOperator();

    void clearState();
    void saveState(State state, QChar savedData);
    void parseState(State &state, QChar &savedData) const;

    SourceCodeStream m_src;
    int m_state;
    const QSet<QString> m_keywords;
    const QSet<QString> m_types;
    const QSet<QString> m_consts;
    const QSet<QString> m_funcs;
};

} // namespace GoEditor
} // namespace Internal
