#ifndef GOEDITOR_GOINDENTER_H
#define GOEDITOR_GOINDENTER_H

#include <texteditor/indenter.h>
#include <QStringList>

namespace GoEditor {

class GoIndenter : public TextEditor::Indenter
{
public:
    GoIndenter();
    ~GoIndenter();

    bool isElectricCharacter(const QChar &ch) const;
    void indentBlock(QTextDocument *document,
                     const QTextBlock &block,
                     const QChar &typedChar,
                     const TextEditor::TabSettings &settings);

private:
    bool isElectricLine(const QString &line) const;
};

} // namespace GoEditor

#endif // GOEDITOR_GOINDENTER_H
