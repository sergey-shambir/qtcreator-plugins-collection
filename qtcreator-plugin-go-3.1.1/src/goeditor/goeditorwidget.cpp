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

#include "goeditorwidget.h"
#include "goeditor.h"
#include "goeditordocument.h"
#include "tools/goindenter.h"
#include "tools/gohighlighter.h"
#include "tools/highlighttask.h"

#include <texteditor/fontsettings.h>
#include <texteditor/texteditorconstants.h>
#include <texteditor/basetextdocument.h>
#include <texteditor/indenter.h>
#include <texteditor/autocompleter.h>
#include <QTimer>
#include <QtConcurrent>

namespace GoEditor {
namespace Internal {

GoEditorWidget::GoEditorWidget(QWidget *parent) :
    TextEditor::BaseTextEditorWidget(new GoEditorDocument(), parent)
{
    ctor();
}

GoEditorWidget::GoEditorWidget(GoEditorWidget *other) :
    TextEditor::BaseTextEditorWidget(other)
{
    ctor();
}

GoEditorWidget::~GoEditorWidget()
{
}

void GoEditorWidget::unCommentSelection()
{
    Utils::unCommentSelection(this, m_commentDefinition);
}

TextEditor::BaseTextEditor *GoEditorWidget::createEditor()
{
    return new GoEditor(this);
}

void GoEditorWidget::applySemantic(const GoSemanticInfoPtr &semantic)
{
    if (!semantic.isNull())
        semantic->displayDiagnostic(this);
}

void GoEditorWidget::ctor()
{
    m_commentDefinition.multiLineStart = QLatin1String("/*");
    m_commentDefinition.multiLineEnd = QLatin1String("*/");
    m_commentDefinition.singleLine = QLatin1String("//");

    setParenthesesMatchingEnabled(true);
    setMarksVisible(true);
    setCodeFoldingSupported(true);

    connect(this, SIGNAL(textChanged()),
            baseTextDocument(), SLOT(deferSemanticUpdate()));
    connect(baseTextDocument(), SIGNAL(semanticUpdated(GoSemanticInfoPtr)),
            this, SLOT(applySemantic(GoSemanticInfoPtr)));
}

} // namespace Internal
} // namespace GoEditor
