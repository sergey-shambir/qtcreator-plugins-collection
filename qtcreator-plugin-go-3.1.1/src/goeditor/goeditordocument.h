#pragma once
#include "goeditor_global.h"
#include <texteditor/basetextdocument.h>
#include "tools/gosemanticinfo.h"
#include "tools/highlighttask.h"
#include <QFutureWatcher>

namespace GoEditor {

class GOEDITOR_EXPORT GoEditorDocument : public TextEditor::BaseTextDocument
{
    Q_OBJECT
public:
    explicit GoEditorDocument();
    ~GoEditorDocument();

    bool save(QString *errorString, const QString &fileName, bool autoSave) override;

public slots:
    void deferSemanticUpdate();

signals:
    void semanticUpdated(const GoSemanticInfoPtr &semantic);

protected:
    void applyFontSettings() override;
    void triggerPendingUpdates() override;

private slots:
    void updateSemaHighlightsNow();
    void acceptSemaHighlights(int from, int to);
    void finishSemaHighlights();
    void acceptSemantic(int from, int to);
    void fixTabSettings();

private:
    int m_indexRevision = 0;
    QFutureWatcher<TextEditor::HighlightingResult> m_indexerWatcher;
    QFutureWatcher<GoSemanticInfoPtr> m_semanticWatcher;
    QHash<int, QTextCharFormat> m_highlightFormatMap;
    QTimer *m_semaHighlightsUpdater = nullptr;
    bool m_isFixingTabSettings = false;
};

} // namespace GoEditor
