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

#include "clangutils.h"

#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/icore.h>

#include <projectexplorer/kit.h>
#include <projectexplorer/kitmanager.h>
#include <projectexplorer/kitinformation.h>
#include <projectexplorer/toolchain.h>

#include <QFile>
#include <QSet>
#include <QString>

using namespace ClangCodeModel;
using namespace ClangCodeModel::Internal;
using namespace Core;
using namespace ProjectExplorer;

typedef CPlusPlus::CppModelManagerInterface::ProjectPart CppProjectPart;

UnsavedFiles ClangCodeModel::Utils::createUnsavedFiles(CPlusPlus::CppModelManagerInterface::WorkingCopy workingCopy)
{
    // TODO: change the modelmanager to hold one working copy, and amend it every time we ask for one.
    // TODO: Reason: the UnsavedFile needs a QByteArray.

    ICore *core = ICore::instance(); // FIXME
    QSet<QString> openFiles;
    foreach (IEditor *editor, core->editorManager()->openedEditors())
        openFiles.insert(editor->document()->fileName());

    UnsavedFiles result;
    QHashIterator<QString, QPair<QString, unsigned> > wcIter = workingCopy.iterator();
    while (wcIter.hasNext()) {
        wcIter.next();
        const QString &fileName = wcIter.key();
        if (openFiles.contains(fileName) && QFile(fileName).exists())
            result.insert(fileName, wcIter.value().first.toUtf8());
    }

    return result;
}

/**
 * @brief Creates list of command-line arguments required for correct parsing
 * @param pPart - null if file isn't part of any project
 * @param fileName - path to file, source isn't ObjC if name is empty
 */
QStringList ClangCodeModel::Utils::createClangOptions(const CppProjectPart::Ptr &pPart, const QString &fileName)
{
    if (pPart.isNull())
        return clangNonProjectFileOptions();

    const bool isObjC = fileName.isEmpty() ? false : pPart->objcSourceFiles.contains(fileName);
    const bool isHeader = fileName.isEmpty() ? false : pPart->headerFiles.contains(fileName);

    return createClangOptions(pPart, isObjC, isHeader);
}

/**
 * @brief Creates list of command-line arguments required for correct parsing
 * @param pPart - null if file isn't part of any project
 * @param isObjectiveC - file is ObjectiveC or ObjectiveC++
 */
QStringList ClangCodeModel::Utils::createClangOptions(const CppProjectPart::Ptr &pPart, bool isObjectiveC, bool isHeader)
{
    if (pPart.isNull())
        return clangNonProjectFileOptions();

    return createClangOptions(pPart->language,
                              isObjectiveC,
                              isHeader,
                              pPart->qtVersion,
                              pPart->defines.split('\n'),
                              pPart->includePaths,
                              pPart->frameworkPaths);
}

namespace {
bool isBlacklisted(const QString &path)
{
    static QStringList blacklistedPaths = QStringList()
            << QLatin1String("lib/gcc/i686-apple-darwin");

    foreach (const QString &blacklisted, blacklistedPaths)
        if (path.contains(blacklisted))
            return true;

    return false;
}
} // anonymous namespace

QStringList ClangCodeModel::Utils::createClangOptions(CppProjectPart::Language lang,
                                                      bool isObjC,
                                                      bool isHeader,
                                                      CppProjectPart::QtVersion qtVersion,
                                                      const QList<QByteArray> &defines,
                                                      const QStringList &includePaths,
                                                      const QStringList &frameworkPaths)
{
    QStringList result;

    switch (lang) {
    case CppProjectPart::C89:
        result << QLatin1String("-std=gnu89");
        result << ClangCodeModel::Utils::clangLanguageOption(false, isHeader, isObjC);
        break;
    case CppProjectPart::C99:
        result << QLatin1String("-std=gnu99");
        result << ClangCodeModel::Utils::clangLanguageOption(false, isHeader, isObjC);
        break;
    case CppProjectPart::CXX:
        result << QLatin1String("-std=gnu++98");
        result << ClangCodeModel::Utils::clangLanguageOption(true, isHeader, isObjC);
        break;
    case CppProjectPart::CXX11:
        result << QLatin1String("-std=c++11");
        result << ClangCodeModel::Utils::clangLanguageOption(true, isHeader, isObjC);
        break;
    default:
        break;
    }

    static const QString injectedHeader(Core::ICore::instance()->resourcePath() + QLatin1String("/cplusplus/qt%1-qobjectdefs-injected.h"));
    if (qtVersion == CppProjectPart::Qt4)
        result << QLatin1String("-include") << injectedHeader.arg(QLatin1Char('4'));
    if (qtVersion == CppProjectPart::Qt5)
        result << QLatin1String("-include") << injectedHeader.arg(QLatin1Char('5'));

#ifdef _MSC_VER
    result << QLatin1String("-fms-extensions")
           << QLatin1String("-fdelayed-template-parsing");
#endif

//    result << QLatin1String("-nobuiltininc");

    foreach (QByteArray def, defines) {
        if (def.isEmpty())
            continue;

        //### FIXME: the next 3 check shouldn't be needed: we probably don't want to get the compiler-defined defines in.
        if (!def.startsWith("#define "))
            continue;
        if (def.startsWith("#define _"))
            continue;
        if (def.startsWith("#define OBJC_NEW_PROPERTIES"))
            continue;

        QByteArray str = def.mid(8);
        int spaceIdx = str.indexOf(' ');
        QString arg;
        if (spaceIdx != -1) {
            arg = QLatin1String("-D" + str.left(spaceIdx) + "=" + str.mid(spaceIdx + 1));
        } else {
            arg = QLatin1String("-D" + str);
        }
        arg = arg.replace(QLatin1String("\\\""), QLatin1String("\""));
        arg = arg.replace(QLatin1String("\""), QLatin1String(""));
        if (!result.contains(arg))
            result.append(arg);
    }
    foreach (const QString &frameworkPath, frameworkPaths)
        result.append(QLatin1String("-F") + frameworkPath);
    foreach (const QString &inc, includePaths)
        if (!inc.isEmpty() && !isBlacklisted(inc))
            result << (QLatin1String("-I") + inc);

#if 0
    qDebug() << "--- m_args:";
    foreach (const QString &arg, result)
        qDebug() << "\t" << qPrintable(arg);
    qDebug() << "---";
#endif

    return result;
}

QStringList ClangCodeModel::Utils::clangNonProjectFileOptions()
{
    if (Kit *kit = KitManager::instance()->defaultKit()) {
        ToolChain *tc = ToolChainKitInformation::toolChain(kit);
        const bool isObjetiveC = false;
        const bool isHeader = false;
        const QStringList cxxflags;
        QStringList includePaths;
        QStringList frameworkPaths;
        foreach (const HeaderPath &header, tc->systemHeaderPaths(cxxflags, SysRootKitInformation::sysRoot(kit)))
            if (header.kind() == HeaderPath::FrameworkHeaderPath)
                frameworkPaths += header.path();
            else
                includePaths += header.path();

        QStringList ret = createClangOptions(CppProjectPart::CXX11,
                                  isObjetiveC,
                                  isHeader,
                                  CppProjectPart::NoQt,
                                  tc->predefinedMacros(cxxflags).split('\n'),
                                  includePaths,
                                  frameworkPaths);
        return ret;
    }
    QStringList ret;
    ret << QLatin1String("-x")
        << QLatin1String("c++")
        << QLatin1String("-std=c++11")
        << QLatin1String("-I.");
    return ret;
}

QStringList ClangCodeModel::Utils::clangLanguageOption(bool cxxEnabled,
                                                      bool isHeader,
                                                      bool isObjC)
{
    QStringList opts;
    opts += QLatin1String("-x");

    if (cxxEnabled && isHeader && isObjC)
        opts += QLatin1String("objective-c++-header");
    else if (!cxxEnabled && isHeader && isObjC)
        opts += QLatin1String("objective-c-header");
    else if (cxxEnabled && !isHeader && isObjC)
        opts += QLatin1String("objective-c++");
    else if (!cxxEnabled && !isHeader && isObjC)
        opts += QLatin1String("objective-c");
    else if (cxxEnabled && isHeader && !isObjC)
        opts += QLatin1String("c++-header");
    else if (!cxxEnabled && isHeader && !isObjC)
        opts += QLatin1String("c-header");
    else if (cxxEnabled && !isHeader && !isObjC)
        opts += QLatin1String("c++");
    else // !cxxEnabled && !isHeader && !isObjC
        opts += QLatin1String("c");

    return opts;
}

QStringList ClangCodeModel::Utils::createPCHInclusionOptions(const QStringList &pchFiles)
{
    QStringList opts;

    foreach (const QString &pchFile, pchFiles) {
        if (QFile(pchFile).exists()) {
            opts += QLatin1String("-include-pch");
            opts += pchFile;
        }
    }

    return opts;
}

QStringList ClangCodeModel::Utils::createPCHInclusionOptions(const QString &pchFile)
{
    return createPCHInclusionOptions(QStringList() << pchFile);
}
