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

#include "clangcodemodelplugin.h"
#include "clangprojectsettingspropertiespage.h"
#include "fastindexer.h"
#include "pchmanager.h"
#include "utils.h"

#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <coreplugin/imode.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/id.h>

#include <cpptools/cppmodelmanager.h>

#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/session.h>

#include <QtPlugin>

namespace ClangCodeModel {
namespace Internal {

ClangCodeModelPlugin::ClangCodeModelPlugin()
    : m_completionAssistProvider(0)
    , m_highlightingFactory(0)
{
}

bool ClangCodeModelPlugin::initialize(const QStringList &arguments, QString *errorMessage)
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorMessage)

    addAutoReleasedObject(new ClangProjectSettingsPanelFactory);

    ClangCodeModel::Internal::initializeClang();

    connect(Core::ICore::editorManager(), SIGNAL(editorAboutToClose(Core::IEditor*)),
            &m_liveUnitsManager, SLOT(editorAboutToClose(Core::IEditor*)));
    connect(Core::ICore::editorManager(), SIGNAL(editorOpened(Core::IEditor*)),
            &m_liveUnitsManager, SLOT(editorOpened(Core::IEditor*)));

#ifdef CLANG_COMPLETION
    m_completionAssistProvider.reset(new ClangCompletionAssistProvider);
    CPlusPlus::CppModelManagerInterface::instance()->setCppCompletionAssistProvider(m_completionAssistProvider.data());
#endif // CLANG_COMPLETION

    PCHManager *pchManager = new PCHManager(this);
    FastIndexer *fastIndexer = 0;

#ifdef CLANG_INDEXING
    m_indexer.reset(new ClangIndexer);
    fastIndexer = m_indexer.data();
    CPlusPlus::CppModelManagerInterface::instance()->setIndexingSupport(m_indexer->indexingSupport());

    // wire up the pch manager
    ProjectExplorer::ProjectExplorerPlugin *pe =
       ProjectExplorer::ProjectExplorerPlugin::instance();
    ProjectExplorer::SessionManager *session = pe->session();
    connect(session, SIGNAL(aboutToRemoveProject(ProjectExplorer::Project*)),
            pchManager, SLOT(onAboutToRemoveProject(ProjectExplorer::Project*)));
    connect(CPlusPlus::CppModelManagerInterface::instance(), SIGNAL(projectPartsUpdated(ProjectExplorer::Project*)),
            pchManager, SLOT(onProjectPartsUpdated(ProjectExplorer::Project*)));
#else // !CLANG_INDEXING
    Q_UNUSED(pchManager);
#endif // CLANG_INDEXING

#ifdef CLANG_HIGHLIGHTING
    m_highlightingFactory.reset(new ClangHighlightingSupportFactory(fastIndexer));
    CPlusPlus::CppModelManagerInterface::instance()->setHighlightingSupportFactory(m_highlightingFactory.data());
#else // !CLANG_HIGHLIGHTING
    Q_UNUSED(fastIndexer);
#endif // CLANG_HIGHLIGHTING

    return true;
}

void ClangCodeModelPlugin::extensionsInitialized()
{
}

ExtensionSystem::IPlugin::ShutdownFlag ClangCodeModelPlugin::aboutToShutdown()
{
    CPlusPlus::CppModelManagerInterface::instance()->setCppCompletionAssistProvider(0);
    CPlusPlus::CppModelManagerInterface::instance()->setHighlightingSupportFactory(0);
    CPlusPlus::CppModelManagerInterface::instance()->setIndexingSupport(0);

    return ExtensionSystem::IPlugin::aboutToShutdown();
}

} // namespace Internal
} // namespace Clang

Q_EXPORT_PLUGIN(ClangCodeModel::Internal::ClangCodeModelPlugin)
