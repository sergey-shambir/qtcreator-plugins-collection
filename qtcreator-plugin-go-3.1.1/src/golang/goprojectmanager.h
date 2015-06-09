/*
 * Copyright 2013 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 2.1.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Juhapekka Piiroinen <juhapekka.piiroinen@canonical.com>
 */

#ifndef GOPROJECTMANAGER_H
#define GOPROJECTMANAGER_H

#include <QObject>
#include "golangconstants.h"
#include "goproject.h"
#include <QFileInfo>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/icontext.h>
#include <coreplugin/mimedatabase.h>
#include <coreplugin/messagemanager.h>
#include <coreplugin/icore.h>
#include <coreplugin/idocument.h>
#include <coreplugin/documentmanager.h>
#include <projectexplorer/iprojectmanager.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/projectnodes.h>
#include <projectexplorer/project.h>
#include <projectexplorer/kitmanager.h>
#include <projectexplorer/target.h>
#include <projectexplorer/session.h>
#include <projectexplorer/runconfiguration.h>
#include <projectexplorer/applicationlauncher.h>

namespace GoLang {
class GoProject;

namespace Internal {
class Manager : public ProjectExplorer::IProjectManager
{
    Q_OBJECT

public:
    Manager();
    virtual QString mimeType() const override;

    ProjectExplorer::Project* openProject(const QString &filePath, QString *errorString) override;
    void registerProject(GoProject *project);
    void unregisterProject(GoProject *project);

    Q_INVOKABLE void *createKitMatcher () const;

protected slots:
    void onProjectAdded(ProjectExplorer::Project*);

private:
    QList<GoProject*> m_projects;
    
};


} // Internal
} // GoLang

#endif // GOPROJECTMANAGER_H
