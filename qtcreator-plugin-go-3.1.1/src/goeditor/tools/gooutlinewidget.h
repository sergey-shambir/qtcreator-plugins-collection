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

#pragma once

#include <texteditor/outlinefactory.h>
#include <QSortFilterProxyModel>
#include "gosemanticinfo.h"

namespace Utils { class NavigationTreeView; }

namespace GoEditor {

namespace Internal { class GoEditorWidget; }
class GoOutlineModel;

class GoOutlineWidget : public TextEditor::IOutlineWidget
{
    Q_OBJECT
public:
    GoOutlineWidget(Internal::GoEditorWidget *editorWidget);

    QList<QAction *> filterMenuActions() const override;
    void setCursorSynchronization(bool syncWithCursor) override;

private slots:
    void applySemanticInfo(const GoSemanticInfoPtr &semantic);
    void updateSelectionInTree(const QModelIndex &index);
    void updateSelectionInText(const QItemSelection &selection);
    void updateTextCursor(const QModelIndex &index);
    void focusEditor();

private:
    bool syncCursor() const;

    GoOutlineModel *m_sourceModel = nullptr;
    Internal::GoEditorWidget *m_editor = nullptr;
    Utils::NavigationTreeView *m_treeView = nullptr;
    QSortFilterProxyModel *m_filterModel = nullptr;
    bool m_enableCursorSync = true;
    bool m_blockCursorSync = false;
};

class GoOutlineWidgetFactory : public TextEditor::IOutlineWidgetFactory
{
    Q_OBJECT
public:
    bool supportsEditor(Core::IEditor *editor) const override;
    TextEditor::IOutlineWidget *createWidget(Core::IEditor *editor) override;
};

} // namespace GoEditor
