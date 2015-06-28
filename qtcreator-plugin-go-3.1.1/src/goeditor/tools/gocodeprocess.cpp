#include "gocodeprocess.h"
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

namespace GoEditor {

static QLatin1String WARN_PANIC_CLASS("Daemon panic() happened.");
static QLatin1String WARN_UNKNOWN_CLASS("Unknown completion class '%1'.");
static QLatin1String ERROR_JSON_PARSING_FAILED("Cannot parse JSON response. JSON error: %2");
static QLatin1String PANIC_COMPLETION_CLASS("PANIC");
static QLatin1String GOCODE_COMMAND("gocode");

GocodeProcess::GocodeProcess(const QString &filePath)
    : GoToolProcess(filePath)
{
    initStringMaps();
}

GocodeProcess::GocodeProcess(const QString &filePath, const QByteArray &contents)
    : GoToolProcess(filePath, contents)
{
    initStringMaps();
}

QList<CodeCompletion> GocodeProcess::codeCompleteAt(quint64 offset)
{
    setToolCommand(GOCODE_COMMAND);
    QStringList arguments;
    arguments << QLatin1String("-f=json");
    if (!isFileInMemory())
        arguments << QString::fromLatin1("--in=%1").arg(filePath());
    arguments << QLatin1String("autocomplete");
    arguments << QFileInfo(filePath()).fileName();
    arguments << QString::number(offset);

    QList<CodeCompletion> ret;
    QByteArray response;
    if (runTool(arguments, response)) {
        parseJson(response);
        ret.swap(m_completions);
    }
    return ret;
}

void GocodeProcess::initStringMaps()
{
    m_kindsMap[QLatin1String("func")] = CodeCompletion::Func;
    m_kindsMap[QLatin1String("package")] = CodeCompletion::Package;
    m_kindsMap[QLatin1String("var")] = CodeCompletion::Variable;
    m_kindsMap[QLatin1String("type")] = CodeCompletion::Type;
    m_kindsMap[QLatin1String("const")] = CodeCompletion::Const;
}

void GocodeProcess::parseJson(const QByteArray &response)
{
    m_completions.clear();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(response, &error);
    if (error.error != QJsonParseError::NoError) {
        reportError(QString(ERROR_JSON_PARSING_FAILED).arg(error.errorString()));
        return;
    }
    QJsonArray items = doc.array();
    if (items.size() != 2) {
        return;
    }
    QJsonArray results = items.at(1).toArray();
    foreach (QJsonValue resultValue, results) {
        QJsonObject result = resultValue.toObject();
        QString kind = result.value(QLatin1String("class")).toString();
        CodeCompletion completion;
        completion.kind = kindFromString(kind);
        completion.text = result.value(QLatin1String("name")).toString();
        completion.hint = result.value(QLatin1String("type")).toString();
        if (completion.hint.startsWith(kind)) {
            completion.hint = kind + QLatin1String(" ") + completion.text
                    + completion.hint.mid(kind.size());
        }

        if (!completion.text.isEmpty())
            m_completions.append(completion);
    }
}

CodeCompletion::Kind GocodeProcess::kindFromString(const QString &string) const
{
    auto it = m_kindsMap.find(string);
    if (it == m_kindsMap.end()) {
        if (string == PANIC_COMPLETION_CLASS) {
            reportError(WARN_PANIC_CLASS);
        } else {
            reportError(QString(WARN_UNKNOWN_CLASS).arg(string));
        }
        return CodeCompletion::Other;
    }
    return it.value();
}

} // namespace GoEditor
