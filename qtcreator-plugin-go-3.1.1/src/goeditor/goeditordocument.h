#pragma once
#include "goeditor_global.h"
#include <texteditor/basetextdocument.h>
#include "tools/gocodetask.h"
#include "tools/highlighttask.h"
#include <QFutureWatcher>

namespace GoEditor {

class GOEDITOR_EXPORT GoEditorDocument : public TextEditor::BaseTextDocument
{
    Q_OBJECT
public:
    explicit GoEditorDocument();
    ~GoEditorDocument();

protected:
    void applyFontSettings();
    void triggerPendingUpdates();

private slots:
    void updateSemaHighlightsNow();
    void acceptSemaHighlights(int from, int to);
    void finishSemaHighlights();

private:
    int m_indexRevision = 0;
    QFutureWatcher<TextEditor::HighlightingResult> m_indexerWatcher;
    QHash<int, QTextCharFormat> m_highlightFormatMap;
    QTimer *m_semaHighlightsUpdater = nullptr;
};

} // namespace GoEditor
