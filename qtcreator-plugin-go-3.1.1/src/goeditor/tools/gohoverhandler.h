#ifndef GOEDITOR_GOHOVERHANDLER_H
#define GOEDITOR_GOHOVERHANDLER_H

#include <texteditor/basehoverhandler.h>

namespace Core { class IEditor; }

namespace TextEditor { class ITextEditor; }

namespace GoEditor {

class GoHoverHandler : public TextEditor::BaseHoverHandler
{
    Q_OBJECT
public:
    explicit GoHoverHandler(QObject *parent = 0);

private:
    bool acceptEditor(Core::IEditor *editor) override;
    void identifyMatch(TextEditor::ITextEditor *editor, int pos) override;
    void decorateToolTip() override;
};

} // namespace GoEditor

#endif // GOEDITOR_GOHOVERHANDLER_H
