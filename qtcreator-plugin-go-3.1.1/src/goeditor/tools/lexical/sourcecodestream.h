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
#include <QString>

namespace GoEditor {
namespace Internal {

class SourceCodeStream
{
public:
    SourceCodeStream(const QChar *text, int length)
        :m_text(text)
        ,m_textLength(length)
        ,m_position(0)
        ,m_markedPosition(0)
    {}

    inline void setAnchor()
    {
        m_markedPosition = m_position;
    }

    inline void move()
    {
        ++m_position;
    }

    inline int length() const
    {
        return m_position - m_markedPosition;
    }

    inline int anchor() const
    {
        return m_markedPosition;
    }

    inline bool isEnd() const
    {
        return m_position >= m_textLength;
    }

    inline QChar peek(int offset = 0) const
    {
        int pos = m_position + offset;
        if (pos >= m_textLength)
            return QLatin1Char('\0');
        return m_text[pos];
    }

    inline QString value() const
    {
        const QChar *start = m_text + m_markedPosition;
        return QString(start, length());
    }

    inline QString value(int begin, int length) const
    {
        return QString(m_text + begin, length);
    }

private:
    const QChar *m_text;
    const int m_textLength;
    int m_position;
    int m_markedPosition;
};

} // namespace Internal
} // namespace GoEditor
