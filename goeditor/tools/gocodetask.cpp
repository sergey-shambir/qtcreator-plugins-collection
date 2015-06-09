#include "gocodetask.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QProcess>
#include <QFileInfo>
#include <QDir>

#include <QDebug>

static const char ERROR_TIMEOUT_EXCEEED[] = "Go lang code completion failed, daemon waiting timeout exceed.";
static const char ERROR_DAEMON_STOPPED[] = "Go lang code completion failed, daemon process returned non-zero code, see error string and stderr:";
static const char ERROR_VIM_PARSING_FAILED[] = "Go lang code completion failed, we cannot parse VIM-style response. JSON error:";
static const char ERROR_JSON_PARSING_FAILED[] = "Go lang code completion failed, we cannot parse JSON response. JSON error:";
static const char ERROR_JSON_INCORRECT[] = "Go lang code completion failed, incorrect VIM-style JSON responce.";
static const char WARN_UNKNOWN_CLASS[] = "Go lang code completion warning: unknown completion class found,";
static const char WARN_UNKNOWN_FORMAT[] = "Go lang highlighting warning: unknown range format found,";
static const char GOCODE_COMMAND[] = "gocode";
static const int COMPLETION_WAIT_TIME_MSEC = 3000;

namespace GoEditor {
namespace Internal {

/// @class GocodeTask
/// @brief Calls 'gocode' daemon to get autocompletion at given file and position.

GocodeTask::GocodeTask(const QString &filePath)
    : m_isInMemory(false)
    , m_filePath(filePath)
{
    initStringMaps();
}

GocodeTask::GocodeTask(const QString &filePath, const QByteArray &contents)
    : m_isInMemory(true)
    , m_filePath(filePath)
    , m_fileContent(contents)
{
    initStringMaps();
}

QList<CodeCompletion> GocodeTask::codeCompleteAt(quint64 offset)
{
    QStringList arguments;
    arguments << QLatin1String("-f=json");
    if (!m_isInMemory)
        arguments << QString::fromLatin1("--in=%1").arg(m_filePath);
    arguments << QLatin1String("autocomplete");
    arguments << QFileInfo(m_filePath).fileName();
    arguments << QString::number(offset);

    QProcess gocode;
    gocode.setWorkingDirectory(QFileInfo(m_filePath).dir().absolutePath());
    gocode.start(QString::fromUtf8(GOCODE_COMMAND), arguments);
    if (m_isInMemory) {
        gocode.write(m_fileContent);
        gocode.closeWriteChannel();
    }
    if (!gocode.waitForFinished(COMPLETION_WAIT_TIME_MSEC)) {
        qDebug() << ERROR_TIMEOUT_EXCEEED;
        return QList<CodeCompletion>();
    }
    if (gocode.exitCode() != 0) {
        qDebug() << ERROR_DAEMON_STOPPED << gocode.errorString() << gocode.readAllStandardError();
        return QList<CodeCompletion>();
    }
    parseJsonCompletionFormat(gocode.readAllStandardOutput());

    QList<CodeCompletion> ret;
    ret.swap(m_completions);
    return ret;
}

QList<HighlightRange> GocodeTask::highlight()
{
    QStringList arguments;
    arguments << QLatin1String("-f=qtjson");
    if (!m_isInMemory)
        arguments << QString::fromLatin1("--in=%1").arg(m_filePath);
    arguments << QLatin1String("highlight");
    arguments << QFileInfo(m_filePath).fileName();

    QProcess gocode;
    gocode.setWorkingDirectory(QFileInfo(m_filePath).dir().absolutePath());
    gocode.start(QString::fromUtf8(GOCODE_COMMAND), arguments);
    if (m_isInMemory) {
        gocode.write(m_fileContent);
        gocode.closeWriteChannel();
    }
    if (!gocode.waitForFinished(COMPLETION_WAIT_TIME_MSEC)) {
        qDebug() << ERROR_TIMEOUT_EXCEEED;
        return QList<HighlightRange>();
    }
    if (gocode.exitCode() != 0) {
        qDebug() << ERROR_DAEMON_STOPPED << gocode.errorString() << gocode.readAllStandardError();
        return QList<HighlightRange>();
    }
    parseJsonHighlightFormat(gocode.readAllStandardOutput());

    QList<HighlightRange> ret;
    ret.swap(m_ranges);
    return ret;
}

void GocodeTask::initStringMaps()
{
    m_kindsMap[QLatin1String("func")] = CodeCompletion::Func;
    m_kindsMap[QLatin1String("package")] = CodeCompletion::Package;
    m_kindsMap[QLatin1String("var")] = CodeCompletion::Variable;
    m_kindsMap[QLatin1String("type")] = CodeCompletion::Type;
    m_kindsMap[QLatin1String("const")] = CodeCompletion::Const;
    m_formatsMap[QLatin1String("error")] = HighlightRange::Error;
    m_formatsMap[QLatin1String("field")] = HighlightRange::Field;
    m_formatsMap[QLatin1String("func")] = HighlightRange::Func;
    m_formatsMap[QLatin1String("label")] = HighlightRange::Label;
    m_formatsMap[QLatin1String("type")] = HighlightRange::Type;
    m_formatsMap[QLatin1String("var")] = HighlightRange::Var;
    m_formatsMap[QLatin1String("const")] = HighlightRange::Const;
    m_formatsMap[QLatin1String("package")] = HighlightRange::Package;
}

void GocodeTask::parseVimFormat(const QByteArray &response)
{
    m_completions.clear();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(response, &error);
    if (error.error != QJsonParseError::NoError) {
        qDebug() << ERROR_VIM_PARSING_FAILED << error.errorString() << response;
        return;
    }
    QJsonArray items = doc.array();
    if (items.size() != 2) {
        return;
    }
    QJsonArray results = items.at(1).toArray();
    foreach (QJsonValue resultValue, results) {
        QJsonObject result = resultValue.toObject();
        CodeCompletion completion;
        completion.kind = CodeCompletion::Other;
        completion.text = result.value(QLatin1String("word")).toString();
        completion.hint = result.value(QLatin1String("info")).toString();
        if (!completion.text.isEmpty() && !completion.hint.isEmpty())
            m_completions.append(completion);
    }
}

void GocodeTask::parseJsonHighlightFormat(const QByteArray &response)
{
    m_ranges.clear();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(response, &error);
    if (error.error != QJsonParseError::NoError) {
        qDebug() << ERROR_JSON_PARSING_FAILED << error.errorString() << response;
        return;
    }
    QJsonArray items = doc.array();
    foreach (QJsonValue resultValue, items) {
        QJsonObject result = resultValue.toObject();
        QString format = result.value(QLatin1String("format")).toString();
        HighlightRange range;
        range.format = formatFromString(format);
        range.line = result.value(QLatin1String("line")).toString().toInt();
        range.column = result.value(QLatin1String("column")).toString().toInt();
        range.length = result.value(QLatin1String("length")).toString().toInt();
        m_ranges.append(range);
    }
}

void GocodeTask::parseJsonCompletionFormat(const QByteArray &response)
{
    m_completions.clear();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(response, &error);
    if (error.error != QJsonParseError::NoError) {
        qDebug() << ERROR_JSON_PARSING_FAILED << error.errorString() << response;
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

HighlightRange::Format GocodeTask::formatFromString(const QString &string) const
{
    QMap<QString, HighlightRange::Format>::const_iterator it = m_formatsMap.find(string);
    if (it == m_formatsMap.end()) {
        qDebug() << WARN_UNKNOWN_FORMAT << string;
        return HighlightRange::Other;
    }
    return it.value();
}

CodeCompletion::Kind GocodeTask::kindFromString(const QString &string) const
{
    QMap<QString, CodeCompletion::Kind>::const_iterator it = m_kindsMap.find(string);
    if (it == m_kindsMap.end()) {
        qDebug() << WARN_UNKNOWN_CLASS << string;
        return CodeCompletion::Other;
    }
    return it.value();
}

} // namespace Internal
} // namespace GoEditor
