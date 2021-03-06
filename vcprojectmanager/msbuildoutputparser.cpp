#include "msbuildoutputparser.h"
#include <projectexplorer/projectexplorerconstants.h>

namespace VcProjectManager {
namespace Internal {

static inline
QRegExp regexp(const char text[])
{
    return QRegExp(QLatin1String(text));
}

MsBuildOutputParser::MsBuildOutputParser()
    : m_counter(0),
      m_buildAttempFinished(false)
{
    setObjectName(QLatin1String("MsBuildParser"));
    m_buildStartTimeRegExp = regexp("(?:Build\\sstarted\\s)(\\d+/\\d+/\\d+)(?:\\s)(\\d+:\\d+:\\d+)(?:\\s)(PM|AM)(?:.*)");
    m_compileWarningRegExp = regexp("(.*)\\((\\d+)\\)(?::\\s)(warning\\s(?:[A-Z]|[a-z]|\\d)+)(?::\\s)(.*)");
    m_compileErrorRegExp = regexp("(.*)\\((\\d+)\\)(?::\\s)(error\\s(?:[A-Z]|[a-z]|\\d)+)(?::\\s)(.*)");
    m_doneTargetBuildRegExp = regexp("Done\\sbuilding\\starget\\s\"(.*)\"\\sin\\sproject\\s\"(.*)\"(\\s--\\sFAILED)?\\.");
    m_doneProjectBuildRegExp = regexp("Done\\sbuilding\\sproject\\s\"(.*)\"((?:\\s--\\s)FAILED)?\\.");
    m_buildSucceededRegExp = regexp("Build succeeded\\.");
    m_buildFailedRegExp = regexp("Build FAILED\\.");
    m_buildTimeElapsedRegExp = regexp("Time\\sElapsed\\s(\\d+:\\d+:\\d+(?:\\.\\d+)?)");
    m_msBuildErrorRegExp = regexp("MSBUILD\\s:\\s(error\\s(?:[A-Z]|[a-z]|\\d)+):\\s(.*)");
}

void MsBuildOutputParser::stdOutput(const QString &line)
{
    if (m_buildStartTimeRegExp.indexIn(line) != -1) {
        emit addTask(ProjectExplorer::Task(ProjectExplorer::Task::Unknown,
                                           m_buildStartTimeRegExp.cap(0).trimmed(),
                                           Utils::FileName() /* filename */,
                                           -1 /* linenumber */,
                                           Core::Id(ProjectExplorer::Constants::TASK_CATEGORY_BUILDSYSTEM)));
    }

    bool warningBeforeError = true;

    // if line contains both ': warning ' and ': error ' check if warning comes after error
    if (line.contains(QLatin1String(": warning ")) && line.contains(QLatin1String(": error "))) {
        if (line.indexOf(QLatin1String(": error ")) < line.indexOf(QLatin1String(": warning ")))
            warningBeforeError = false;
    }
    // else, use warningBeforeError as a sign that line doesn't contain no warning
    else if (line.contains(QLatin1String(": error ")))
        warningBeforeError = false;

    if (!m_buildAttempFinished && warningBeforeError && line.contains(QLatin1String(": warning "))) {
        QString leftover = line;
        leftover = leftover.trimmed();
        QStringList splits = leftover.split(QLatin1String(": warning "));
        // get file path
        QString filePath = splits.at(0).trimmed();
        leftover = splits.at(1);

        // check if the file path contains line number, example D:\blabla\gggg.cpp(55)
        QRegExp warningLineRegExp = regexp(".*\\((\\d+)\\)$");
        if (warningLineRegExp.exactMatch(filePath)) {
            filePath.resize(filePath.length() - warningLineRegExp.cap(1).length() - 2);
        }

        // get warning code
        int splitIndex = leftover.indexOf(QLatin1String(": "));
        QString warningCode = leftover.left(splitIndex).trimmed();
        leftover.remove(0, splitIndex);
        leftover.remove(0, 2); // warning description remains
        leftover = leftover.trimmed();

        QString description(warningCode
                            + QLatin1String(" ")
                            + leftover);
        // if line where warning has originated is present
        if (warningLineRegExp.captureCount() == 2 && !warningLineRegExp.cap(1).isEmpty())
            description.append(QLatin1String(" line: ")
                               + warningLineRegExp.cap(1));

        emit addTask(ProjectExplorer::Task(ProjectExplorer::Task::Warning,
                                           description,
                                           Utils::FileName::fromUserInput(filePath),
                                           warningLineRegExp.cap(1).toInt(),
                                           Core::Id(ProjectExplorer::Constants::TASK_CATEGORY_BUILDSYSTEM)));
    }

    if (!m_buildAttempFinished && !warningBeforeError && line.contains(QLatin1String(": error "))) {
        QString leftover = line;
        leftover = leftover.trimmed();
        QStringList splits = leftover.split(QLatin1String(": error "));
        // get file path
        QString filePath = splits.at(0).trimmed();
        leftover = splits.at(1);

        // check if the file path contains line number, example D:\blabla\gggg.cpp(55)
        QRegExp errorLineRegExp = regexp(".*\\((\\d+)\\)$");
        if (errorLineRegExp.exactMatch(filePath)) {
            filePath.resize(filePath.length() - errorLineRegExp.cap(1).length() - 2);
        }

        // get error code
        int splitIndex = leftover.indexOf(QLatin1String(": "));
        QString errorCode = leftover.left(splitIndex).trimmed();
        leftover.remove(0, splitIndex);
        leftover.remove(0, 2); // error description remains
        leftover = leftover.trimmed();

        QString description(errorCode
                            + QLatin1String(" ")
                            + leftover);
        // if line where error has originated is present
        if (errorLineRegExp.captureCount() == 2 && !errorLineRegExp.cap(1).isEmpty())
            description.append(QLatin1String(" line: ")
                               + errorLineRegExp.cap(1));

        emit addTask(ProjectExplorer::Task(ProjectExplorer::Task::Error,
                                           description,
                                           Utils::FileName::fromUserInput(filePath),
                                           errorLineRegExp.cap(1).toInt(),
                                           Core::Id(ProjectExplorer::Constants::TASK_CATEGORY_BUILDSYSTEM)));
    }

    if (m_doneTargetBuildRegExp.indexIn(line) != -1)
        emit addTask(ProjectExplorer::Task(ProjectExplorer::Task::Unknown,
                                           m_doneTargetBuildRegExp.cap(0).trimmed(),
                                           Utils::FileName() /* filename */,
                                           -1 /* linenumber */,
                                           Core::Id(ProjectExplorer::Constants::TASK_CATEGORY_BUILDSYSTEM)));

    if (m_doneProjectBuildRegExp.indexIn(line) != -1)
        emit addTask(ProjectExplorer::Task(ProjectExplorer::Task::Unknown,
                                           m_doneProjectBuildRegExp.cap(0).trimmed(),
                                           Utils::FileName() /* filename */,
                                           -1 /* linenumber */,
                                           Core::Id(ProjectExplorer::Constants::TASK_CATEGORY_BUILDSYSTEM)));

    if (m_buildSucceededRegExp.indexIn(line) != -1) {
        emit addTask(ProjectExplorer::Task(ProjectExplorer::Task::Unknown,
                                           QLatin1String("Build Succeeded."),
                                           Utils::FileName(),
                                           -1,
                                           Core::Id(ProjectExplorer::Constants::TASK_CATEGORY_BUILDSYSTEM)));
        m_buildAttempFinished = true;
    }

    if (m_buildFailedRegExp.indexIn(line) != -1) {
        emit addTask(ProjectExplorer::Task(ProjectExplorer::Task::Unknown,
                                           QLatin1String("Build FAILED!"),
                                           Utils::FileName(),
                                           -1,
                                           Core::Id(ProjectExplorer::Constants::TASK_CATEGORY_BUILDSYSTEM)));
        m_buildAttempFinished = true;
    }

    if (m_buildTimeElapsedRegExp.indexIn(line) != -1) {
        QString description(QLatin1String("Build lasted for: ")
                            + m_buildTimeElapsedRegExp.cap(1));
        emit addTask(ProjectExplorer::Task(ProjectExplorer::Task::Unknown,
                                           description,
                                           Utils::FileName(),
                                           -1,
                                           Core::Id(ProjectExplorer::Constants::TASK_CATEGORY_BUILDSYSTEM)));
    }

    if (m_msBuildErrorRegExp.indexIn(line) != -1) {
        QString description(QLatin1String("MSBuild ")
                            + m_msBuildErrorRegExp.cap(1)
                            + QLatin1String(": ")
                            + m_msBuildErrorRegExp.cap(2));
        emit addTask(ProjectExplorer::Task(ProjectExplorer::Task::Error,
                                           description,
                                           Utils::FileName(),
                                           -1,
                                           Core::Id(ProjectExplorer::Constants::TASK_CATEGORY_BUILDSYSTEM)));
    }

    ++m_counter;
}

} // namespace Internal
} // namespace VcProjectManager
