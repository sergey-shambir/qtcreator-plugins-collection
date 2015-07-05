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
#include "goeditor_global.h"
#include <extensionsystem/iplugin.h>
#include <texteditor/texteditoractionhandler.h>
#include <QScopedPointer>
#include <QSet>

namespace GoEditor {
namespace Internal {

class GoEditorFactory;
class GoEditorWidget;

class GoEditorPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "GoEditor.json")

public:
    GoEditorPlugin();
    ~GoEditorPlugin();

    bool initialize(const QStringList &arguments, QString *errorString) override;
    void extensionsInitialized() override;
    ShutdownFlag aboutToShutdown() override;

    static QSet<QString> goKeywords();
    static QSet<QString> goPredeclaratedTypes();
    static QSet<QString> goPredeclaratedConsts();
    static QSet<QString> goPredeclaratedFuncs();
    static void reportToolNotInstalled(const QString &toolCommand);

signals:
    void reportedError(QString errorMessage);

private slots:
    void reportErrorOnce(const QString &errorMessage);

private:
    static GoEditorPlugin *m_instance;
    GoEditorFactory *m_factory;
    QSet<QString> m_goKeywords;
    QSet<QString> m_goPredeclaratedTypes;
    QSet<QString> m_goPredeclaratedConsts;
    QSet<QString> m_goPredeclaratedFuncs;
    QSet<QString> m_reportedErrors;
};

} // namespace Internal
} // namespace GoEditor

