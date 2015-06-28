#include "goeditordocument.h"
#include "tools/gohighlighter.h"
#include "tools/goindenter.h"
#include "tools/highlighttask.h"
#include "tools/gofmtprocess.h"
#include <texteditor/tabsettings.h>
#include <coreplugin/messagemanager.h>
#include <QTimer>
#include <QTextDocument>

static const int UPDATE_HIGHLIGHTS_INTERVAL_MSEC = 250;

using namespace ::GoEditor::Internal;
using TextEditor::TabSettings;

namespace GoEditor {

GoEditorDocument::GoEditorDocument()
{
    setSyntaxHighlighter(new GoHighlighter(document()));
    setIndenter(new GoIndenter);

    connect(&m_indexerWatcher, SIGNAL(resultsReadyAt(int,int)),
            this, SLOT(acceptSemaHighlights(int,int)));
    connect(&m_indexerWatcher, SIGNAL(finished()),
            this, SLOT(finishSemaHighlights()));
    connect(&m_semanticWatcher, SIGNAL(resultsReadyAt(int,int)),
            this, SLOT(acceptSemantic(int,int)));
    connect(this, SIGNAL(tabSettingsChanged()), this, SLOT(fixTabSettings()));

    m_semaHighlightsUpdater = new QTimer(this);
    m_semaHighlightsUpdater->setSingleShot(true);
    m_semaHighlightsUpdater->setInterval(UPDATE_HIGHLIGHTS_INTERVAL_MSEC);
    connect(m_semaHighlightsUpdater, SIGNAL(timeout()),
            this, SLOT(updateSemaHighlightsNow()));
}

GoEditorDocument::~GoEditorDocument()
{
}

bool GoEditorDocument::save(QString *errorString, const QString &fileName, bool autoSave)
{
    if (!BaseTextDocument::save(errorString, fileName, autoSave))
        return false;

    GofmtProcess process(filePath());
    QString reloadError;
    if (process.runFormatting() && !reload(&reloadError)) {
        QLatin1String prefix("GoEditor: reload formatted code failed. ");
        Core::MessageManager::write(prefix + reloadError);
    }

    return true;
}

void GoEditorDocument::deferSemanticUpdate()
{
    m_semaHighlightsUpdater->start();
}

void GoEditorDocument::applyFontSettings()
{
    BaseTextDocument::applyFontSettings();

    const TextEditor::FontSettings &fs = fontSettings();
    m_highlightFormatMap[GoHighlightRange::Type] =
            fs.toTextCharFormat(TextEditor::C_TYPE);
    m_highlightFormatMap[GoHighlightRange::Var] =
            fs.toTextCharFormat(TextEditor::C_LOCAL);
    m_highlightFormatMap[GoHighlightRange::Field] =
            fs.toTextCharFormat(TextEditor::C_FIELD);
    m_highlightFormatMap[GoHighlightRange::Const] =
            fs.toTextCharFormat(TextEditor::C_ENUMERATION);
    m_highlightFormatMap[GoHighlightRange::Label] =
            fs.toTextCharFormat(TextEditor::C_LABEL);
    m_highlightFormatMap[GoHighlightRange::Func] =
            fs.toTextCharFormat(TextEditor::C_VIRTUAL_METHOD);
    m_highlightFormatMap[GoHighlightRange::Package] =
            fs.toTextCharFormat(TextEditor::C_STRING);

    updateSemaHighlightsNow();
}

void GoEditorDocument::triggerPendingUpdates()
{
    TextEditor::BaseTextDocument::triggerPendingUpdates();
    m_semaHighlightsUpdater->start();
}

void GoEditorDocument::updateSemaHighlightsNow()
{
    m_indexRevision = document()->revision();
    auto task = new SingleShotHighlightTask;
    task->setFilename(filePath());
    task->setText(plainText().toUtf8());
    auto futures = task->start();
    m_indexerWatcher.setFuture(futures.highlightFuture);
    m_semanticWatcher.setFuture(futures.semanticFuture);
}

void GoEditorDocument::acceptSemaHighlights(int from, int to)
{
    if (m_indexRevision != document()->revision() || m_indexerWatcher.isCanceled())
            return; // aborted

    TextEditor::SemanticHighlighter::incrementalApplyExtraAdditionalFormats(
                syntaxHighlighter(), m_indexerWatcher.future(), from, to, m_highlightFormatMap);
}

void GoEditorDocument::finishSemaHighlights()
{
    if (m_indexRevision != document()->revision() || m_indexerWatcher.isCanceled())
            return; // aborted

    TextEditor::SemanticHighlighter::clearExtraAdditionalFormatsUntilEnd(
                syntaxHighlighter(), m_indexerWatcher.future());
}

void GoEditorDocument::acceptSemantic(int from, int to)
{
    Q_UNUSED(to);
    if (m_indexRevision != document()->revision() || m_semanticWatcher.isCanceled())
            return; // aborted
    GoSemanticInfoPtr sema = m_semanticWatcher.resultAt(from);
    sema->applyCodeFolding(document());
    emit semanticUpdated(sema);
}

void GoEditorDocument::fixTabSettings()
{
    // Prevent recursive call from signal.
    if (m_isFixingTabSettings)
        return;
    m_isFixingTabSettings = true;
    TabSettings settings = tabSettings();
    settings.m_tabPolicy = TabSettings::TabsOnlyTabPolicy;
    setTabSettings(settings);
    m_isFixingTabSettings = false;
}

} // namespace GoEditor
