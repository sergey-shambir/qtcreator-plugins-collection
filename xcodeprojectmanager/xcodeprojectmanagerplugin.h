#ifndef XCODEPROJECTMANAGER_H
#define XCODEPROJECTMANAGER_H

#include "xcodeprojectmanager_global.h"

#include <extensionsystem/iplugin.h>

namespace XCodeProjectManager {
namespace Internal {

class XCodeProjectManagerPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "XCodeProjectManager.json")
    
public:
    XCodeProjectManagerPlugin();
    ~XCodeProjectManagerPlugin();
    
    bool initialize(const QStringList &arguments, QString *errorString);
    void extensionsInitialized();
    ShutdownFlag aboutToShutdown();
    
private slots:
    void triggerAction();
};

} // namespace Internal
} // namespace XCodeProjectManager

#endif // XCODEPROJECTMANAGER_H

