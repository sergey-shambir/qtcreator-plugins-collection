/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
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

#ifndef QMLPROJECTITEM_H
#define QMLPROJECTITEM_H

#include <QObject>
#include <QSet>
#include <QStringList>

namespace GoLang {

class GoProjectContentItem : public QObject {
    // base class for all elements that should be direct children of Project element
    Q_OBJECT

public:
    GoProjectContentItem(QObject *parent = 0) : QObject(parent) {}
};

class GoBaseTargetItemPrivate;

class GoBaseTargetItem : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(GoBaseTargetItem)

    Q_PROPERTY(QString sourceDirectory READ sourceDirectory NOTIFY sourceDirectoryChanged)
    Q_PROPERTY(QStringList importPaths READ importPaths WRITE setImportPaths NOTIFY importPathsChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

public:
    GoBaseTargetItem(QObject *parent = 0);
    ~GoBaseTargetItem();

    QString sourceDirectory() const;
    void setSourceDirectory(const QString &directoryPath);

    QStringList importPaths() const;
    void setImportPaths(const QStringList &paths);

    QStringList files() const;
    bool matchesFile(const QString &filePath) const;

    QString name() const;
    void setName(const QString &name);

    void appendContent(GoProjectContentItem* contentItem);

signals:
    void qmlFilesChanged(const QSet<QString> &, const QSet<QString> &);
    void sourceDirectoryChanged();
    void importPathsChanged();
    void nameChanged();

protected:
    GoBaseTargetItemPrivate *d_ptr;
};

class GoApplicationItem : public GoBaseTargetItem
{
    Q_OBJECT
public:
    GoApplicationItem(QObject *parent = 0);
    ~GoApplicationItem();
};

class GoPackageItem : public GoBaseTargetItem
{
    Q_OBJECT
public:
    GoPackageItem(QObject *parent = 0);
    ~GoPackageItem();
};

class GoProjectItem : public QObject
{
    Q_OBJECT
public:
    GoProjectItem(QObject *parent = 0);

    void appendTarget(GoBaseTargetItem* contentItem);
    QList<GoBaseTargetItem*> commands () const;

    QStringList files() const;
    bool matchesFile(const QString &filePath) const;

    QString sourceDirectory() const;
    void setSourceDirectory(const QString &directoryPath);

signals:
    void filesChanged(const QSet<QString> &, const QSet<QString> &);
    void sourceDirectoryChanged();
    void importPathsChanged();

private:
    QList<GoBaseTargetItem*> m_content;
    QString m_sourceDir;
};

} // namespace QmlProjectManager

#endif // QMLPROJECTITEM_H
