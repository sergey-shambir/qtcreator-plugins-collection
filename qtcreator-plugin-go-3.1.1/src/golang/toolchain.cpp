#include "toolchain.h"
#include "toolchainmanager.h"
#include "toolchainconfigwidget.h"
#include "golangconstants.h"

#include <utils/fileutils.h>
#include <projectexplorer/projectconfiguration.h>
#include <projectexplorer/abi.h>
#include <coreplugin/id.h>


#include <QCoreApplication>
#include <QUuid>

static const char ID_KEY[] = "GoLang.ToolChain.Id";
static const char DISPLAY_NAME_KEY[] = "GoLang.ToolChain.DisplayName";
static const char AUTODETECT_KEY[] = "GoLang.ToolChain.Autodetect";

namespace GoLang {

namespace Internal {

// --------------------------------------------------------------------------
// ToolChainPrivate
// --------------------------------------------------------------------------

class ToolChainPrivate
{
public:
    typedef ToolChain::Detection Detection;

    explicit ToolChainPrivate(const QString &id, Detection d) :
        m_detection(d)
    {
        m_id = createId(id);
    }

    static QString createId(const QString &id)
    {
        QString newId = id.left(id.indexOf(QLatin1Char(':')));
        newId.append(QLatin1Char(':') + QUuid::createUuid().toString());
        return newId;
    }

    QString m_id;
    Detection m_detection;
    mutable QString m_displayName;
};

} // namespace Internal



/*!
    \class GoLang::ToolChain
    \brief The ToolChain class represents a go tool chain.
    \sa GoLang::ToolChainManager
*/

// --------------------------------------------------------------------------

ToolChain::ToolChain(const QString &id, Detection d) :
    d(new Internal::ToolChainPrivate(id, d))
{ }

ToolChain::ToolChain(const ToolChain &other) :
    d(new Internal::ToolChainPrivate(other.d->m_id, ManualDetection))
{
    // leave the autodetection bit at false.
    d->m_displayName = QCoreApplication::translate("ProjectExplorer::ToolChain", "Clone of %1")
            .arg(other.displayName());
}

/*!
    Used by the tool chain manager to load user-generated tool chains.

    Make sure to call this function when deriving.
*/
bool ToolChain::fromMap(const QVariantMap &data)
{
    d->m_displayName = data.value(QLatin1String(DISPLAY_NAME_KEY)).toString();
    // make sure we have new style ids:
    d->m_id = data.value(QLatin1String(ID_KEY)).toString();
    const bool autoDetect = data.value(QLatin1String(AUTODETECT_KEY), false).toBool();
    d->m_detection = autoDetect ? AutoDetectionFromSettings : ManualDetection;

    return true;
}

ToolChain::~ToolChain()
{
    delete d;
}

QString ToolChain::displayName() const
{
    if (d->m_displayName.isEmpty())
        return typeDisplayName();
    return d->m_displayName;
}

void ToolChain::setDisplayName(const QString &name)
{
    if (d->m_displayName == name)
        return;

    d->m_displayName = name;
    toolChainUpdated();
}

ToolChain::Detection ToolChain::detection() const
{
    return d->m_detection;
}

QString ToolChain::id() const
{
    return d->m_id;
}

bool ToolChain::canClone() const
{
    return true;
}

/*!
    Used by the tool chain manager to save user-generated tool chains.

    Make sure to call this function when deriving.
*/
QVariantMap ToolChain::toMap() const
{
    QVariantMap result;
    result.insert(QLatin1String(ID_KEY), id());
    result.insert(QLatin1String(DISPLAY_NAME_KEY), displayName());
    result.insert(QLatin1String(AUTODETECT_KEY), isAutoDetected());

    return result;
}

/*!
    Used by the tool chain kit information to validate the kit.
*/
QList<ProjectExplorer::Task> ToolChain::validateKit(const ProjectExplorer::Kit *) const
{
    return QList<ProjectExplorer::Task>();
}

bool ToolChain::operator ==(const ToolChain &tc) const
{
    if (this == &tc)
        return true;

    const QString thisId = id().left(id().indexOf(QLatin1Char(':')));
    const QString tcId = tc.id().left(tc.id().indexOf(QLatin1Char(':')));

    // We ignore displayname
    return thisId == tcId && isAutoDetected() == tc.isAutoDetected();
}

void ToolChain::setDetection(ToolChain::Detection detect)
{
    d->m_detection = detect;
}

void ToolChain::toolChainUpdated()
{
    ToolChainManager::notifyAboutUpdate(this);
}

QString ToolChainFactory::idFromMap(const QVariantMap &data)
{
    return data.value(QLatin1String(ID_KEY)).toString();
}

void ToolChainFactory::idToMap(QVariantMap &data, const QString id)
{
    data.insert(QLatin1String(ID_KEY), id);
}

void ToolChainFactory::autoDetectionToMap(QVariantMap &data, bool detected)
{
    data.insert(QLatin1String(AUTODETECT_KEY), detected);
}

} // namespace GoLang
