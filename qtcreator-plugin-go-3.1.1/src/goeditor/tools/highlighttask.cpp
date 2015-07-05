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
    if (sema.isNull()) {
        m_highlightFuture.reportFinished();
        m_semanticFuture.reportFinished();
        return;
    }

    int lastLine = 0;
    QVector<TextEditor::HighlightingResult> results;
    foreach (const GoHighlightRange &range, sema->ranges()) {
        lastLine = qMax(lastLine, range.line);
        if (range.format == GoHighlightRange::Other)
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
