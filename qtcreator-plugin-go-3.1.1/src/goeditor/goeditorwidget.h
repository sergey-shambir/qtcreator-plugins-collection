#ifndef GOEDITOR_INTERNAL_GOEDITORWIDGET_H
#define GOEDITOR_INTERNAL_GOEDITORWIDGET_H

#include <texteditor/basetexteditor.h>
#include <utils/uncommentselection.h>

namespace GoEditor {
namespace Internal {

class GoEditorWidget : public TextEditor::BaseTextEditorWidget
{
    Q_OBJECT
public:
    explicit GoEditorWidget(QWidget *parent = nullptr);
    explicit GoEditorWidget(GoEditorWidget *other);
    ~GoEditorWidget();

    void unCommentSelection() override;

protected:
    TextEditor::BaseTextEditor *createEditor() override;

private:
    GoEditorWidget(TextEditor::BaseTextEditorWidget *) = delete;
    void ctor();

    Utils::CommentDefinition m_commentDefinition;
};

} // namespace Internal
} // namespace GoEditor

#endif // GOEDITOR_INTERNAL_GOEDITORWIDGET_H
