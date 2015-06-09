#ifndef GOEDITOR_INTERNAL_GOEDITOR_H
#define GOEDITOR_INTERNAL_GOEDITOR_H

#include <texteditor/basetexteditor.h>

namespace GoEditor {
namespace Internal {

class GoEditorWidget;

class GoEditor : public TextEditor::BaseTextEditor
{
    Q_OBJECT
public:
    explicit GoEditor(GoEditorWidget *editorWidget);
    ~GoEditor();

    bool duplicateSupported() const override { return true; }
    Core::IEditor *duplicate() override;
    TextEditor::CompletionAssistProvider *completionAssistProvider() override;

    /**
      Opens file for editing, actual work performed by base class
      */
    bool open(QString *errorString,
              const QString &fileName,
              const QString &realFileName) override;
};

} // namespace Internal
} // namespace GoEditor

#endif // GOEDITOR_INTERNAL_GOEDITOR_H
