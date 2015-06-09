#ifndef GOEDITOR_INTERNAL_GOCOMPLETIONASSIST_H
#define GOEDITOR_INTERNAL_GOCOMPLETIONASSIST_H

#include "gocodetask.h"

#include <texteditor/codeassist/completionassistprovider.h>
#include <texteditor/codeassist/iassistprocessor.h>
#include <texteditor/codeassist/defaultassistinterface.h>
#include <texteditor/codeassist/ifunctionhintproposalmodel.h>
#include <texteditor/codeassist/basicproposalitem.h>

#include <QScopedPointer>
#include <QIcon>

namespace GoEditor {
namespace Internal {

class GoCompletionAssistProvider : public TextEditor::CompletionAssistProvider
{
    Q_OBJECT
public:
    bool supportsEditor(const Core::Id &editorId) const override;
    TextEditor::IAssistProcessor *createProcessor() const override;

    int activationCharSequenceLength() const override;
    bool isActivationCharSequence(const QString &sequence) const override;
};

class GoCompletionAssistProcessor : public TextEditor::IAssistProcessor
{
public:
    GoCompletionAssistProcessor();
    virtual ~GoCompletionAssistProcessor();

    TextEditor::IAssistProposal *perform(const TextEditor::IAssistInterface *interface) override;

private:
    struct Context {
        int startPosition;
        int position;
        bool isFunctionCompletion;
        QString functionName;
    };

    static bool isIdentifierTail(const QChar &ch);
    static bool isIdentifierHead(const QChar &ch);
    void deduceContext();
    QIcon iconForKind(CodeCompletion::Kind kind) const;

    QScopedPointer<const TextEditor::DefaultAssistInterface> m_interface;
    Context m_context;
    QList<TextEditor::BasicProposalItem *> m_completions;

    QIcon m_keywordIcon;
    QIcon m_varIcon;
    QIcon m_funcIcon;
    QIcon m_typeIcon;
    QIcon m_constIcon;
    QIcon m_packageIcon;
    QIcon m_otherIcon;
};

} // namespace Internal
} // namespace GoEditor

#endif // GOEDITOR_INTERNAL_GOCOMPLETIONASSIST_H
