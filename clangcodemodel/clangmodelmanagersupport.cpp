#include "clangcompletion.h"
#include "clanghighlightingsupport.h"
#include "clangmodelmanagersupport.h"

#include <QCoreApplication>

using namespace ClangCodeModel;
using namespace ClangCodeModel::Internal;

ModelManagerSupport::ModelManagerSupport(FastIndexer *fastIndexer)
    : m_completionAssistProvider(new ClangCompletionAssistProvider)
    , m_fastIndexer(fastIndexer)
{
}

ModelManagerSupport::~ModelManagerSupport()
{
}

QString ModelManagerSupport::id() const
{
    return QLatin1String("ClangCodeMode.ClangCodeMode");
}

QString ModelManagerSupport::displayName() const
{
    return QCoreApplication::translate("ModelManagerSupport::displayName",
                                       "Clang");
}

CppTools::CppCompletionAssistProvider *ModelManagerSupport::completionAssistProvider()
{
    return m_completionAssistProvider.data();
}

CppTools::CppHighlightingSupport *ModelManagerSupport::highlightingSupport(
        TextEditor::ITextEditor *editor)
{
    return new ClangHighlightingSupport(editor, m_fastIndexer);
}
