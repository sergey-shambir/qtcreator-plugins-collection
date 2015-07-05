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
#include "gocodeprocess.h"

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
