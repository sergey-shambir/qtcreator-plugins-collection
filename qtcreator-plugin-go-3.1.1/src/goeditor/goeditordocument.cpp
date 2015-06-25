#include "goeditordocument.h"
#include "tools/gohighlighter.h"
#include "tools/goindenter.h"
#include "tools/gocodetask.h"
#include "tools/highlighttask.h"
#include <QTimer>
#include <QTextDocument>

static const int UPDATE_HIGHLIGHTS_INTERVAL_MSEC = 250;

using namespace ::GoEditor::Internal;

namespace GoEditor {

GoEditorDocument::GoEditorDocument()
{
    setSyntaxHighlighter(new GoHighlighter(document()));
    setIndenter(new GoIndenter);

    connect(&m_indexerWatcher, SIGNAL(resultsReadyAt(int,int)),
            this, SLOT(acceptSemaHighlights(int,int)));
    connect(&m_indexerWatcher, SIGNAL(finished()),
            this, SLOT(finishSemaHighlights()));

    m_semaHighlightsUpdater = new QTimer(this);
    m_semaHighlightsUpdater->setSingleShot(true);
    m_semaHighlightsUpdater->setInterval(UPDATE_HIGHLIGHTS_INTERVAL_MSEC);
    connect(m_semaHighlightsUpdater, SIGNAL(timeout()),
            this, SLOT(updateSemaHighlightsNow()));
}

GoEditorDocument::~GoEditorDocument()
{
}

void GoEditorDocument::applyFontSettings()
{
    BaseTextDocument::applyFontSettings();

    const TextEditor::FontSettings &fs = fontSettings();
    m_highlightFormatMap[HighlightRange::Type] =
            fs.toTextCharFormat(TextEditor::C_TYPE);
    m_highlightFormatMap[HighlightRange::Var] =
            fs.toTextCharFormat(TextEditor::C_LOCAL);
    m_highlightFormatMap[HighlightRange::Field] =
            fs.toTextCharFormat(TextEditor::C_FIELD);
    m_highlightFormatMap[HighlightRange::Const] =
            fs.toTextCharFormat(TextEditor::C_ENUMERATION);
    m_highlightFormatMap[HighlightRange::Label] =
            fs.toTextCharFormat(TextEditor::C_LABEL);
    m_highlightFormatMap[HighlightRange::Func] =
            fs.toTextCharFormat(TextEditor::C_VIRTUAL_METHOD);
    m_highlightFormatMap[HighlightRange::Package] =
            fs.toTextCharFormat(TextEditor::C_STRING);

    updateSemaHighlightsNow();
}

void GoEditorDocument::triggerPendingUpdates()
{
    TextEditor::BaseTextDocument::triggerPendingUpdates();
    m_semaHighlightsUpdater->start();
}

void GoEditorDocument::updateSemaHighlightsNow()
{
    m_indexRevision = document()->revision();
    SingleShotHighlightTask *task = new SingleShotHighlightTask;
    task->setFilename(filePath());
    task->setText(plainText().toUtf8());
    m_indexerWatcher.setFuture(task->start());
}

void GoEditorDocument::acceptSemaHighlights(int from, int to)
{
    if (m_indexRevision != document()->revision() || m_indexerWatcher.isCanceled())
            return; // aborted

    TextEditor::SemanticHighlighter::incrementalApplyExtraAdditionalFormats(
                syntaxHighlighter(), m_indexerWatcher.future(), from, to, m_highlightFormatMap);
}

void GoEditorDocument::finishSemaHighlights()
{
    if (m_indexRevision != document()->revision() || m_indexerWatcher.isCanceled())
            return; // aborted

    TextEditor::SemanticHighlighter::clearExtraAdditionalFormatsUntilEnd(
                syntaxHighlighter(), m_indexerWatcher.future());
}

} // namespace GoEditor
