#ifndef GOEDITOR_H
#define GOEDITOR_H

#include "goeditor_global.h"

#include <extensionsystem/iplugin.h>
#include <texteditor/texteditoractionhandler.h>
#include <QScopedPointer>
#include <QSet>

namespace GoEditor {
namespace Internal {

class GoEditorFactory;
class GoEditorWidget;

class GoEditorPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "GoEditor.json")

public:
    GoEditorPlugin();
    ~GoEditorPlugin();

    bool initialize(const QStringList &arguments, QString *errorString) override;
    void extensionsInitialized() override;
    ShutdownFlag aboutToShutdown() override;

    static QSet<QString> goKeywords();
    static QSet<QString> goPredeclaratedTypes();
    static QSet<QString> goPredeclaratedConsts();
    static QSet<QString> goPredeclaratedFuncs();

private:
    static GoEditorPlugin *m_instance;
    GoEditorFactory *m_factory;
    QScopedPointer<TextEditor::TextEditorActionHandler> m_actionHandler;
    QSet<QString> m_goKeywords;
    QSet<QString> m_goPredeclaratedTypes;
    QSet<QString> m_goPredeclaratedConsts;
    QSet<QString> m_goPredeclaratedFuncs;
};

} // namespace Internal
} // namespace GoEditor

#endif // GOEDITOR_H

