#ifndef GOLANG_RUNCONFIGURATION_H
#define GOLANG_RUNCONFIGURATION_H

#include <projectexplorer/localapplicationrunconfiguration.h>

namespace Utils {
    class PathChooser;
    class DetailsWidget;
}

namespace GoLang {

class GoProject;
class GoRunConfiguration;

class GoRunConfigurationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GoRunConfigurationWidget(GoRunConfiguration *goRc, QWidget *parent = 0);

private slots:
    void setArguments(const QString &args);
    void setWorkingDirectory();
    void resetWorkingDirectory();
    void runInTerminalToggled(bool toggled);
    void environmentWasChanged();

    void workingDirectoryChanged();
    void onRcEnabledChanged();

private:
    void ctor();
    bool m_ignoreChange;
    GoRunConfiguration *m_goRc;
    Utils::PathChooser *m_workingDirectoryEdit;
    Utils::DetailsWidget *m_detailsContainer;
};

class GoRunConfiguration : public ProjectExplorer::LocalApplicationRunConfiguration
{
    Q_OBJECT
public:
    GoRunConfiguration(ProjectExplorer::Target *t, const Core::Id &id);
    GoRunConfiguration(ProjectExplorer::Target *t, GoRunConfiguration *other);

    GoProject *project () const;
    void setCommandName (const QString &cmdName);
    void setWorkingDirectory (const QString &dir);
    void setRunMode (RunMode mode);
    void setCommandLineArguments (const QString &args);

    // LocalApplicationRunConfiguration interface
    virtual QString executable() const override;
    virtual RunMode runMode() const override;
    virtual QString workingDirectory() const override;
    virtual QString commandLineArguments() const override;
    virtual void addToBaseEnvironment(Utils::Environment &env) const override;

    // RunConfiguration interface
    virtual QWidget *createConfigurationWidget() override;

    // ProjectConfiguration interface
    virtual bool fromMap(const QVariantMap &map) override;
    virtual QVariantMap toMap() const override;

signals:
    void workingDirectoryChanged ();

private:
    QString m_defaultWorkingDirectory;
    QString m_userWorkingDirectory;
    QString m_commandName;
    QString m_args;
    RunMode m_runMode;
};

} // namespace GoLang

#endif // GOLANG_RUNCONFIGURATION_H
