#ifndef GOEDITOR_GOTOOLPROCESS_H
#define GOEDITOR_GOTOOLPROCESS_H

#include <QStringList>
#include <QByteArray>

namespace GoEditor {

class GoToolProcess
{
protected:
    explicit GoToolProcess(const QString &filePath);
    explicit GoToolProcess(const QString &filePath, const QByteArray &contents);

    QString filePath() const;
    void setFilePath(const QString &filePath);
    QByteArray fileContent() const;
    void setFileContent(const QByteArray &fileContent);
    bool isFileInMemory() const;
    QString toolCommand() const;
    void setToolCommand(const QString &toolCommand);
    void reportError(const QString &text) const;
    bool runTool(const QStringList &arguments, QByteArray &response);

private:
    bool m_isFileInMemory;
    QString m_filePath;
    QString m_toolCommand;
    QByteArray m_fileContent;
};

} // namespace GoEditor

#endif // GOEDITOR_GOTOOLPROCESS_H
