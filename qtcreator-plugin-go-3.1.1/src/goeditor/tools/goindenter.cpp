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

#include "goindenter.h"
#include <texteditor/tabsettings.h>

namespace GoEditor {

GoIndenter::GoIndenter()
{
}

GoIndenter::~GoIndenter()
{
}

bool GoIndenter::isElectricCharacter(const QChar &ch) const
{
    return ch == QLatin1Char('(') || ch == QLatin1Char('{');
}

/**
 * @brief Indents one block (i.e. one line) of code
 * @param doc Unused
 * @param block Block that represents line
 * @param typedChar Unused
 * @param tabSettings An IDE tabulation settings
 *
 * Usually this method called once when you begin new line of code by pressing
 * Enter. If Indenter reimplements indent() method, than indentBlock() may be
 * called in other cases.
 */
void GoIndenter::indentBlock(QTextDocument *document,
                                 const QTextBlock &block,
                                 const QChar &typedChar,
                                 const TextEditor::TabSettings &settings)
{
    // TODO: indent 'case ABCD:'
    Q_UNUSED(document);
    Q_UNUSED(typedChar);
    QTextBlock previousBlock = block.previous();
    if (previousBlock.isValid()) {
        QString previousLine = previousBlock.text();
        int indentation = settings.indentationColumn(previousLine);
        if (isElectricLine(previousLine))
            indentation += settings.m_tabSize;
        settings.indentLine(block, indentation);
    } else {
        // First line in whole document
        settings.indentLine(block, 0);
    }
}

/// @return True if electric character is last non-space character at given string
bool GoIndenter::isElectricLine(const QString &line) const
{
    QString trimmed = line.trimmed();
    if (trimmed.isEmpty())
        return false;

    QChar last = trimmed[trimmed.size() - 1];
    if (isElectricCharacter(last))
        return true;
    if (last == QLatin1Char(':') && trimmed.startsWith(QLatin1String("case")))
        return true;

    return false;
}

} // namespace GoEditor
