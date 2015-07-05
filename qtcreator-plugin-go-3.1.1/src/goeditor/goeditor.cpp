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
