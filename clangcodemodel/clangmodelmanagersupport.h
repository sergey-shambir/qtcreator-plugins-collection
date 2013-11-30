#ifndef CLANGCODEMODEL_INTERNAL_CLANGMODELMANAGERSUPPORT_H
#define CLANGCODEMODEL_INTERNAL_CLANGMODELMANAGERSUPPORT_H

#include <cpptools/cppmodelmanagersupport.h>

#include <QScopedPointer>

namespace ClangCodeModel {
namespace Internal {

class FastIndexer;

class ModelManagerSupport: public CppTools::ModelManagerSupport
{
    Q_DISABLE_COPY(ModelManagerSupport)

public:
    ModelManagerSupport(FastIndexer *fastIndexer);
    virtual ~ModelManagerSupport();

    virtual QString id() const;
    virtual QString displayName() const;

    virtual CppTools::CppCompletionAssistProvider *completionAssistProvider();
    virtual CppTools::CppHighlightingSupport *highlightingSupport(TextEditor::ITextEditor *editor);

private:
    QScopedPointer<CppTools::CppCompletionAssistProvider> m_completionAssistProvider;
    FastIndexer *m_fastIndexer;
};

} // namespace Internal
} // namespace ClangCodeModel

#endif // CLANGCODEMODEL_INTERNAL_CLANGMODELMANAGERSUPPORT_H
