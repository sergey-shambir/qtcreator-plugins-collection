#ifndef GOLANG_INTERNAL_GOTOOLCHAIN_H
#define GOLANG_INTERNAL_GOTOOLCHAIN_H

#include "toolchain.h"

namespace GoLang {
namespace Internal {

class GoToolChain : public ToolChain
{
public:
    GoToolChain(ToolChain::Detection detect);

    void setCompilerCommand(const Utils::FileName &path, const Utils::FileName &goRoot);
    QString version() const;
    void setTargetAbi(const ProjectExplorer::Abi &abi);
    QList<ProjectExplorer::Abi> supportedAbis () const;
    virtual QString defaultDisplayName() const;

    virtual Utils::FileName goRoot() const;

    // ToolChain interface
    virtual QString type() const override;
    virtual QString typeDisplayName() const override;
    virtual ProjectExplorer::Abi targetAbi() const override;
    virtual bool isValid() const override;
    virtual QList<ProjectExplorer::HeaderPath> systemGoPaths() const override;
    virtual void addToEnvironment(Utils::Environment &env) const override;
    virtual Utils::FileName compilerCommand() const override;
    virtual ProjectExplorer::IOutputParser *outputParser() const override;
    virtual ToolChainConfigWidget *configurationWidget() override;
    virtual bool canClone() const override;
    virtual ToolChain *clone() const override;
    virtual QVariantMap toMap() const override;
    virtual QList<ProjectExplorer::Task> validateKit(const ProjectExplorer::Kit *k) const override;

    static void addCommandPathToEnvironment(const Utils::FileName &command, Utils::Environment &env);
    static QString toString(ProjectExplorer::Abi::Architecture arch, int width);
protected:
    GoToolChain(const GoToolChain &other);
    virtual bool fromMap(const QVariantMap &data) override;
    virtual QList<ProjectExplorer::Abi> detectSupportedAbis() const;

    void updateSupportedAbis() const;
private:
    Utils::FileName m_compilerCommand;
    Utils::FileName m_goRoot;
    ProjectExplorer::Abi m_targetAbi;
    mutable QList<ProjectExplorer::HeaderPath> m_headerPaths;
    mutable QList<ProjectExplorer::Abi> m_supportedAbis;

    friend class GoToolChainFactory;
};


class GoToolChainFactory : public ToolChainFactory
{
    Q_OBJECT
public:
    GoToolChainFactory ();

    virtual QList<ToolChain *> autoDetect() override;
    virtual bool canCreate() override;
    virtual GoLang::ToolChain *create() override;
    virtual bool canRestore(const QVariantMap &data) override;
    virtual GoLang::ToolChain *restore(const QVariantMap &data) override;
protected:
    virtual GoToolChain *createToolChain(bool autoDetect);
};

} // namespace Internal
} // namespace GoLang

#endif // GOLANG_INTERNAL_GOTOOLCHAIN_H
