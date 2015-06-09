#ifndef GOEDITOR_INTERNAL_GOAUTOCOMPLETER_H
#define GOEDITOR_INTERNAL_GOAUTOCOMPLETER_H

#include <texteditor/autocompleter.h>

namespace GoEditor {
namespace Internal {

class GoAutoCompleter : public TextEditor::AutoCompleter
{
public:
    GoAutoCompleter();
    ~GoAutoCompleter();

    virtual bool contextAllowsAutoParentheses(const QTextCursor &cursor,
                                              const QString &textToInsert = QString()) const;
    virtual bool contextAllowsElectricCharacters(const QTextCursor &cursor) const;
    virtual bool isInComment(const QTextCursor &cursor) const;
    virtual QString insertMatchingBrace(const QTextCursor &cursor,
                                        const QString &text,
                                        QChar la,
                                        int *skippedChars) const;
    virtual QString insertParagraphSeparator(const QTextCursor &cursor) const;
};

} // namespace Internal
} // namespace GoEditor

#endif // GOEDITOR_INTERNAL_GOAUTOCOMPLETER_H
