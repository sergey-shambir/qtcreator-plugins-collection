#include "gohighlighter.h"
#include "lexical/goscanner.h"

#include <texteditor/texteditorconstants.h>

namespace GoEditor {

using namespace GoEditor::Internal;

/// @return List that maps enum Format values to TextEditor plugin formats
QVector<TextEditor::TextStyle> initFormatCategories()
{
    QVector<TextEditor::TextStyle> categories(Format_FormatsAmount);
    categories[Format_Number] = TextEditor::C_NUMBER;
    categories[Format_Rune] = TextEditor::C_NUMBER;
    categories[Format_String] = TextEditor::C_STRING;
    categories[Format_Keyword] = TextEditor::C_KEYWORD;
    categories[Format_Comment] = TextEditor::C_COMMENT;
    categories[Format_Identifier] = TextEditor::C_TEXT;
    categories[Format_Whitespace] = TextEditor::C_VISUAL_WHITESPACE;
    categories[Format_PredeclaratedType] = TextEditor::C_TYPE;
    categories[Format_PredeclaratedConst] = TextEditor::C_ENUMERATION;
    categories[Format_PredeclaratedFunction] = TextEditor::C_FUNCTION;
    categories[Format_Operator] = TextEditor::C_OPERATOR;

    return categories;
}

GoHighlighter::GoHighlighter(QTextDocument *document) :
    TextEditor::SyntaxHighlighter(document)
{
    setTextFormatCategories(initFormatCategories());
}

GoHighlighter::~GoHighlighter()
{
}

void GoHighlighter::highlightBlock(const QString &text)
{
    int initialState = previousBlockState();
    if (initialState == -1)
        initialState = 0;
    setCurrentBlockState(highlightLine(text, initialState));
}

int GoHighlighter::highlightLine(const QString &text, int initialState)
{
    GoScanner scanner(text.constData(), text.size());
    scanner.setState(initialState);

    FormatToken tk;
    while ((tk = scanner.read()).format() != Format_EndOfBlock)
        setFormat(tk.begin(), tk.length(), formatForCategory(tk.format()));
    return scanner.state();
}

} // namespace GoEditor
