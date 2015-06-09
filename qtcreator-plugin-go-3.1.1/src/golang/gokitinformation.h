#ifndef GOLANG_GOKITINFORMATION_H
#define GOLANG_GOKITINFORMATION_H

#include <projectexplorer/kitinformation.h>
#include <projectexplorer/kitconfigwidget.h>
#include <QComboBox>
#include <QPushButton>

namespace GoLang {

class ToolChain;

class GoKitMatcher : public ProjectExplorer::KitMatcher
{
public:
    GoKitMatcher()
    {    }

    bool matches(const ProjectExplorer::Kit *k) const override;
};

class GoToolChainKitInformation : public ProjectExplorer::KitInformation
{
    Q_OBJECT
public:
    GoToolChainKitInformation();

    // KitInformation interface
    virtual QVariant defaultValue(ProjectExplorer::Kit *) const override;
    virtual QList<ProjectExplorer::Task> validate(const ProjectExplorer::Kit *kit) const override;
    virtual void fix(ProjectExplorer::Kit *) override;
    virtual void setup(ProjectExplorer::Kit *) override;
    virtual ItemList toUserOutput(const ProjectExplorer::Kit *) const override;
    virtual ProjectExplorer::KitConfigWidget *createConfigWidget(ProjectExplorer::Kit *) const override;
    virtual void addToEnvironment(const ProjectExplorer::Kit *k, Utils::Environment &env) const override;
    virtual ProjectExplorer::IOutputParser *createOutputParser(const ProjectExplorer::Kit *k) const override;
    virtual QString displayNamePostfix(const ProjectExplorer::Kit *k) const override;
    static Core::Id id();
    static ToolChain *toolChain(const ProjectExplorer::Kit *k);
    static void setToolChain(ProjectExplorer::Kit *k, ToolChain *tc);
protected slots:
    void kitsWereLoaded();
    void toolChainUpdated(GoLang::ToolChain *tc);
    void toolChainRemoved(GoLang::ToolChain *tc);
};

namespace Internal {

// --------------------------------------------------------------------------
// ToolChainInformationConfigWidget:
// --------------------------------------------------------------------------

class ToolChainInformationConfigWidget : public ProjectExplorer::KitConfigWidget
{
    Q_OBJECT

public:
    ToolChainInformationConfigWidget(ProjectExplorer::Kit *k, const ProjectExplorer::KitInformation *ki);
    ~ToolChainInformationConfigWidget();

    QString displayName() const override;
    void refresh() override;
    void makeReadOnly() override;
    QWidget *mainWidget() const override;
    QWidget *buttonWidget() const override;
    QString toolTip() const override;

private slots:
    void toolChainAdded(GoLang::ToolChain *tc);
    void toolChainRemoved(GoLang::ToolChain *tc);
    void toolChainUpdated(GoLang::ToolChain *tc);
    void manageToolChains();
    void currentToolChainChanged(int idx);

private:
    void updateComboBox();
    int indexOf(const ToolChain *tc);

    QComboBox *m_comboBox;
    QPushButton *m_manageButton;
};

} // namespace Internal
} // namespace GoLang

#endif // GOLANG_GOKITINFORMATION_H
