#include "gocodetask.h"
#include <coreplugin/messagemanager.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QProcess>
#include <QFileInfo>
#include <QDir>

static QLatin1String GOEDITOR_ERROR_PREFIX("GoEditor: %1 failed. ");
static QLatin1String ERROR_TIMEOUT_EXCEEED("Daemon waiting timeout exceed.");
static QLatin1String ERROR_DAEMON_STOPPED("Return code '%2', process stderr:\n%3");
static QLatin1String ERROR_VIM_PARSING_FAILED("Cannot parse VIM-style response. JSON error: %2");
static QLatin1String ERROR_JSON_PARSING_FAILED("Cannot parse JSON response. JSON error: %2");
static QLatin1String ERROR_JSON_INCORRECT("Incorrect VIM-style JSON responce.");
static QLatin1String WARN_PANIC_CLASS("Daemon panic() happened.");
static QLatin1String WARN_UNKNOWN_CLASS("Unknown completion class '%1'.");
static QLatin1String WARN_UNKNOWN_FORMAT("Unknown highlight range format '%1'.");
static QLatin1String PANIC_COMPLETION_CLASS("PANIC");
static QLatin1String GOCODE_COMMAND("gocode");
static QLatin1String GOSEMKI_COMMAND("gosemki");
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
    m_command = GOCODE_COMMAND;
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
    m_command = GOSEMKI_COMMAND;
    QStringList arguments;
    if (!m_isInMemory)
        arguments << QString::fromLatin1("--in=%1").arg(m_filePath);
    arguments << QLatin1String("highlight");
    if (m_isInMemory)
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
    m_formatsMap[QLatin1String("fun")] = HighlightRange::Func;
    m_formatsMap[QLatin1String("lbl")] = HighlightRange::Label;
    m_formatsMap[QLatin1String("typ")] = HighlightRange::Type;
    m_formatsMap[QLatin1String("var")] = HighlightRange::Var;
    m_formatsMap[QLatin1String("con")] = HighlightRange::Const;
    m_formatsMap[QLatin1String("pkg")] = HighlightRange::Package;
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
    const QString KEY_LINE(QLatin1String("lin"));
    const QString KEY_COLUMN(QLatin1String("col"));
    const QString KEY_LENGTH(QLatin1String("len"));
    const QString KEY_KIND(QLatin1String("knd"));
    m_ranges.clear();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(response, &error);
    if (error.error != QJsonParseError::NoError) {
        reportError(QString(ERROR_JSON_PARSING_FAILED).arg(error.errorString()));
        return;
    }
    QJsonObject root = doc.object();
    foreach (QJsonValue resultValue, root.value(QLatin1String("Ranges")).toArray()) {
        QJsonObject result = resultValue.toObject();
        QByteArray jsonStr = QJsonDocument(result).toJson();
        qDebug() << jsonStr;
        HighlightRange range;
        range.format = formatFromString(result.value(QLatin1String("knd")).toString());
        range.line = result.value(KEY_LINE).toInt();
        range.column = result.value(KEY_COLUMN).toInt();
        range.length = result.value(KEY_LENGTH).toInt();
        m_ranges.append(range);
    }
    foreach (QJsonValue resultValue, root.value(QLatin1String("Errors")).toArray()) {
        QJsonObject result = resultValue.toObject();
        HighlightRange range;
        range.format = HighlightRange::Error;
        range.line = result.value(KEY_LINE).toInt();
        range.column = result.value(KEY_COLUMN).toInt();
        range.length = result.value(KEY_LENGTH).toInt();
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
    QString prefix(GOEDITOR_ERROR_PREFIX);
    Core::MessageManager::write(prefix.arg(m_command) + text);
}

// m_command should be set before this call
bool GocodeTask::runGocode(const QStringList &arguments, QByteArray &response)
{
    QProcess gocode;
    gocode.setWorkingDirectory(QFileInfo(m_filePath).dir().absolutePath());
    gocode.start(m_command, arguments);
    if (m_isInMemory) {
        gocode.write(m_fileContent);
        gocode.closeWriteChannel();
    }
    if (!gocode.waitForFinished(COMPLETION_WAIT_TIME_MSEC)) {
        reportError(ERROR_TIMEOUT_EXCEEED);
        return false;
    }
    if (gocode.exitCode() != 0) {
        reportError(QString(ERROR_DAEMON_STOPPED).arg(QString::number(gocode.exitCode()),
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
        reportError(QString(WARN_UNKNOWN_CLASS).arg(string));
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
            reportError(QString(WARN_UNKNOWN_CLASS).arg(string));
        }
        return CodeCompletion::Other;
    }
    return it.value();
}

} // namespace Internal
} // namespace GoEditor
