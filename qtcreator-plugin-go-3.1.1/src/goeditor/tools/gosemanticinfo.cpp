#include "gosemanticinfo.h"
#include <texteditor/basetextdocumentlayout.h>
#include <texteditor/basetexteditor.h>
#include <QTextDocument>

namespace GoEditor {

using TextEditor::BaseTextDocumentLayout;
using TextEditor::TextBlockUserData;

// FIXME: doesn't work at all.
void GoSemanticInfo::applyCodeFolding(QTextDocument *document)
{
    if (nullptr == document)
        return;

    // Pre-calculate code folding level for each block in document
    QVector<int> levelPerBlock;
    levelPerBlock.resize(document->blockCount());
    int blockIdx = 0;
    int areaIdx = 0;
    for (int n = m_foldAreas.size(); areaIdx < n; ++areaIdx) {
        int lineFrom = qMin(m_foldAreas[areaIdx].lineFrom, levelPerBlock.size());
        for (; blockIdx < lineFrom; ++blockIdx) {
            levelPerBlock[blockIdx] = areaIdx;
        }
    }
    for (int n = levelPerBlock.size(); blockIdx < n; ++blockIdx) {
        levelPerBlock[blockIdx] = areaIdx - 1;
    }
    blockIdx = 0;
    areaIdx = 0;
    for (int n = m_foldAreas.size(); areaIdx < n; ++areaIdx) {
        int lineTo = qMin(m_foldAreas[areaIdx].lineTo, levelPerBlock.size());
        for (; blockIdx < lineTo; ++blockIdx) {
            levelPerBlock[blockIdx] -= areaIdx;
        }
    }
    for (int n = levelPerBlock.size(); blockIdx < n; ++blockIdx) {
        levelPerBlock[blockIdx] -= areaIdx - 1;
    }

    int index = 0;
    int prevLevel = 0;
    for (QTextBlock block = document->firstBlock(); block.isValid(); block = block.next()) {
        const int level = levelPerBlock[index];
        if (TextBlockUserData *userData = BaseTextDocumentLayout::testUserData(block)) {
            userData->setFoldingIndent(level);
            userData->setFoldingStartIncluded(level > prevLevel);
            userData->setFoldingEndIncluded(level < prevLevel);
        }
        prevLevel = level;
    }
    if (0 != document->blockCount())
        document->markContentsDirty(0, document->lastBlock().position() + document->lastBlock().length());
}

void GoSemanticInfo::displayDiagnostic(TextEditor::BaseTextEditorWidget *editor)
{
    if (nullptr == editor)
        return;

    QTextCharFormat errorFormat;
    errorFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
    errorFormat.setUnderlineColor(Qt::red);

    QList<QTextEdit::ExtraSelection> sels;
    QSet<int> linesBlacklist;
    QTextDocument *document = editor->document();
    for (const GoError &err : m_errors) {
        if (0 == err.line || linesBlacklist.contains(err.line))
            continue;
        linesBlacklist.insert(err.line);

        QTextCursor cursor(document->findBlockByNumber(err.line - 1));
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

        QTextEdit::ExtraSelection sel;
        sel.cursor = cursor;
        sel.format = errorFormat;
        sel.format.setToolTip(err.message);
        sels.append(sel);
    }
    editor->setExtraSelections(TextEditor::BaseTextEditorWidget::CodeWarningsSelection, sels);
}

template<class T>
static bool _lesserPositionPredicate(const T &a, const T &b)
{
    return (a.line < b.line) || (a.line == b.line && a.column < b.column);
}

template<class T>
static bool _lesserLineFromPredicate(const T &a, const T &b)
{
    return (a.lineFrom < b.lineFrom);
}

void GoSemanticInfo::sort()
{
    qSort(m_ranges.begin(), m_ranges.end(), _lesserPositionPredicate<GoHighlightRange>);
    qSort(m_errors.begin(), m_errors.end(), _lesserPositionPredicate<GoError>);
    qSort(m_outlineItems.begin(), m_outlineItems.end(), _lesserPositionPredicate<GoOutlineItem>);
    qSort(m_foldAreas.begin(), m_foldAreas.end(), _lesserLineFromPredicate<GoFoldArea>);
}

QVector<GoFoldArea> GoSemanticInfo::foldAreas() const
{
    return m_foldAreas;
}

void GoSemanticInfo::setFoldAreas(const QVector<GoFoldArea> &foldAreas)
{
    m_foldAreas = foldAreas;
}

void GoSemanticInfo::addFoldArea(const GoFoldArea &area)
{
    m_foldAreas.append(area);
}

QVector<GoOutlineItem> GoSemanticInfo::outlineItems() const
{
    return m_outlineItems;
}

void GoSemanticInfo::setOutlineItems(const QVector<GoOutlineItem> &outlineItems)
{
    m_outlineItems = outlineItems;
}

void GoSemanticInfo::addOutlineItem(const GoOutlineItem &item)
{
    m_outlineItems.append(item);
}

QVector<GoError> GoSemanticInfo::errors() const
{
    return m_errors;
}

void GoSemanticInfo::setErrors(const QVector<GoError> &errors)
{
    m_errors = errors;
}

void GoSemanticInfo::addError(const GoError &error)
{
    m_errors.append(error);
}

QVector<GoHighlightRange> GoSemanticInfo::ranges() const
{
    return m_ranges;
}

void GoSemanticInfo::setRanges(const QVector<GoHighlightRange> &ranges)
{
    m_ranges = ranges;
}

void GoSemanticInfo::addRange(const GoHighlightRange &range)
{
    m_ranges.append(range);
}

} // namespace GoEditor
