#include "gofmtprocess.h"
#include <QFileInfo>

namespace GoEditor {

static QLatin1String GOFMT_COMMAND("gofmt");

GofmtProcess::GofmtProcess(const QString &filePath)
    : GoToolProcess(filePath)
{
}

GofmtProcess::GofmtProcess(const QString &filePath, const QByteArray &contents)
    : GoToolProcess(filePath, contents)
{
}

bool GofmtProcess::runFormatting()
{
    setToolCommand(GOFMT_COMMAND);
    QStringList arguments;
    arguments << QLatin1String("-w=true");
    arguments << QFileInfo(filePath()).fileName();
    QByteArray ignored;
    return runTool(arguments, ignored);
}

} // namespace GoEditor
