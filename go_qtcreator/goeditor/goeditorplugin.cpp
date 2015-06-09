#include "goeditorplugin.h"
#include "goeditorconstants.h"
#include "goeditorfactory.h"
#include "goeditorwidget.h"
#include "tools/gocompletionassist.h"

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/mimedatabase.h>
#include <coreplugin/fileiconprovider.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <texteditor/texteditorsettings.h>

#include <QAction>
#include <QMenu>

#include <QtPlugin>

using namespace GoEditor::Internal;
using namespace GoEditor::Constants;

/*******************************************************************************
 * List of Go keywords
 ******************************************************************************/
static const char *const LIST_OF_GO_KEYWORDS[] = {
    "break",
    "case",
    "chan",
    "const",
    "continue",
    "default",
    "defer",
    "else",
    "fallthrough",
    "for",
    "func",
    "go",
    "goto",
    "if",
    "import",
    "interface",
    "map",
    "package",
    "range",
    "return",
    "select",
    "struct",
    "switch",
    "type",
    "var"
};


/*******************************************************************************
 * List of Go predeclarated entities.
 * See http://golang.org/ref/spec#Iota
 ******************************************************************************/

static const char *const LIST_OF_GO_TYPES[] = {
    "bool",
    "byte",
    "complex64",
    "complex128",
    "error",
    "float32",
    "float64",
    "int",
    "int8",
    "int16",
    "int32",
    "int64",
    "rune",
    "string",
    "uint",
    "uint8",
    "uint16",
    "uint32",
    "uint64",
    "uintptr"
};

static const char *const LIST_OF_GO_CONSTANTS[] = {
    "true",
    "false",
    "iota",
    "nil"
};

static const char *const LIST_OF_GO_FUNCS[] = {
    "append",
    "cap",
    "close",
    "complex",
    "copy",
    "delete",
    "imag",
    "len",
    "make",
    "new",
    "panic",
    "print",
    "println",
    "real",
    "recover"
};

/// Copies identifiers from array to QSet
static void copyIdentifiers(const char * const words[], size_t bytesCount, QSet<QString> &result)
{
    const size_t count = bytesCount / sizeof(const char * const);
    for (size_t i = 0; i < count; ++i)
        result.insert(QLatin1String(words[i]));
}

GoEditorPlugin *GoEditorPlugin::m_instance = NULL;

GoEditorPlugin::GoEditorPlugin()
{
    m_instance = this;
    copyIdentifiers(LIST_OF_GO_KEYWORDS, sizeof(LIST_OF_GO_KEYWORDS), m_goKeywords);
    copyIdentifiers(LIST_OF_GO_TYPES, sizeof(LIST_OF_GO_TYPES), m_goPredeclaratedTypes);
    copyIdentifiers(LIST_OF_GO_CONSTANTS, sizeof(LIST_OF_GO_CONSTANTS), m_goPredeclaratedConsts);
    copyIdentifiers(LIST_OF_GO_FUNCS, sizeof(LIST_OF_GO_FUNCS), m_goPredeclaratedFuncs);
}

GoEditorPlugin::~GoEditorPlugin()
{
    removeObject(m_factory);
    m_instance = 0;
}

bool GoEditorPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

    if (!Core::MimeDatabase::addMimeTypes(QLatin1String(RC_GO_MIMETYPE_XML), errorString))
        return false;

    m_factory = new GoEditorFactory(this);
    addObject(m_factory);

    addAutoReleasedObject(new GoCompletionAssistProvider);

    m_actionHandler.reset(new TextEditor::TextEditorActionHandler(
                              this,
                              C_GOEDITOR_ID,
                              TextEditor::TextEditorActionHandler::Format
                              | TextEditor::TextEditorActionHandler::UnCommentSelection
                              | TextEditor::TextEditorActionHandler::UnCollapseAll));

    // Add MIME overlay icons (these icons displayed at Project dock panel)
    const QIcon icon = QIcon::fromTheme(QLatin1String(C_GO_MIME_ICON));
    if (!icon.isNull())
        Core::FileIconProvider::registerIconOverlayForMimeType(icon, C_GO_MIMETYPE);

    return true;
}

void GoEditorPlugin::extensionsInitialized()
{
}

ExtensionSystem::IPlugin::ShutdownFlag GoEditorPlugin::aboutToShutdown()
{
    return SynchronousShutdown;
}

QSet<QString> GoEditorPlugin::goKeywords()
{
    return m_instance->m_goKeywords;
}

QSet<QString> GoEditorPlugin::goPredeclaratedTypes()
{
    return m_instance->m_goPredeclaratedTypes;
}

QSet<QString> GoEditorPlugin::goPredeclaratedConsts()
{
    return m_instance->m_goPredeclaratedConsts;
}

QSet<QString> GoEditorPlugin::goPredeclaratedFuncs()
{
    return m_instance->m_goPredeclaratedFuncs;
}

Q_EXPORT_PLUGIN2(GoEditor, GoEditorPlugin)

