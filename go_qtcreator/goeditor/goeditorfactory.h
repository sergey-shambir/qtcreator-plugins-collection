#ifndef GOEDITOR_INTERNAL_GOEDITORFACTORY_H
#define GOEDITOR_INTERNAL_GOEDITORFACTORY_H

#include <coreplugin/editormanager/ieditorfactory.h>

namespace GoEditor {
namespace Internal {

class GoEditorFactory : public Core::IEditorFactory
{
    Q_OBJECT
public:
    explicit GoEditorFactory(QObject *parent = 0);

    /**
      Creates and initializes new editor widget
      */
    Core::IEditor *createEditor() override;
};

} // namespace Internal
} // namespace GoEditor

#endif // GOEDITOR_INTERNAL_GOEDITORFACTORY_H
