#ifndef GOEDITOR_INTERNAL_HIGHLIGHTTASK_H
#define GOEDITOR_INTERNAL_HIGHLIGHTTASK_H

#include <texteditor/semantichighlighter.h>
#include <utils/runextensions.h>
#include <QFutureInterface>
#include <QSharedPointer>

namespace GoEditor {
namespace Internal {

class SingleShotHighlightTask :
        public QRunnable,
        public QFutureInterface<TextEditor::HighlightingResult>
{
public:
    SingleShotHighlightTask();
    ~SingleShotHighlightTask();
    void setFilename(const QString &filename);
    void setText(const QByteArray &text);

    QFuture<TextEditor::HighlightingResult> start();
    void run();

private:
    void runHelper();

    QString m_filename;
    QByteArray m_text;
};

} // namespace Internal
} // namespace GoEditor

#endif // GOEDITOR_INTERNAL_HIGHLIGHTTASK_H
