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

#include "goprojectfile.h"

using namespace GoLang::Internal;

GoProjectFile::GoProjectFile(GoProject *parent, QString fileName)
    : Core::IDocument(parent),
      m_project(parent),
      m_fileName(fileName) {
    QTC_CHECK(m_project);
    QTC_CHECK(!fileName.isEmpty());
    setFilePath(fileName);
}

bool GoProjectFile::save(QString *, const QString &, bool) {
    return false;
}

void GoProjectFile::rename(const QString &newName) {
    // Can't happen...
    Q_UNUSED(newName);
    Q_ASSERT(false);
}

QString GoProjectFile::fileName() const {
    return m_fileName;
}

QString GoProjectFile::defaultPath() const {
    return QString();
}

QString GoProjectFile::suggestedFileName() const {
    return QString();
}

QString GoProjectFile::mimeType() const {
    return QLatin1String(Constants::GO_PROJECT_MIMETYPE);
}

bool GoProjectFile::isModified() const {
    return false;
}

bool GoProjectFile::isSaveAsAllowed() const {
    return false;
}

Core::IDocument::ReloadBehavior GoProjectFile::reloadBehavior(ChangeTrigger state, ChangeType type) const {
    Q_UNUSED(state)
    Q_UNUSED(type)
    return BehaviorSilent;
}

bool GoProjectFile::reload(QString *errorString, ReloadFlag flag, ChangeType type) {
    Q_UNUSED(errorString)
    Q_UNUSED(flag)

    if (type == TypeContents)
        m_project->refreshProjectFile();

    return true;
}
