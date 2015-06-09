#include "goeditorfactory.h"
#include "goeditorconstants.h"
#include "goeditorwidget.h"
#include "goeditorplugin.h"
#include "goeditor.h"

#include <coreplugin/icore.h>
#include <coreplugin/editormanager/editormanager.h>
#include <texteditor/texteditorsettings.h>
#include <texteditor/texteditoractionhandler.h>

namespace GoEditor {
namespace Internal {

using TextEditor::TextEditorActionHandler;

GoEditorFactory::GoEditorFactory(QObject *parent) :
    Core::IEditorFactory(parent)
{
    setId(Constants::C_GOEDITOR_ID);
    setDisplayName(tr(Constants::EN_EDITOR_DISPLAY_NAME));
    addMimeType(QLatin1String(Constants::C_GO_MIMETYPE));
    new TextEditorActionHandler(this, Constants::C_GOEDITOR_ID,
                                TextEditorActionHandler::Format
                                | TextEditorActionHandler::UnCommentSelection
                                | TextEditorActionHandler::UnCollapseAll);
}

Core::IEditor *GoEditorFactory::createEditor()
{
    auto widget = new GoEditorWidget();
    TextEditor::TextEditorSettings::initializeEditor(widget);
    return widget->editor();
}

} // namespace Internal
} // namespace GoEditor
