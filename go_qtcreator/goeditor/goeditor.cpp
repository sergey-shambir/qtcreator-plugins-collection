#include "goeditor.h"
#include "goeditorconstants.h"
#include "goeditorplugin.h"
#include "goeditorwidget.h"
#include "tools/gocompletionassist.h"

#include <coreplugin/icore.h>
#include <coreplugin/mimedatabase.h>
#include <extensionsystem/pluginmanager.h>
#include <texteditor/texteditorconstants.h>
#include <texteditor/texteditorsettings.h>

#include <QFileInfo>

namespace GoEditor {
namespace Internal {

GoEditor::GoEditor(GoEditorWidget *editorWidget) :
    TextEditor::BaseTextEditor(editorWidget)
{
    setId(Constants::C_GOEDITOR_ID);
    setContext(Core::Context(Constants::C_GOEDITOR_ID,
                             TextEditor::Constants::C_TEXTEDITOR));
}

GoEditor::~GoEditor()
{
}

Core::IEditor *GoEditor::duplicate()
{
    auto widget = new GoEditorWidget(qobject_cast<GoEditorWidget *>(editorWidget()));
    TextEditor::TextEditorSettings::initializeEditor(widget);
    return widget->editor();
}

TextEditor::CompletionAssistProvider *GoEditor::completionAssistProvider()
{
    return ExtensionSystem::PluginManager::getObject<GoCompletionAssistProvider>();
}

bool GoEditor::open(QString *errorString,
                    const QString &fileName,
                    const QString &realFileName)
{
    Core::MimeType mimeType = Core::MimeDatabase::findByFile(QFileInfo(fileName));
    baseTextDocument()->setMimeType(mimeType.type());
    return TextEditor::BaseTextEditor::open(errorString, fileName, realFileName);
}

} // namespace Internal
} // namespace GoEditor
