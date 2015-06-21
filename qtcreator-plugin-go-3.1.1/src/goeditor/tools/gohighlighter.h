#ifndef GOEDITOR_GOHIGHLIGHTER_H
#define GOEDITOR_GOHIGHLIGHTER_H

#include <texteditor/syntaxhighlighter.h>
#include <texteditor/fontsettings.h>

namespace GoEditor {

class GoHighlighter : public TextEditor::SyntaxHighlighter
{
    Q_OBJECT
public:
    explicit GoHighlighter(QTextDocument *document);
    ~GoHighlighter();

protected:
    void highlightBlock(const QString &text) override;

private:
    int highlightLine(const QString &text, int initialState);
};

} // namespace GoEditor

#endif // GOEDITOR_GOHIGHLIGHTER_H
