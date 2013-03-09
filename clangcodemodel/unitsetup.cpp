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

#include "unitsetup.h"
#include "liveunitsmanager.h"
#include "indexer.h"

#include <QtCore/QtConcurrentRun>
#include <QtCore/QFutureWatcher>

using namespace ClangCodeModel;
using namespace Internal;

UnitSetup::UnitSetup()
{}

UnitSetup::UnitSetup(const QString &fileName, Indexer *indexer)
    : m_fileName(fileName)
    , m_unit(LiveUnitsManager::instance()->unit(fileName))
    , m_indexer(indexer)
{
    connect(LiveUnitsManager::instance(), SIGNAL(unitAvailable(Unit)),
            this, SLOT(assignUnit(Unit)));

    if (!m_unit.isLoaded()) {
        if (!LiveUnitsManager::instance()->isTracking(fileName)) {
            // Start tracking this file so the indexer is aware of it and will consequently
            // update the corresponding unit in the manager.
            LiveUnitsManager::instance()->requestTracking(fileName);
        }
        indexer->evaluateFile(fileName);
    }
}

UnitSetup::~UnitSetup()
{
    m_unit = Unit(); // We want to "release" this unit share so the manager might remove it
        // in the case no one else is tracking it.
    LiveUnitsManager::instance()->cancelTrackingRequest(m_fileName);
}

void UnitSetup::assignUnit(const Unit &unit)
{
    if (m_fileName == unit.fileName() && unit.isLoaded())
        m_unit = unit;
}

void UnitSetup::checkForNewerUnit()
{
    if (LiveUnitsManager::instance()->isTracking(m_fileName))
        assignUnit(LiveUnitsManager::instance()->unit(m_fileName));
}
