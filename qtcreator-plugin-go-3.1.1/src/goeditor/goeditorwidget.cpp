#include "goeditorwidget.h"
#include "goeditor.h"
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

static const int UPDATE_HIGHLIGHTS_INTERVAL_MSEC = 250;

namespace GoEditor {
namespace Internal {

GoEditorWidget::GoEditorWidget(QWidget *parent) :
    TextEditor::BaseTextEditorWidget(parent),
    m_semaHighlighterRevision(0),
    m_semaHighlightsUpdater(nullptr)
{
    baseTextDocument()->setIndenter(new GoIndenter());
    ctor();
}

GoEditorWidget::GoEditorWidget(GoEditorWidget *other) :
    TextEditor::BaseTextEditorWidget(other),
    m_semaHighlighterRevision(0),
    m_semaHighlightsUpdater(nullptr)
{
    ctor();

//    m_semaHighlightsUpdater = new QTimer(this);
//    m_semaHighlightsUpdater->setSingleShot(true);
//    m_semaHighlightsUpdater->setInterval(UPDATE_HIGHLIGHTS_INTERVAL_MSEC);
//    connect(m_semaHighlightsUpdater, SIGNAL(timeout()),
//            this, SLOT(updateSemaHighlightsNow()));
//    connect(this, SIGNAL(cursorPositionChanged()),
//            this, SLOT(deferUpdateSemaHighlights()));
//    connect(this, SIGNAL(textChanged()),
//            this, SLOT(deferUpdateSemaHighlights()));
//    connect(&m_semaHighlightWatcher, SIGNAL(resultsReadyAt(int,int)),
//            this, SLOT(acceptSemaHighlights(int,int)));
//    connect(&m_semaHighlightWatcher, SIGNAL(finished()),
//            this, SLOT(finishSemaHighlights()));
}

GoEditorWidget::~GoEditorWidget()
{
}

void GoEditorWidget::unCommentSelection()
{
    Utils::unCommentSelection(this, m_commentDefinition);
}

// TODO: how to update font settings now?
//void GoEditorWidget::setFontSettings(const TextEditor::FontSettings &fs)
//{
//    TextEditor::BaseTextEditorWidget::setFontSettings(fs);

//    GoHighlighter *highlighter = qobject_cast<GoHighlighter *>(baseTextDocument()->syntaxHighlighter());
//    if (highlighter)
//        highlighter->setFontSettings(fs);

//    m_semaHighlightFormatMap[HighlightRange::Type] =
//            fs.toTextCharFormat(TextEditor::C_TYPE);
//    m_semaHighlightFormatMap[HighlightRange::Var] =
//            fs.toTextCharFormat(TextEditor::C_LOCAL);
//    m_semaHighlightFormatMap[HighlightRange::Field] =
//            fs.toTextCharFormat(TextEditor::C_FIELD);
//    m_semaHighlightFormatMap[HighlightRange::Const] =
//            fs.toTextCharFormat(TextEditor::C_ENUMERATION);
//    m_semaHighlightFormatMap[HighlightRange::Label] =
//            fs.toTextCharFormat(TextEditor::C_LABEL);
//    m_semaHighlightFormatMap[HighlightRange::Func] =
//            fs.toTextCharFormat(TextEditor::C_FUNCTION);
//    m_semaHighlightFormatMap[HighlightRange::Package] =
//            fs.toTextCharFormat(TextEditor::C_TYPE);

//    updateSemaHighlightsNow();
//}

TextEditor::BaseTextEditor *GoEditorWidget::createEditor()
{
    return new GoEditor(this);
}

void GoEditorWidget::deferUpdateSemaHighlights()
{
    m_semaHighlightsUpdater->start();
}

void GoEditorWidget::updateSemaHighlightsNow()
{
    m_semaHighlighter.cancel();
    m_semaHighlighterRevision = document()->revision();
    SingleShotHighlightTask *task = new SingleShotHighlightTask;
    task->setFilename(QFileInfo(baseTextDocument()->filePath()).fileName());
    task->setText(editor()->textDocument()->plainText().toUtf8());
    m_semaHighlighter = task->start();
    m_semaHighlightWatcher.setFuture(m_semaHighlighter);
}

void GoEditorWidget::acceptSemaHighlights(int from, int to)
{
    if (m_semaHighlighterRevision != document()->revision() || m_semaHighlighter.isCanceled())
            return; // aborted

    TextEditor::SyntaxHighlighter *highlighter = baseTextDocument()->syntaxHighlighter();
    TextEditor::SemanticHighlighter::incrementalApplyExtraAdditionalFormats(
                highlighter,
                m_semaHighlighter,
                from,
                to,
                m_semaHighlightFormatMap
                );
}

void GoEditorWidget::finishSemaHighlights()
{
    if (m_semaHighlighterRevision != document()->revision() || m_semaHighlighter.isCanceled())
            return; // aborted

    TextEditor::SyntaxHighlighter *highlighter = baseTextDocument()->syntaxHighlighter();
    TextEditor::SemanticHighlighter::clearExtraAdditionalFormatsUntilEnd(
                highlighter,
                m_semaHighlighter
                );
}

void GoEditorWidget::ctor()
{
    m_commentDefinition.multiLineStart = QLatin1String("/*");
    m_commentDefinition.multiLineEnd = QLatin1String("*/");
    m_commentDefinition.singleLine = QLatin1String("//");

    setParenthesesMatchingEnabled(true);
    setMarksVisible(true);
    setCodeFoldingSupported(true);

    new GoHighlighter(baseTextDocument());
}

} // namespace Internal
} // namespace GoEditor
