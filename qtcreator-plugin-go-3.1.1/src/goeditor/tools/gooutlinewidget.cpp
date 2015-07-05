#include "gooutlinewidget.h"
#include "../goeditor.h"
#include "../goeditorwidget.h"
#include "../goeditordocument.h"
#include "gooutlinemodel.h"
#include <coreplugin/editormanager/ieditor.h>
#include <utils/navigationtreeview.h>
#include <QScopedPointer>
#include <QTextBlock>
#include <QVBoxLayout>

namespace GoEditor {

GoOutlineWidget::GoOutlineWidget(Internal::GoEditorWidget *editorWidget)
    : TextEditor::IOutlineWidget(editorWidget)
    , m_editor(editorWidget)
    , m_treeView(new Utils::NavigationTreeView(this))
    , m_filterModel(new QSortFilterProxyModel(this))
{
    auto document = qobject_cast<GoEditorDocument*>(editorWidget->baseTextDocument());
    m_sourceModel = new GoOutlineModel(document);
    m_filterModel->setSourceModel(m_sourceModel);
    m_treeView->setModel(m_filterModel);
    m_treeView->setEditTriggers(QTreeView::NoEditTriggers);
    setFocusProxy(m_treeView);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_treeView);

    connect(document, SIGNAL(semanticUpdated(GoSemanticInfoPtr)),
            this, SLOT(applySemanticInfo(GoSemanticInfoPtr)));
    connect(m_treeView, SIGNAL(activated(QModelIndex)),
            this, SLOT(focusEditor()));
    connect(m_treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(updateSelectionInText(QItemSelection)));

    setLayout(layout);
    applySemanticInfo(document->semanticInfo());
}

QList<QAction *> GoOutlineWidget::filterMenuActions() const
{
    return QList<QAction *>();
}

void GoOutlineWidget::setCursorSynchronization(bool syncWithCursor)
{
    m_enableCursorSync = syncWithCursor;
#if 0
    if (m_enableCursorSync)
        updateSelectionInTree(m_editor->outlineModelIndex());
#endif
}

void GoOutlineWidget::applySemanticInfo(const GoSemanticInfoPtr &semantic)
{
    if (semantic && m_sourceModel) {
        m_sourceModel->update(*semantic);
        m_treeView->expandAll();
    }
}

void GoOutlineWidget::updateSelectionInTree(const QModelIndex &index)
{
    // TODO: implement it.
    Q_UNUSED(index);
}

void GoOutlineWidget::updateSelectionInText(const QItemSelection &selection)
{
    if (!syncCursor())
        return;

    if (!selection.indexes().isEmpty()) {
        QModelIndex index = selection.indexes().first();
        updateTextCursor(index);
    }
}

void GoOutlineWidget::updateTextCursor(const QModelIndex &index)
{
    QModelIndex sourceIndex = m_filterModel->mapToSource(index);
    quint64 encodedPos = m_sourceModel->data(sourceIndex, GoOutlineModel::GoLocationRole).toULongLong();
    GoPosition position;
    position.decodePosition(encodedPos);

    const QTextBlock itemBlock = m_editor->document()->findBlockByLineNumber(position.line - 1);
    const int offset = itemBlock.position() + position.column - 1;

    Core::EditorManager::cutForwardNavigationHistory();
    Core::EditorManager::addCurrentPositionToNavigationHistory();

    QTextCursor textCursor(itemBlock);
    m_blockCursorSync = true;
    textCursor.setPosition(offset);
    m_editor->setTextCursor(textCursor);
    m_editor->centerCursor();
    m_blockCursorSync = false;
}

void GoOutlineWidget::focusEditor()
{
    m_editor->setFocus();
}

bool GoOutlineWidget::syncCursor() const
{
    return m_enableCursorSync && !m_blockCursorSync;
}

bool GoOutlineWidgetFactory::supportsEditor(Core::IEditor *editor) const
{
    return nullptr != qobject_cast<Internal::GoEditor*>(editor);
}

TextEditor::IOutlineWidget *GoOutlineWidgetFactory::createWidget(Core::IEditor *editor)
{
    auto editorWidget = qobject_cast<Internal::GoEditorWidget*>(editor->widget());
    return new GoOutlineWidget(editorWidget);
}

} // namespace GoEditor
