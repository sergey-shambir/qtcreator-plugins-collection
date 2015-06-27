#ifndef GOEDITOR_INTERNAL_HIGHLIGHTTASK_H
#define GOEDITOR_INTERNAL_HIGHLIGHTTASK_H

#include <texteditor/semantichighlighter.h>
#include <utils/runextensions.h>
#include <QFutureInterface>
#include <QSharedPointer>

namespace GoEditor {

class GoSemanticInfo;
class GoEditorDocument;

namespace Internal {

class SingleShotHighlightTask :
        public QRunnable
{
public:
    struct Result
    {
        QFuture<TextEditor::HighlightingResult> highlightFuture;
        QFuture<QSharedPointer<GoSemanticInfo>> semanticFuture;
    };

    SingleShotHighlightTask();
    ~SingleShotHighlightTask();
    void setFilename(const QString &filename);
    void setText(const QByteArray &text);

    Result start();
    void run();

private:
    QFutureInterface<TextEditor::HighlightingResult> m_highlightFuture;
    QFutureInterface<QSharedPointer<GoSemanticInfo>> m_semanticFuture;
    QString m_filename;
    QByteArray m_text;
};

} // namespace Internal
} // namespace GoEditor

#endif // GOEDITOR_INTERNAL_HIGHLIGHTTASK_H
