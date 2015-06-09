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
