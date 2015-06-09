#include "gocodetask.h"
#include <coreplugin/messagemanager.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QProcess>
#include <QFileInfo>
#include <QDir>

static QLatin1String ERROR_TIMEOUT_EXCEEED("GoEditor: code completion failed, daemon waiting timeout exceed.");
static QLatin1String ERROR_DAEMON_STOPPED("GoEditor: code completion failed with non-zero return code, see error and stderr: %1; %2");
static QLatin1String ERROR_VIM_PARSING_FAILED("GoEditor: code completion failed, cannot parse VIM-style response. JSON error: %1");
static QLatin1String ERROR_JSON_PARSING_FAILED("GoEditor: code completion failed, cannot parse JSON response. JSON error: %1");
static QLatin1String ERROR_JSON_INCORRECT("GoEditor: code completion failed, incorrect VIM-style JSON responce.");
static QLatin1String WARN_PANIC_CLASS("GoEditor: panic() in gocode occured");
static QLatin1String WARN_UNKNOWN_CLASS("GoEditor: found unknown completion class ");
static QLatin1String WARN_UNKNOWN_FORMAT("GoEditor: found unknown highlight range format ");
static QLatin1String PANIC_COMPLETION_CLASS("PANIC");
static const char GOCODE_COMMAND[] = "gocode";
static const char GOSEMKI_COMMAND[] = "gosemki";
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

    QList<CodeCompletion> ret;
    QByteArray response;
    if (runGocode(arguments, response)) {
        parseJsonCompletionFormat(response);
        ret.swap(m_completions);
    }
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

    QList<HighlightRange> ret;
    QByteArray response;
    if (runGocode(arguments, response)) {
        parseJsonHighlightFormat(response);
        ret.swap(m_ranges);
    }
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
        reportError(QString(ERROR_VIM_PARSING_FAILED).arg(error.errorString()));
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
        reportError(QString(ERROR_JSON_PARSING_FAILED).arg(error.errorString()));
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

void GocodeTask::reportError(const QString &text) const
{
    Core::MessageManager::write(text);
}

bool GocodeTask::runGocode(const QStringList &arguments, QByteArray &response)
{
    QProcess gocode;
    gocode.setWorkingDirectory(QFileInfo(m_filePath).dir().absolutePath());
    gocode.start(QString::fromUtf8(GOCODE_COMMAND), arguments);
    if (m_isInMemory) {
        gocode.write(m_fileContent);
        gocode.closeWriteChannel();
    }
    if (!gocode.waitForFinished(COMPLETION_WAIT_TIME_MSEC)) {
        reportError(ERROR_TIMEOUT_EXCEEED);
        return false;
    }
    if (gocode.exitCode() != 0) {
        reportError(QString(ERROR_DAEMON_STOPPED).arg(gocode.errorString(),
                                                      QString::fromUtf8(gocode.readAllStandardError())));
        return false;
    }
    response = gocode.readAllStandardOutput();
    return true;
}

HighlightRange::Format GocodeTask::formatFromString(const QString &string) const
{
    auto it = m_formatsMap.find(string);
    if (it == m_formatsMap.end()) {
        reportError(WARN_UNKNOWN_CLASS + string);
        return HighlightRange::Other;
    }
    return it.value();
}

CodeCompletion::Kind GocodeTask::kindFromString(const QString &string) const
{
    auto it = m_kindsMap.find(string);
    if (it == m_kindsMap.end()) {
        if (string == PANIC_COMPLETION_CLASS) {
            reportError(WARN_PANIC_CLASS);
        } else {
            reportError(WARN_UNKNOWN_CLASS + string);
        }
        return CodeCompletion::Other;
    }
    return it.value();
}

} // namespace Internal
} // namespace GoEditor
