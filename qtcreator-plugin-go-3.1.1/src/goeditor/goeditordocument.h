/*******************************************************************************
The MIT License (MIT)

Copyright (c) 2015 Sergey Shambir

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*******************************************************************************/

#pragma once
#include "goeditor_global.h"
#include <texteditor/basetextdocument.h>
#include "tools/highlighttask.h"
#include <QFutureWatcher>
#include <QSharedPointer>

namespace GoEditor {

class GoSemanticInfo;
typedef QSharedPointer<GoSemanticInfo> GoSemanticInfoPtr;

class GOEDITOR_EXPORT GoEditorDocument : public TextEditor::BaseTextDocument
{
    Q_OBJECT
public:
    explicit GoEditorDocument();
    ~GoEditorDocument();

    /// Can return nullptr.
    const GoSemanticInfoPtr &semanticInfo() const;
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
    bool reloadKeepHistory(QString &errorString);

    int m_indexRevision = 0;
    QFutureWatcher<TextEditor::HighlightingResult> m_indexerWatcher;
    QFutureWatcher<GoSemanticInfoPtr> m_semanticWatcher;
    QHash<int, QTextCharFormat> m_highlightFormatMap;
    QTimer *m_semaHighlightsUpdater = nullptr;
    bool m_isFixingTabSettings = false;
    GoSemanticInfoPtr m_semanticInfo;
};

} // namespace GoEditor
