#ifndef GOLANG_INTERNAL_GOPARSER_H
#define GOLANG_INTERNAL_GOPARSER_H

#include <projectexplorer/ioutputparser.h>
#include <projectexplorer/task.h>

namespace GoLang {

class GoProject;

namespace Internal {

class GoParser : public ProjectExplorer::IOutputParser
{
    Q_OBJECT
public:
    GoParser();

    void stdError(const QString &line) override;
    void stdOutput(const QString &line) override;

protected:
    void doFlush() override;

private:
    GoProject *findCurrentProject() const;
    void parseErrorLine(const QString &line);

    QRegExp m_regexpError;
    ProjectExplorer::Task m_currentTask;
};

} // namespace Internal
} // namespace GoLang

#endif // GOLANG_INTERNAL_GOPARSER_H
