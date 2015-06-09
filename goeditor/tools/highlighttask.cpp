#include "highlighttask.h"
#include "gocodetask.h"

namespace GoEditor {
namespace Internal {

static bool _lesserRangePositionPredicate(const HighlightRange &a, const HighlightRange &b)
{
    return (a.line < b.line) || (a.line == b.line && a.column < b.column);
}

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

QFuture<TextEditor::HighlightingResult> SingleShotHighlightTask::start()
{
    this->setRunnable(this);
    this->reportStarted();
    QFuture<TextEditor::HighlightingResult> future = this->future();
    QThreadPool::globalInstance()->start(this, QThread::LowestPriority);
    return future;
}

void SingleShotHighlightTask::run()
{
    runHelper();
    this->reportFinished();
}

void SingleShotHighlightTask::runHelper()
{
    GocodeTask task(m_filename, m_text);
    QList<HighlightRange> ranges = task.highlight();
    qSort(ranges.begin(), ranges.end(), _lesserRangePositionPredicate);
    int lastLine = 0;
    QVector<TextEditor::HighlightingResult> results;
    foreach (const HighlightRange &range, ranges) {
        lastLine = qMax(lastLine, range.line);
        if (range.format == HighlightRange::Other || range.format == HighlightRange::Error)
            continue;

        TextEditor::HighlightingResult hr;
        hr.kind = range.format;
        hr.line = range.line;
        hr.column = range.column;
        hr.length = range.length;
        results.append(hr);
    }
    this->reportResults(results);
}

} // namespace Internal
} // namespace GoEditor
