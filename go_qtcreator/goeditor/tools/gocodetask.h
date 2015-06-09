#ifndef GOCODETASK_H
#define GOCODETASK_H

#include <texteditor/semantichighlighter.h>
#include <QObject>
#include <QRunnable>
#include <QStringList>
#include <QMap>

namespace GoEditor {
namespace Internal {

struct CodeCompletion
{
public:
    enum Kind {
        Func,
        Package,
        Variable,
        Type,
        Const,
        Other
    };

    Kind kind;
    QString text;
    QString hint;
};

struct HighlightRange {
    enum Format {
        Error,
        Field,
        Func,
        Label,
        Type,
        Var,
        Const,
        Package,
        Other
    };

    Format format;
    int line;
    int column;
    int length;
};

class GocodeTask : public QObject
{
    Q_OBJECT
public:
    GocodeTask(const QString &filePath);
    GocodeTask(const QString &filePath, const QByteArray &contents);

    QList<CodeCompletion> codeCompleteAt(quint64 offset);
    QList<HighlightRange> highlight();

private:
    void initStringMaps();
    void parseVimFormat(const QByteArray &response);
    void parseJsonHighlightFormat(const QByteArray &response);
    void parseJsonCompletionFormat(const QByteArray &response);

    HighlightRange::Format formatFromString(const QString &string) const;
    CodeCompletion::Kind kindFromString(const QString &string) const;

    bool m_isInMemory;
    QString m_filePath;
    QByteArray m_fileContent;
    QList<CodeCompletion> m_completions;
    QList<HighlightRange> m_ranges;
    QMap<QString, CodeCompletion::Kind> m_kindsMap;
    QMap<QString, HighlightRange::Format> m_formatsMap;
};

} // namespace Internal
} // namespace GoEditor

#endif // GOCODETASK_H
