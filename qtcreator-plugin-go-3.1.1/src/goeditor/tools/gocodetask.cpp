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
static QLatin1String WARN_UNKNOWN_OUTLINE_KIND("Unknown outline kind '%1'.");
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

QSharedPointer<GoSemanticInfo> GocodeTask::highlight()
{
    m_command = GOSEMKI_COMMAND;
    QStringList arguments;
    if (!m_isInMemory)
        arguments << QString::fromLatin1("--in=%1").arg(m_filePath);
    arguments << QLatin1String("highlight");
    if (m_isInMemory)
        arguments << QFileInfo(m_filePath).fileName();

    QSharedPointer<GoSemanticInfo> ret;
    QByteArray response;
    if (runGocode(arguments, response)) {
        parseJsonHighlightFormat(response);
        m_sema->sort();
        ret.swap(m_sema);
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
    m_outlineKindsMap[QLatin1String("fun")] = GoOutlineItem::Func;
    m_outlineKindsMap[QLatin1String("typ")] = GoOutlineItem::Struct;
    m_formatsMap[QLatin1String("fun")] = GoHighlightRange::Func;
    m_formatsMap[QLatin1String("lbl")] = GoHighlightRange::Label;
    m_formatsMap[QLatin1String("typ")] = GoHighlightRange::Type;
    m_formatsMap[QLatin1String("var")] = GoHighlightRange::Var;
    m_formatsMap[QLatin1String("con")] = GoHighlightRange::Const;
    m_formatsMap[QLatin1String("pkg")] = GoHighlightRange::Package;
    m_formatsMap[QLatin1String("fld")] = GoHighlightRange::Field;
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
    const QString KEY_MESSAGE(QLatin1String("msg"));
    const QString KEY_FROM(QLatin1String("from"));
    const QString KEY_TO(QLatin1String("to"));
    const QString KEY_TITLE_STRING(QLatin1String("str"));
    const QString KEY_RANGES(QLatin1String("Ranges"));
    const QString KEY_OUTLINE(QLatin1String("Outline"));
    const QString KEY_ERRORS(QLatin1String("Errors"));
    const QString KEY_FOLDS(QLatin1String("Folds"));
    m_sema.reset(new GoSemanticInfo);

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(response, &error);
    if (error.error != QJsonParseError::NoError) {
        reportError(QString(ERROR_JSON_PARSING_FAILED).arg(error.errorString()));
        return;
    }
    QJsonObject root = doc.object();
    foreach (QJsonValue resultValue, root.value(KEY_RANGES).toArray()) {
        QJsonObject result = resultValue.toObject();
        GoHighlightRange range;
        range.format = formatFromString(result.value(KEY_KIND).toString());
        range.line = result.value(KEY_LINE).toInt();
        range.column = result.value(KEY_COLUMN).toInt();
        range.length = result.value(KEY_LENGTH).toInt();
        m_sema->addRange(range);
    }
    foreach (QJsonValue value, root.value(KEY_OUTLINE).toArray()) {
        QJsonObject result = value.toObject();
        GoOutlineItem item;
        item.kind = outlineKindFromString(result.value(KEY_KIND).toString());
        item.line = result.value(KEY_LINE).toInt();
        item.column = result.value(KEY_COLUMN).toInt();
        item.title = result.value(KEY_TITLE_STRING).toString();
        m_sema->addOutlineItem(item);
    }
    foreach (QJsonValue resultValue, root.value(KEY_ERRORS).toArray()) {
        QJsonObject result = resultValue.toObject();
        GoError err;
        err.message = result.value(KEY_MESSAGE).toString();
        err.line = result.value(KEY_LINE).toInt();
        err.column = result.value(KEY_COLUMN).toInt();
        err.length = result.value(KEY_LENGTH).toInt();
        m_sema->addError(err);
    }
    foreach (QJsonValue value, root.value(KEY_FOLDS).toArray()) {
        QJsonObject result = value.toObject();
        GoFoldArea area;
        area.lineFrom = result.value(KEY_FROM).toInt();
        area.lineTo = result.value(KEY_TO).toInt();
        m_sema->addFoldArea(area);
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

GoHighlightRange::Format GocodeTask::formatFromString(const QString &string) const
{
    auto it = m_formatsMap.find(string);
    if (it == m_formatsMap.end()) {
        reportError(QString(WARN_UNKNOWN_FORMAT).arg(string));
        return GoHighlightRange::Other;
    }
    return it.value();
}

GoOutlineItem::Kind GocodeTask::outlineKindFromString(const QString &string) const
{
    auto it = m_outlineKindsMap.find(string);
    if (it == m_outlineKindsMap.end()) {
        reportError(QString(WARN_UNKNOWN_OUTLINE_KIND).arg(string));
        return GoOutlineItem::Func;
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
