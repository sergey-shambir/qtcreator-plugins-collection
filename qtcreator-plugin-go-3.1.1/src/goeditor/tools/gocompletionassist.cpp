#include "gocompletionassist.h"
#include "../goeditorconstants.h"
#include "gocodeprocess.h"

#include <texteditor/completionsettings.h>
#include <texteditor/codeassist/basicproposalitem.h>
#include <texteditor/codeassist/basicproposalitemlistmodel.h>
#include <texteditor/codeassist/genericproposal.h>
#include <texteditor/codeassist/functionhintproposal.h>

#include <QTextDocument>

namespace GoEditor {
namespace Internal {

class GoFunctionHintProposalModel : public TextEditor::IFunctionHintProposalModel
{
public:
    GoFunctionHintProposalModel(const QList<CodeCompletion> &completions)
        : m_items(completions)
    {}

    virtual void reset() {}
    virtual int size() const;
    virtual QString text(int index) const;
    virtual int activeArgument(const QString &prefix) const;

private:
    QList<CodeCompletion> m_items;
};

int GoFunctionHintProposalModel::size() const
{
    return m_items.size();
}

QString GoFunctionHintProposalModel::text(int index) const
{
    return m_items[index].hint;
}

int GoFunctionHintProposalModel::activeArgument(const QString &prefix) const
{
    // TODO: calculate active argument
    Q_UNUSED(prefix)
    return 0;
}

bool GoCompletionAssistProvider::supportsEditor(const Core::Id &editorId) const
{
    return editorId == Constants::C_GOEDITOR_ID;
}

TextEditor::IAssistProcessor *GoCompletionAssistProvider::createProcessor() const
{
    return new GoCompletionAssistProcessor;
}

int GoCompletionAssistProvider::activationCharSequenceLength() const
{
    return 1;
}

bool GoCompletionAssistProvider::isActivationCharSequence(const QString &sequence) const
{
    QChar ch = sequence.at(0);
    return ch == QLatin1Char('.') || ch == QLatin1Char('(');
}

GoCompletionAssistProcessor::GoCompletionAssistProcessor()
    : m_keywordIcon(QLatin1String(":/goeditor/images/keyword.png"))
    , m_varIcon(QLatin1String(":/goeditor/images/var.png"))
    , m_funcIcon(QLatin1String(":/goeditor/images/func.png"))
    , m_typeIcon(QLatin1String(":/goeditor/images/type.png"))
    , m_constIcon(QLatin1String(":/goeditor/images/const.png"))
    , m_packageIcon(QLatin1String(":/goeditor/images/package.png"))
    , m_otherIcon(QLatin1String(":/goeditor/images/other.png"))
{
}

GoCompletionAssistProcessor::~GoCompletionAssistProcessor()
{
}

TextEditor::IAssistProposal *GoCompletionAssistProcessor::perform(const TextEditor::IAssistInterface *interface)
{
    if (interface->reason() == TextEditor::IdleEditor)
        return 0;
    m_interface.reset(static_cast<const TextEditor::DefaultAssistInterface *>(interface));
    deduceContext();

    GocodeProcess process(interface->fileName(), interface->textDocument()->toPlainText().toUtf8());
    QList<CodeCompletion> completions = process.codeCompleteAt(interface->position());

    TextEditor::IAssistProposal *proposal = NULL;
    if (m_context.isFunctionCompletion) {
        QList<CodeCompletion> functions;
        foreach (const CodeCompletion &cc, completions) {
            if (cc.kind == CodeCompletion::Func && m_context.functionName == cc.text)
                functions.append(cc);
        }
        TextEditor::IFunctionHintProposalModel *model = new GoFunctionHintProposalModel(functions);
        proposal = new TextEditor::FunctionHintProposal(m_context.startPosition, model);
    } else {
        foreach (const CodeCompletion &cc, completions) {
            TextEditor::BasicProposalItem *item = new TextEditor::BasicProposalItem();
            QString text = cc.text;
            if (cc.kind == CodeCompletion::Func) {
                if (cc.hint.contains(QLatin1String("()"))) {
                    text += QLatin1String("()");
                } else {
                    text += QLatin1String("(");
                }
            }
            item->setText(text);
            item->setDetail(cc.hint);
            item->setIcon(iconForKind(cc.kind));
            m_completions.append(item);
        }
        TextEditor::IGenericProposalModel *model = new TextEditor::BasicProposalItemListModel(m_completions);
        proposal = new TextEditor::GenericProposal(m_context.startPosition, model);
    }
    return proposal;
}

bool GoCompletionAssistProcessor::isIdentifierHead(const QChar &ch)
{
    return ch.isLetter() || ch == QLatin1Char('_');
}

bool GoCompletionAssistProcessor::isIdentifierTail(const QChar &ch)
{
    return ch.isLetterOrNumber() || ch == QLatin1Char('_');
}

void GoCompletionAssistProcessor::deduceContext()
{
    m_context.isFunctionCompletion = false;
    m_context.position = m_interface->position();
    m_context.startPosition = m_context.position;

    int pos = m_context.position - 1;
    if (pos < 0)
        return;

    QChar activationChar = m_interface->characterAt(pos);
    if (activationChar == QLatin1Char('(')) {
        const int functionNameEnd = pos;
        --pos;
        while (pos > 0) {
            QChar identifierPart = m_interface->characterAt(pos - 1);
            if (isIdentifierTail(identifierPart))
                --pos;
            else
                break;
        }
        if (isIdentifierHead(m_interface->characterAt(pos))) {
            m_context.startPosition = pos;
            m_context.isFunctionCompletion = true;
            m_context.functionName = m_interface->textAt(pos, functionNameEnd - pos);
        }
    } else if (isIdentifierTail(activationChar)) {
        while (pos > 0) {
            QChar identifierPart = m_interface->characterAt(pos - 1);
            if (isIdentifierTail(identifierPart))
                --pos;
            else
                break;
        }
        if (isIdentifierHead(m_interface->characterAt(pos)))
            m_context.startPosition = pos;
    }
}

QIcon GoCompletionAssistProcessor::iconForKind(CodeCompletion::Kind kind) const
{
    switch (kind) {
    case CodeCompletion::Func:
        return m_funcIcon;
    case CodeCompletion::Package:
        return m_packageIcon;
    case CodeCompletion::Variable:
        return m_varIcon;
    case CodeCompletion::Type:
        return m_typeIcon;
    case CodeCompletion::Const:
        return m_constIcon;
    case CodeCompletion::Other:
    default:
        return m_otherIcon;
    }
}

} // namespace Internal
} // namespace GoEditor
