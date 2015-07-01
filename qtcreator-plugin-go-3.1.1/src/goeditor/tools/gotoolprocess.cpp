#include "gotoolprocess.h"
#include <QProcess>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

namespace GoEditor {

static QLatin1String GOEDITOR_ERROR_PREFIX("GoEditor: %1 failed. ");
static QLatin1String ERROR_TIMEOUT_EXCEEED("Waiting timeout exceed.");
static QLatin1String ERROR_IN_PROCESS("Return code '%2', process stderr:\n%3");
static const int COMPLETION_WAIT_TIME_MSEC = 3000;

GoToolProcess::GoToolProcess(const QString &filePath)
    : m_isFileInMemory(false)
    , m_filePath(filePath)
{
}

GoToolProcess::GoToolProcess(const QString &filePath, const QByteArray &contents)
    : m_isFileInMemory(true)
    , m_filePath(filePath)
    , m_fileContent(contents)
{
}

bool GoToolProcess::isFileInMemory() const
{
    return m_isFileInMemory;
}

QString GoToolProcess::toolCommand() const
{
    return m_toolCommand;
}

void GoToolProcess::setToolCommand(const QString &toolCommand)
{
    m_toolCommand = toolCommand;
}

void GoToolProcess::reportError(const QString &text) const
{
    // Don't use Core::MessageManager - it causes crash when reportError() from completion assistant.
    QString prefix(GOEDITOR_ERROR_PREFIX);
    qDebug() << prefix.arg(m_toolCommand) + text;
}

bool GoToolProcess::runTool(const QStringList &arguments, QByteArray &response)
{
    QProcess tool;
    tool.setWorkingDirectory(QFileInfo(m_filePath).dir().absolutePath());
    tool.start(m_toolCommand, arguments);
    if (m_isFileInMemory) {
        tool.write(m_fileContent);
        tool.closeWriteChannel();
    }
    if (!tool.waitForFinished(COMPLETION_WAIT_TIME_MSEC)) {
        reportError(ERROR_TIMEOUT_EXCEEED);
        return false;
    }
    if (tool.exitCode() != 0) {
        reportError(QString(ERROR_IN_PROCESS).arg(QString::number(tool.exitCode()),
                                                      QString::fromUtf8(tool.readAllStandardError())));
        return false;
    }
    response = tool.readAllStandardOutput();
    return true;
}

QByteArray GoToolProcess::fileContent() const
{
    return m_fileContent;
}

void GoToolProcess::setFileContent(const QByteArray &fileContent)
{
    m_isFileInMemory = true;
    m_fileContent = fileContent;
}

QString GoToolProcess::filePath() const
{
    return m_filePath;
}

void GoToolProcess::setFilePath(const QString &filePath)
{
    m_filePath = filePath;
}

} // namespace GoEditor
