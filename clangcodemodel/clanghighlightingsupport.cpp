/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "clanghighlightingsupport.h"

#include <coreplugin/idocument.h>
#include <texteditor/basetexteditor.h>
#include <texteditor/itexteditor.h>

#include <QTextBlock>
#include <QTextEdit>
#include "pchmanager.h"


using namespace ClangCodeModel;
using namespace ClangCodeModel::Internal;
using namespace CppTools;

DiagnosticsHandler::DiagnosticsHandler(TextEditor::ITextEditor *textEditor)
    : m_editor(textEditor)
{
}

void DiagnosticsHandler::setDiagnostics(const QList<ClangCodeModel::Diagnostic> &diagnostics)
{
    TextEditor::BaseTextEditorWidget *ed = qobject_cast<TextEditor::BaseTextEditorWidget *>(m_editor->widget());
    // set up the format for the errors
    QTextCharFormat errorFormat;
    errorFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
    errorFormat.setUnderlineColor(Qt::red);

    // set up the format for the warnings.
    QTextCharFormat warningFormat;
    warningFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
    warningFormat.setUnderlineColor(Qt::darkYellow);

    QList<QTextEdit::ExtraSelection> selections;
    foreach (const ClangCodeModel::Diagnostic &m, diagnostics) {
        QTextEdit::ExtraSelection sel;

        switch (m.severity()) {
        case ClangCodeModel::Diagnostic::Error:
        case ClangCodeModel::Diagnostic::Fatal:
            sel.format = errorFormat;
            break;

        case ClangCodeModel::Diagnostic::Warning:
            sel.format = warningFormat;
            break;

        default:
            continue;
        }

        QTextCursor c(ed->document()->findBlockByNumber(m.location().line() - 1));
        const int linePos = c.position();
        c.setPosition(linePos + m.location().column() - 1);

        const QString text = c.block().text();
        if (m.length() == 0) {
            int i = m.location().column() - 1;
            if (i == text.size() || (i < text.size() && text.at(i).isSpace())) {
                // move backward to 1 character
                c.setPosition(linePos + i - 1, QTextCursor::KeepAnchor);
            } else {
                // forward scan
                for ( ; i < text.size(); ++i)
                    if (text.at(i).isSpace()) {
                        ++i;
                        break;
                    }
                c.setPosition(linePos + i, QTextCursor::KeepAnchor);
            }
        } else {
            c.setPosition(c.position() + m.length(), QTextCursor::KeepAnchor);
        }

        sel.cursor = c;
        sel.format.setToolTip(m.spelling());
        selections.append(sel);
    }

    ed->setExtraSelections(TextEditor::BaseTextEditorWidget::CodeWarningsSelection, selections);
}

ClangHighlightingSupport::ClangHighlightingSupport(TextEditor::ITextEditor *textEditor, FastIndexer *fastIndexer)
    : CppHighlightingSupport(textEditor)
    , m_fastIndexer(fastIndexer)
    , m_semanticMarker(new ClangCodeModel::SemanticMarker)
    , m_diagnosticsHandler(new DiagnosticsHandler(textEditor))
{
}

ClangHighlightingSupport::~ClangHighlightingSupport()
{
}

QFuture<CppHighlightingSupport::Use> ClangHighlightingSupport::highlightingFuture(
        const CPlusPlus::Document::Ptr &doc,
        const CPlusPlus::Snapshot &snapshot) const
{
    Q_UNUSED(doc);
    Q_UNUSED(snapshot);

    TextEditor::BaseTextEditorWidget *ed = qobject_cast<TextEditor::BaseTextEditorWidget *>(editor()->widget());
    int firstLine = 1;
    int lastLine = ed->document()->blockCount();

    const QString fileName = editor()->document()->fileName();
    CPlusPlus::CppModelManagerInterface *modelManager = CPlusPlus::CppModelManagerInterface::instance();
    QList<CPlusPlus::CppModelManagerInterface::ProjectPart::Ptr> parts = modelManager->projectPart(fileName);
    QStringList options;
    foreach (const CPlusPlus::CppModelManagerInterface::ProjectPart::Ptr &part, parts) {
        options = Utils::createClangOptions(part, fileName);
        if (PCHInfo::Ptr pchInfo = PCHManager::instance()->pchInfo(part))
            options.append(ClangCodeModel::Utils::createPCHInclusionOptions(pchInfo->fileName()));
        if (!options.isEmpty())
            break;
    }
    if (options.isEmpty())
        options = Utils::clangNonProjectFileOptions();

    //### FIXME: the range is way too big.. can't we just update the visible lines?
    CreateMarkers *createMarkers = CreateMarkers::create(m_semanticMarker, fileName, options, firstLine, lastLine, m_fastIndexer);
    QObject::connect(createMarkers, SIGNAL(diagnosticsReady(const QList<ClangCodeModel::Diagnostic> &)),
                     m_diagnosticsHandler.data(), SLOT(setDiagnostics(const QList<ClangCodeModel::Diagnostic> &)));
    return createMarkers->start();
}

ClangHighlightingSupportFactory::~ClangHighlightingSupportFactory()
{
}

CppHighlightingSupport *ClangHighlightingSupportFactory::highlightingSupport(TextEditor::ITextEditor *editor)
{
    return new ClangHighlightingSupport(editor, m_fastIndexer);
}
