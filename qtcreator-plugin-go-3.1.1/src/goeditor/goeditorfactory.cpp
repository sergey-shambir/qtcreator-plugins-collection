/*******************************************************************************
The MIT License (MIT)

Copyright (c) 2015 Sergey Shambir

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*******************************************************************************/

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
