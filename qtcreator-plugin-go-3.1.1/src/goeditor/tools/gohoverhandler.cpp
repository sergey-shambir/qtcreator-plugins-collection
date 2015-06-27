#include "gohoverhandler.h"
#include "../goeditor.h"
#include "../goeditorwidget.h"

namespace GoEditor {

using namespace GoEditor::Internal;

GoHoverHandler::GoHoverHandler(QObject *parent) :
    TextEditor::BaseHoverHandler(parent)
{
}

bool GoHoverHandler::acceptEditor(Core::IEditor *editor)
{
    if (qobject_cast<GoEditor *>(editor) != 0)
        return true;
    return false;
}

void GoHoverHandler::identifyMatch(TextEditor::ITextEditor *editor, int pos)
{
    if (auto goEditor = qobject_cast<GoEditorWidget *>(editor->widget())) {
        QString tooltip = goEditor->extraSelectionTooltip(pos);
        if (!tooltip.isEmpty())
            setToolTip(tooltip);
    }
}

void GoHoverHandler::decorateToolTip()
{
    if (Qt::mightBeRichText(toolTip()))
        setToolTip(Qt::escape(toolTip()));
}

} // namespace GoEditor
