#ifndef GOLANG_PLUGIN_H
#define GOLANG_PLUGIN_H

#include "golang_global.h"

#include <extensionsystem/iplugin.h>

namespace GoLang {
namespace Internal {

class GoLangPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "GoLang.json")

public:
    GoLangPlugin();
    ~GoLangPlugin();

    bool initialize(const QStringList &arguments, QString *errorString) override;
    void extensionsInitialized() override;
    ShutdownFlag aboutToShutdown() override;

private slots:
    void restoreToolChains();
};

} // namespace Internal
} // namespace Go

#endif // GOLANG_PLUGIN_H

