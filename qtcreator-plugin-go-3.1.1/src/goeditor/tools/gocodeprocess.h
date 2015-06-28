#ifndef GOEDITOR_GOCODEPROCESS_H
#define GOEDITOR_GOCODEPROCESS_H

#include "gotoolprocess.h"
#include <QMap>

namespace GoEditor {

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

class GocodeProcess : public GoToolProcess
{
public:
    explicit GocodeProcess(const QString &filePath);
    explicit GocodeProcess(const QString &filePath, const QByteArray &contents);

    QList<CodeCompletion> codeCompleteAt(quint64 offset);

private:
    void initStringMaps();
    void parseJson(const QByteArray &response);
    CodeCompletion::Kind kindFromString(const QString &string) const;

    QMap<QString, CodeCompletion::Kind> m_kindsMap;
    QList<CodeCompletion> m_completions;
};

} // namespace GoEditor

#endif // GOEDITOR_GOCODEPROCESS_H
