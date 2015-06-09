#include "goautocompleter.h"

namespace GoEditor {
namespace Internal {

GoAutoCompleter::GoAutoCompleter()
{
}

GoAutoCompleter::~GoAutoCompleter()
{
}

bool GoAutoCompleter::contextAllowsAutoParentheses(const QTextCursor &cursor, const QString &textToInsert) const
{
}

bool GoAutoCompleter::contextAllowsElectricCharacters(const QTextCursor &cursor) const
{
}

bool GoAutoCompleter::isInComment(const QTextCursor &cursor) const
{
}

QString GoAutoCompleter::insertMatchingBrace(const QTextCursor &cursor, const QString &text,
                                             QChar la, int *skippedChars) const
{
}

QString GoAutoCompleter::insertParagraphSeparator(const QTextCursor &cursor) const
{
}

} // namespace Internal
} // namespace GoEditor
