#ifndef GOLANG_INTERNAL_TOOLCHAIN_H
#define GOLANG_INTERNAL_TOOLCHAIN_H

#include "toolchain.h"
#include <projectexplorer/kit.h>
#include <projectexplorer/task.h>
#include <projectexplorer/abi.h>
#include <projectexplorer/headerpath.h>

namespace GoLang {

namespace Internal {
class GoLangPlugin;
class ToolChainPrivate;
}

class ToolChainFactory;
class ToolChainConfigWidget;

class ToolChain
{
public:
    enum Detection {
        ManualDetection,
        AutoDetection,
        AutoDetectionFromSettings
    };

    QString displayName() const;
    void setDisplayName(const QString &name);

    inline bool isAutoDetected() const { return detection() != ManualDetection; }
    ToolChain::Detection detection() const;
    QString id() const;

    virtual QString type() const = 0;
    virtual QString typeDisplayName() const = 0;
    virtual ProjectExplorer::Abi targetAbi() const = 0;
    virtual bool isValid() const = 0;
    virtual QList<ProjectExplorer::HeaderPath> systemGoPaths( ) const = 0;
    virtual void addToEnvironment(Utils::Environment &env) const = 0;
    virtual Utils::FileName compilerCommand() const = 0;
    virtual ProjectExplorer::IOutputParser *outputParser() const = 0;
    virtual ToolChainConfigWidget *configurationWidget() = 0;
    virtual bool canClone() const;
    virtual ToolChain *clone() const = 0;
    virtual QVariantMap toMap() const;
    virtual QList<ProjectExplorer::Task> validateKit(const ProjectExplorer::Kit *k) const;

    virtual bool operator ==(const ToolChain &tc) const;

    virtual ~ToolChain();
protected:
    explicit ToolChain(const QString &id, Detection d);
    explicit ToolChain(const ToolChain &other);
    virtual bool fromMap(const QVariantMap &data);
    void toolChainUpdated();

private:
    void setDetection(Detection detect);

    Internal::ToolChainPrivate *const d;

    friend class ToolChainManager;
    friend class ToolChainFactory;
};

class ToolChainFactory : public QObject
{
    Q_OBJECT
public:
    Core::Id id() const { return m_id; }
    QString displayName() const { return m_displayName; }

    virtual QList<ToolChain *> autoDetect() = 0;

    virtual bool canCreate() = 0;
    virtual GoLang::ToolChain *create() = 0;
    virtual bool canRestore(const QVariantMap &data) = 0;
    virtual GoLang::ToolChain *restore(const QVariantMap &data) = 0;

    static QString idFromMap(const QVariantMap &data);
    static void idToMap(QVariantMap &data, const QString id);
    static void autoDetectionToMap(QVariantMap &data, bool detected);
protected:
    void setId(Core::Id id) { m_id = id; }
    void setDisplayName(const QString &name) { m_displayName = name; }

private:
    QString m_displayName;
    Core::Id m_id;
};

} // namespace GoLang

#endif // GOLANG_INTERNAL_TOOLCHAIN_H
