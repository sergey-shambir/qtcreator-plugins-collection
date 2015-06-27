#ifndef GOEDITOR_GOCODETASK_H
#define GOEDITOR_GOCODETASK_H

#include "gosemanticinfo.h"
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

class GocodeTask : public QObject
{
    Q_OBJECT
public:
    GocodeTask(const QString &filePath);
    GocodeTask(const QString &filePath, const QByteArray &contents);

    QList<CodeCompletion> codeCompleteAt(quint64 offset);
    QSharedPointer<GoSemanticInfo> highlight();

private:
    void initStringMaps();
    void parseVimFormat(const QByteArray &response);
    void parseJsonHighlightFormat(const QByteArray &response);
    void parseJsonCompletionFormat(const QByteArray &response);
    void reportError(const QString &text) const;
    bool runGocode(const QStringList &arguments, QByteArray &response);

    GoHighlightRange::Format formatFromString(const QString &string) const;
    GoOutlineItem::Kind outlineKindFromString(const QString &string) const;
    CodeCompletion::Kind kindFromString(const QString &string) const;

    bool m_isInMemory;
    QString m_filePath;
    QString m_command;
    QByteArray m_fileContent;
    QList<CodeCompletion> m_completions;
    QSharedPointer<GoSemanticInfo> m_sema;
    QMap<QString, CodeCompletion::Kind> m_kindsMap;
    QMap<QString, GoOutlineItem::Kind> m_outlineKindsMap;
    QMap<QString, GoHighlightRange::Format> m_formatsMap;
};

} // namespace Internal
} // namespace GoEditor

#endif // GOEDITOR_GOCODETASK_H
