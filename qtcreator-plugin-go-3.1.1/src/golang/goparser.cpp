#include "goparser.h"
#include "goproject.h"
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/session.h>
#include <utils/qtcassert.h>

namespace GoLang {
namespace Internal {

using ProjectExplorer::Task;

GoParser::GoParser()
{
    m_regexpError.setPattern(QLatin1String("^([^:]+):([\\d]+):[\\d]+:"));
    m_regexpError.setMinimal(true);
    QTC_CHECK(m_regexpError.isValid());
}

void GoParser::stdError(const QString &line)
{
    parseErrorLine(line);
}

void GoParser::stdOutput(const QString &line)
{
    Q_UNUSED(line)
}

void GoParser::doFlush()
{
    if (m_currentTask.isNull())
        return;
    Task t = m_currentTask;
    m_currentTask.clear();
    emit addTask(t);
}

GoProject *GoParser::findCurrentProject() const
{
    return qobject_cast<GoProject *>(ProjectExplorer::SessionManager::startupProject());
}

void GoParser::parseErrorLine(const QString &line)
{
    if (-1 != m_regexpError.indexIn(line)) {
        const int lineNo = m_regexpError.cap(2).toInt();
        const QString description = line;
        QString filepath = m_regexpError.cap(1);
        if (GoProject *project = findCurrentProject())
            filepath = project->projectDir().absoluteFilePath(filepath);

        doFlush();
        m_currentTask = Task(Task::Error, description, Utils::FileName::fromString(filepath),
                             lineNo, ProjectExplorer::Constants::TASK_CATEGORY_COMPILE);
    } else if (line.startsWith(QLatin1Char('\t')) && !m_currentTask.isNull()) {
        m_currentTask.description.append(line.mid(1));
    }
}

} // namespace Internal
} // namespace GoLang
