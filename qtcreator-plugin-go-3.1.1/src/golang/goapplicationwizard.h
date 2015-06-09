#ifndef GOLANG_GOAPPLICATIONWIZARD_H
#define GOLANG_GOAPPLICATIONWIZARD_H

#include <projectexplorer/baseprojectwizarddialog.h>
#include <projectexplorer/customwizard/customwizard.h>
#include <projectexplorer/targetsetuppage.h>

QT_BEGIN_NAMESPACE
class QLabel;
QT_END_NAMESPACE

namespace Utils {
class PathChooser;
}

namespace GoLang {

class GoApplicationWizard : public ProjectExplorer::CustomProjectWizard
{
    Q_OBJECT

public:
    GoApplicationWizard();

private:
    QWizard *createWizardDialog(QWidget *parent,
                                const Core::WizardDialogParameters &wizardDialogParameters) const override;
    bool postGenerateFiles(const QWizard *, const Core::GeneratedFiles &l, QString *errorMessage) override;

private:
    enum { targetPageId = 1 };
};

class GoApplicationWizardDialog : public ProjectExplorer::BaseProjectWizardDialog
{
    Q_OBJECT
public:
    explicit GoApplicationWizardDialog(QWidget *parent, const Core::WizardDialogParameters &parameters);
    virtual ~GoApplicationWizardDialog();

    int addTargetSetupPage(int id = -1);

    QList<Core::Id> selectedKits() const;
    bool writeUserFile(const QString &projectFileName) const;
private slots:
    void generateProfileName(const QString &name, const QString &path);
private:
    ProjectExplorer::TargetSetupPage *m_targetSetupPage;
    void init();
};


} // namespace GoLang

#endif // GOLANG_GOAPPLICATIONWIZARD_H
