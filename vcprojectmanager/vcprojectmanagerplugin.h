#ifndef VCPROJECTMANAGERPLUGIN_H
#define VCPROJECTMANAGERPLUGIN_H

#include "vcprojectmanager_global.h"

#include <extensionsystem/iplugin.h>

namespace VcProjectManager {
namespace Internal {

class VcProjectManagerPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "VcProjectManagerPlugin.json")

public:
    VcProjectManagerPlugin();
    ~VcProjectManagerPlugin();

    bool initialize(const QStringList &arguments, QString *errorString);
    void extensionsInitialized();
    ShutdownFlag aboutToShutdown();
};

} // namespace Internal
} // namespace VcProjectManager

#endif // VCPROJECTMANAGERPLUGIN_H

