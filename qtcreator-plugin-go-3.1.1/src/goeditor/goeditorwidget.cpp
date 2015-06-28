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
