#ifndef GOEDITOR_INTERNAL_GOEDITORWIDGET_H
#define GOEDITOR_INTERNAL_GOEDITORWIDGET_H

#include <texteditor/basetexteditor.h>
#include <utils/uncommentselection.h>
#include "tools/gocodetask.h"
#include "tools/highlighttask.h"
#include <QFuture>
#include <QFutureWatcher>

namespace GoEditor {
namespace Internal {

class GoEditorWidget : public TextEditor::BaseTextEditorWidget
{
    Q_OBJECT
public:
    explicit GoEditorWidget(QWidget *parent = nullptr);
    explicit GoEditorWidget(GoEditorWidget *other);
    virtual ~GoEditorWidget();

    void unCommentSelection() override;

protected:
    TextEditor::BaseTextEditor *createEditor();

private slots:
    void deferUpdateSemaHighlights();
    void updateSemaHighlightsNow();
    void acceptSemaHighlights(int from, int to);
    void finishSemaHighlights();

private:
    GoEditorWidget(TextEditor::BaseTextEditorWidget *) = delete;
    void ctor();

    int m_semaHighlighterRevision;
    QFuture<TextEditor::HighlightingResult> m_semaHighlighter;
    QFutureWatcher<TextEditor::HighlightingResult> m_semaHighlightWatcher;
    Utils::CommentDefinition m_commentDefinition;
    QTimer *m_semaHighlightsUpdater;
    QHash<int, QTextCharFormat> m_semaHighlightFormatMap;
};

} // namespace Internal
} // namespace GoEditor

#endif // GOEDITOR_INTERNAL_GOEDITORWIDGET_H
