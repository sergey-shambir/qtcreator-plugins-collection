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

#ifndef GOPROJECTFILE_H
#define GOPROJECTFILE_H

#include <QObject>
#include "golangconstants.h"
#include "goproject.h"
#include "goprojectmanager.h"

#include <coreplugin/idocument.h>

#include <utils/qtcassert.h>
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

namespace GoLang {
class GoProject;

namespace Internal {
class GoProjectFile : public Core::IDocument
{
    Q_OBJECT
public:
    GoProjectFile(GoProject *parent, QString fileName);
    ~GoProjectFile() {}

    bool save(QString *errorString, const QString &fileName, bool autoSave) override;
    QString fileName() const;
    void rename(const QString &newName);

    QString defaultPath() const override;
    QString suggestedFileName() const override;
    QString mimeType() const override;

    bool isModified() const override;
    bool isSaveAsAllowed() const override;

    ReloadBehavior reloadBehavior(ChangeTrigger state, ChangeType type) const override;
    bool reload(QString *errorString, ReloadFlag flag, ChangeType type) override;

private:
    GoProject *m_project;
    QString m_fileName;
    
};
}
}

#endif // GOPROJECTFILE_H
