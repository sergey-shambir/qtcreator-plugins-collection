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

#include "pchmanager.h"
#include "utils.h"
#include "clangutils.h"

#include <coreplugin/icore.h>
#include <coreplugin/messagemanager.h>
#include <coreplugin/progressmanager/progressmanager.h>

#include <utils/runextensions.h>

#include <QFile>

using namespace ClangCodeModel;
using namespace ClangCodeModel::Internal;
using namespace CPlusPlus;

PCHManager *PCHManager::m_instance = 0;

PCHManager::PCHManager(QObject *parent)
    : QObject(parent)
{
    Q_ASSERT(!m_instance);
    m_instance = this;

    Core::MessageManager *msgMgr = Core::MessageManager::instance();
    connect(this, SIGNAL(pchMessage(QString, bool)),
            msgMgr, SLOT(printToOutputPane(QString, bool)));

    connect(&m_pchGenerationWatcher, SIGNAL(finished()),
            this, SLOT(updateActivePCHFiles()));
}

PCHManager::~PCHManager()
{
    Q_ASSERT(m_instance);
    m_instance = 0;
    qDeleteAll(m_projectSettings.values());
    m_projectSettings.clear();
}

PCHManager *PCHManager::instance()
{
    return m_instance;
}

PCHInfo::Ptr PCHManager::pchInfo(const ProjectPart::Ptr &projectPart) const
{
    QMutexLocker locker(&m_mutex);

    return m_activePCHFiles[projectPart];
}

ClangProjectSettings *PCHManager::settingsForProject(ProjectExplorer::Project *project)
{
    QMutexLocker locker(&m_mutex);

    ClangProjectSettings *cps = m_projectSettings.value(project);
    if (!cps) {
        cps = new ClangProjectSettings(project);
        m_projectSettings.insert(project, cps);
        cps->pullSettings();
        connect(cps, SIGNAL(pchSettingsChanged()),
                this, SLOT(clangProjectSettingsChanged()));
    }
    return cps;
}

void PCHManager::setPCHInfo(const QList<ProjectPart::Ptr> &projectParts,
                            const PCHInfo::Ptr &pchInfo,
                            const QPair<bool, QStringList> &msgs)
{
    QMutexLocker locker(&m_mutex);

    foreach (ProjectPart::Ptr pPart, projectParts)
        m_activePCHFiles[pPart] = pchInfo;

    if (pchInfo) {
        if (msgs.first)
            emit pchMessage(tr("Successfully generated PCH file \"%1\".").arg(
                                pchInfo->fileName()), false);
        else
            emit pchMessage(tr("Failed to generate PCH file \"%1\".").arg(
                                pchInfo->fileName()), false);
        if (!msgs.second.isEmpty())
            emit pchMessage(msgs.second.join(QLatin1String("\n")), true);
    }
}

void PCHManager::clangProjectSettingsChanged()
{
    ClangProjectSettings *cps = qobject_cast<ClangProjectSettings *>(sender());
    if (!cps)
        return;

    onProjectPartsUpdated(cps->project());
}

void PCHManager::onAboutToRemoveProject(ProjectExplorer::Project *project)
{
    Q_UNUSED(project);

    // we cannot ask the ModelManager for the parts, because, depending on
    // the order of signal delivery, it might already have wiped any information
    // about the project.

    updateActivePCHFiles();
}

void PCHManager::onProjectPartsUpdated(ProjectExplorer::Project *project)
{
    ClangProjectSettings *cps = settingsForProject(project);
    Q_ASSERT(cps);

    CppModelManagerInterface *mmi = CppModelManagerInterface::instance();
    const QList<ProjectPart::Ptr> projectParts = mmi->projectInfo(
                cps->project()).projectParts();
    updatePchInfo(cps, projectParts);

    emit pchInfoUpdated();
}

void PCHManager::updatePchInfo(ClangProjectSettings *cps,
                               const QList<ProjectPart::Ptr> &projectParts)
{
    if (m_pchGenerationWatcher.isRunning()) {
//        m_pchGenerationWatcher.cancel();
        m_pchGenerationWatcher.waitForFinished();
    }

    QFuture<void> future = QtConcurrent::run(&PCHManager::doPchInfoUpdate,
                                             cps->pchUsage(),
                                             cps->customPchFile(),
                                             projectParts);
    m_pchGenerationWatcher.setFuture(future);
    Core::ICore::instance()->progressManager()->addTask(future,
                                                        tr("Precompiling..."),
                                                        QLatin1String("Key.Tmp.Precompiling"));
}

void PCHManager::doPchInfoUpdate(QFutureInterface<void> &future,
                                 ClangProjectSettings::PchUsage pchUsage,
                                 const QString customPchFile,
                                 const QList<ProjectPart::Ptr> projectParts)
{
    PCHManager *pchManager = PCHManager::instance();

//    qDebug() << "switching to" << pchUsage;

    Core::MessageManager *msgMgr = Core::MessageManager::instance();

    if (pchUsage == ClangProjectSettings::PchUse_None
            || (pchUsage == ClangProjectSettings::PchUse_Custom && customPchFile.isEmpty())) {
        future.setProgressRange(0, 2);
        msgMgr->printToOutputPane(QLatin1String("updatePchInfo: switching to none"));
        PCHInfo::Ptr emptyPch = PCHInfo::createEmpty();
        pchManager->setPCHInfo(projectParts, emptyPch, qMakePair(true, QStringList()));
        future.setProgressValue(1);
    } else if (pchUsage == ClangProjectSettings::PchUse_BuildSystem_Fuzzy) {
        msgMgr->printToOutputPane(QLatin1String("updatePchInfo: switching to build system (fuzzy)"));
        QHash<QString, QSet<QString> > includes, frameworks, clangFlags;
        QHash<QString, QSet<QByteArray> > definesPerPCH;
        QHash<QString, ProjectPart::Language> lang;
        QHash<QString, bool> objc;
        QHash<QString, ProjectPart::QtVersion> qtVersions;
        QHash<QString, QList<ProjectPart::Ptr> > inputToParts;
        foreach (const ProjectPart::Ptr &projectPart, projectParts) {
            if (projectPart->precompiledHeaders.isEmpty())
                continue;
            const QString &pch = projectPart->precompiledHeaders.first(); // TODO: support more than 1 PCH file.
            if (!QFile(pch).exists())
                continue;
            inputToParts[pch].append(projectPart);

            includes[pch].unite(QSet<QString>::fromList(projectPart->includePaths));
            frameworks[pch].unite(QSet<QString>::fromList(projectPart->frameworkPaths));
            lang[pch] = std::max(lang.value(pch, ProjectPart::C89), projectPart->language);
            objc[pch] = objc.value(pch, false) || !projectPart->objcSourceFiles.isEmpty();

            QSet<QByteArray> projectDefines = QSet<QByteArray>::fromList(projectPart->defines.split('\n'));
            QMutableSetIterator<QByteArray> iter(projectDefines);
            while (iter.hasNext()){
                QByteArray v = iter.next();
                if (v.startsWith("#define _") || v.isEmpty()) // TODO: see ProjectPart::createClangOptions
                    iter.remove();
            }

            if (definesPerPCH.contains(pch)) {
                definesPerPCH[pch].intersect(projectDefines);
            } else {
                definesPerPCH[pch] = projectDefines;
            }

            qtVersions[pch] = projectPart->qtVersion;
        }

        future.setProgressRange(0, definesPerPCH.size() + 1);
        future.setProgressValue(0);

        foreach (const QString &pch, inputToParts.keys()) {
            if (future.isCanceled())
                return;
            QStringList options = Utils::createClangOptions(lang[pch],
                                                            objc[pch],
                                                            true,
                                                            qtVersions[pch],
                                                            definesPerPCH[pch].toList(),
                                                            includes[pch].toList(),
                                                            frameworks[pch].toList());
            PCHInfo::Ptr pchInfo = pchManager->findMatchingPCH(pch, options, true);
            QPair<bool, QStringList> msgs = qMakePair(true, QStringList());
            if (pchInfo.isNull()) {

                pchInfo = PCHInfo::createWithFileName(pch, options, objc[pch]);
                msgs = precompile(pchInfo);
            }
            pchManager->setPCHInfo(inputToParts[pch], pchInfo, msgs);
            future.setProgressValue(future.progressValue() + 1);
        }
    } else if (pchUsage == ClangProjectSettings::PchUse_BuildSystem_Exact) {
        future.setProgressRange(0, projectParts.size() + 1);
        future.setProgressValue(0);
        msgMgr->printToOutputPane(QLatin1String("updatePchInfo: switching to build system (exact)"));
        foreach (const ProjectPart::Ptr &projectPart, projectParts) {
            if (future.isCanceled())
                return;
            if (projectPart->precompiledHeaders.isEmpty())
                continue;
            const QString &pch = projectPart->precompiledHeaders.first(); // TODO: support more than 1 PCH file.
            if (!QFile(pch).exists())
                continue;

            bool objc = !projectPart->objcSourceFiles.isEmpty();
            QStringList options = Utils::createClangOptions(
                        projectPart->language,
                        objc,
                        true,
                        projectPart->qtVersion,
                        projectPart->defines.split('\n'),
                        projectPart->includePaths,
                        projectPart->frameworkPaths);
            PCHInfo::Ptr pchInfo = pchManager->findMatchingPCH(pch, options, false);
            QPair<bool, QStringList> msgs = qMakePair(true, QStringList());
            if (pchInfo.isNull()) {
                pchInfo = PCHInfo::createWithFileName(pch, options, objc);
                msgs = precompile(pchInfo);
            }
            pchManager->setPCHInfo(QList<ProjectPart::Ptr>() << projectPart,
                                   pchInfo, msgs);
            future.setProgressValue(future.progressValue() + 1);
        }
    } else if (pchUsage == ClangProjectSettings::PchUse_Custom) {
        future.setProgressRange(0, 2);
        future.setProgressValue(0);
        msgMgr->printToOutputPane(QLatin1String("updatePchInfo: switching to custom") + customPchFile);
        QSet<QString> includes, frameworks, clangFlags;
        ProjectPart::Language lang = ProjectPart::C89;
        bool objc = false;
        ProjectPart::QtVersion qtVersion = ProjectPart::NoQt;
        foreach (const ProjectPart::Ptr &projectPart, projectParts) {
            includes.unite(QSet<QString>::fromList(projectPart->includePaths));
            frameworks.unite(QSet<QString>::fromList(projectPart->frameworkPaths));
            lang = std::max(lang, projectPart->language);
            qtVersion = std::max(qtVersion, projectPart->qtVersion);
            objc |= !projectPart->objcSourceFiles.isEmpty();
        }

        QStringList opts = Utils::createClangOptions(lang,
                                                     objc,
                                                     true,
                                                     qtVersion,
                                                     QList<QByteArray>(),
                                                     includes.toList(),
                                                     frameworks.toList());
        PCHInfo::Ptr pchInfo = pchManager->findMatchingPCH(customPchFile, opts, true);
        QPair<bool, QStringList> msgs = qMakePair(true, QStringList());;
        if (future.isCanceled())
            return;
        if (pchInfo.isNull()) {
            pchInfo = PCHInfo::createWithFileName(customPchFile, opts, objc);
            msgs = precompile(pchInfo);
        }
        pchManager->setPCHInfo(projectParts, pchInfo, msgs);
        future.setProgressValue(1);
    }

    future.setProgressValue(future.progressValue() + 1);
}

PCHInfo::Ptr PCHManager::findMatchingPCH(const QString &inputFileName,
                                         const QStringList &options,
                                         bool fuzzyMatching) const
{
    QMutexLocker locker(&m_mutex);

    if (fuzzyMatching) {
        QStringList opts = options;
        opts.sort();
        foreach (PCHInfo::Ptr pchInfo, m_activePCHFiles.values()) {
            if (pchInfo->inputFileName() != inputFileName)
                continue;
            QStringList pchOpts = pchInfo->options();
            pchOpts.sort();
            if (pchOpts == opts)
                return pchInfo;
        }
    } else {
        foreach (PCHInfo::Ptr pchInfo, m_activePCHFiles.values())
            if (pchInfo->inputFileName() == inputFileName
                    && pchInfo->options() == options)
                return pchInfo;
    }

    return PCHInfo::Ptr();
}

void PCHManager::updateActivePCHFiles()
{
    QMutexLocker locker(&m_mutex);

    QSet<ProjectPart::Ptr> activeParts;
    CppModelManagerInterface *mmi = CppModelManagerInterface::instance();
    foreach (const CppModelManagerInterface::ProjectInfo &pi, mmi->projectInfos())
        activeParts.unite(QSet<ProjectPart::Ptr>::fromList(pi.projectParts()));
    QList<ProjectPart::Ptr> partsWithPCHFiles = m_activePCHFiles.keys();
    foreach (ProjectPart::Ptr pPart, partsWithPCHFiles)
        if (!activeParts.contains(pPart))
            m_activePCHFiles.remove(pPart);
}
