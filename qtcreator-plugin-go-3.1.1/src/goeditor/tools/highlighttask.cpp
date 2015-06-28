#include "highlighttask.h"
#include "gosemkiprocess.h"
#include "goeditordocument.h"

namespace GoEditor {
namespace Internal {

SingleShotHighlightTask::SingleShotHighlightTask()
{
}

SingleShotHighlightTask::~SingleShotHighlightTask()
{
}

void SingleShotHighlightTask::setFilename(const QString &filename)
{
    m_filename = filename;
}

void SingleShotHighlightTask::setText(const QByteArray &text)
{
    m_text = text;
}

SingleShotHighlightTask::Result SingleShotHighlightTask::start()
{
    m_highlightFuture.setRunnable(this);
    m_semanticFuture.setRunnable(this);
    m_highlightFuture.reportStarted();
    m_semanticFuture.reportStarted();
    Result result = {
        m_highlightFuture.future(),
        m_semanticFuture.future()
    };
    QThreadPool::globalInstance()->start(this, QThread::LowestPriority);
    return result;
}

void SingleShotHighlightTask::run()
{
    GosemkiProcess process(m_filename, m_text);
    QSharedPointer<GoSemanticInfo> sema = process.collectSemanticInfo();
    int lastLine = 0;
    QVector<TextEditor::HighlightingResult> results;
    foreach (const GoHighlightRange &range, sema->ranges()) {
        lastLine = qMax(lastLine, range.line);
        if (range.format == GoHighlightRange::Other || range.format == GoHighlightRange::Error)
            continue;

        TextEditor::HighlightingResult hr;
        hr.kind = range.format;
        hr.line = range.line;
        hr.column = range.column;
        hr.length = range.length;
        results.append(hr);
    }
    m_highlightFuture.reportResults(results);
    m_highlightFuture.reportFinished();
    m_semanticFuture.reportResult(sema);
    m_semanticFuture.reportFinished();
}

} // namespace Internal
} // namespace GoEditor
