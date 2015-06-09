#include "gotoolchain.h"
#include "golangconstants.h"
#include "toolchainconfigwidget.h"

#include <utils/synchronousprocess.h>
#include <utils/fileutils.h>
#include <utils/environment.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/ansifilterparser.h>
#include <projectexplorer/kitinformation.h>
#include <projectexplorer/toolchain.h>
#include <projectexplorer/task.h>

#include <QProcess>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QDir>
#include <QChar>

namespace GoLang {
namespace Internal {

static const char compilerCommandKeyC[] = "GoLang.GoToolChain.Path";
static const char goRootKeyC[] = "GoLang.GoToolChain.GoRoot";
static const char targetAbiKeyC[] = "GoLang.GoToolChain.TargetAbi";
static const char supportedAbisKeyC[] = "GoLang.GoToolChain.SupportedAbis";

static QByteArray runGo(const Utils::FileName &goGc, const QStringList &arguments, const QStringList &env)
{
    if (goGc.isEmpty() || !goGc.toFileInfo().isExecutable())
        return QByteArray();

    QProcess go;
    // Force locale: This function is used only to detect settings inside the tool chain, so this is save.
    QStringList environment(env);
    environment.append(QLatin1String("LC_ALL=C"));

    go.setEnvironment(environment);
    go.start(goGc.toString(), arguments);
    if (!go.waitForStarted()) {
        qWarning("%s: Cannot start '%s': %s", Q_FUNC_INFO, qPrintable(goGc.toUserOutput()),
                 qPrintable(go.errorString()));
        return QByteArray();
    }
    go.closeWriteChannel();
    if (!go.waitForFinished(10000)) {
        Utils::SynchronousProcess::stopProcess(go);
        qWarning("%s: Timeout running '%s'.", Q_FUNC_INFO, qPrintable(goGc.toUserOutput()));
        return QByteArray();
    }
    if (go.exitStatus() != QProcess::NormalExit) {
        qWarning("%s: '%s' crashed.", Q_FUNC_INFO, qPrintable(goGc.toUserOutput()));
        return QByteArray();
    }

    const QByteArray stdErr = go.readAllStandardError();
    if (go.exitCode() != 0) {
        qWarning().nospace()
                << Q_FUNC_INFO << ": " << goGc.toUserOutput() << ' '
                << arguments.join(QLatin1String(" ")) << " returned exit code "
                << go.exitCode() << ": " << stdErr;
        return QByteArray();
    }

    QByteArray data = go.readAllStandardOutput();
    if (!data.isEmpty() && !data.endsWith('\n'))
        data.append('\n');
    data.append(stdErr);
    return data;
}

static QMap<QString, QString> getGoEnv (const Utils::FileName &goGc, const QStringList &env)
{
    QStringList arguments = QStringList()<<QStringLiteral("env");
    QByteArray output = runGo(goGc,arguments,env);

    QMap<QString, QString> goEnv;
    if(output.isEmpty())
        return goEnv;

    QRegularExpression regExp(QStringLiteral("\\s?(\\w+)\\s?=\\s?\"(.*)\""));

    QList<QByteArray> lines = output.split('\n');
    foreach(const QByteArray &line , lines ) {
        QRegularExpressionMatch match = regExp.match(QString::fromLatin1(line));
        if(!match.hasMatch())
            continue;

        if(match.lastCapturedIndex() != 2)
            continue;

        if(!goEnv.contains(match.captured(1)))
            goEnv.insert(match.captured(1),match.captured(2));
    }
    return goEnv;
}

static QList<ProjectExplorer::Abi> guessGoAbi (const QString &goRoot)
{
    QList<ProjectExplorer::Abi> result;

    QDir pkgDir(goRoot+QStringLiteral("/pkg"));
    if(!pkgDir.exists())
        return QList<ProjectExplorer::Abi>();

    QStringList abis = pkgDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach(const QString &abi,abis) {
        QStringList parts = abi.split(QChar::fromLatin1('_'));

        //only support linux_arch entries for now
        if(parts[0] != QStringLiteral("linux") || parts.size() == 0 || parts.size() > 2)
            continue;

        ProjectExplorer::Abi::Architecture arch = ProjectExplorer::Abi::UnknownArchitecture;
        ProjectExplorer::Abi::OS os = ProjectExplorer::Abi::UnknownOS;
        ProjectExplorer::Abi::OSFlavor flavor = ProjectExplorer::Abi::UnknownFlavor;
        ProjectExplorer::Abi::BinaryFormat format = ProjectExplorer::Abi::UnknownFormat;
        int width = 0;
        int unknownCount = 0;

        //copied from gcc code, need to check how all the architectures are named in go
        foreach (const QString &p, parts) {
            if (p == QLatin1String("386") || p == QLatin1String("486") || p == QLatin1String("586")
                    || p == QLatin1String("686") || p == QLatin1String("x86")) {
                arch = ProjectExplorer::Abi::X86Architecture;
                width = 32;
            } else if (p.startsWith(QLatin1String("arm"))) {
                arch = ProjectExplorer::Abi::ArmArchitecture;
                width = 32;
            } /*else if (p == QLatin1String("mipsel")) {
                arch = ProjectExplorer::Abi::MipsArchitecture;
                width = 32;
            } */else if (p == QLatin1String("x86_64") || p == QLatin1String("amd64")) {
                arch = ProjectExplorer::Abi::X86Architecture;
                width = 64;
            } /*else if (p == QLatin1String("powerpc64")) {
                arch = ProjectExplorer::Abi::PowerPCArchitecture;
                width = 64;
            } else if (p == QLatin1String("powerpc")) {
                arch = ProjectExplorer::Abi::PowerPCArchitecture;
                width = 32;
            } */ else if (p == QLatin1String("linux")) {
                os = ProjectExplorer::Abi::LinuxOS;
                if (flavor == ProjectExplorer::Abi::UnknownFlavor)
                    flavor = ProjectExplorer::Abi::GenericLinuxFlavor;
                format = ProjectExplorer::Abi::ElfFormat;
            } else if (p.startsWith(QLatin1String("freebsd"))) {
                os = ProjectExplorer::Abi::BsdOS;
                if (flavor == ProjectExplorer::Abi::UnknownFlavor)
                    flavor = ProjectExplorer::Abi::FreeBsdFlavor;
                format = ProjectExplorer::Abi::ElfFormat;
            } /*else if (p == QLatin1String("mingw32") || p == QLatin1String("win32") || p == QLatin1String("mingw32msvc")) {
                arch = ProjectExplorer::Abi::X86Architecture;
                os = ProjectExplorer::Abi::WindowsOS;
                flavor = ProjectExplorer::Abi::WindowsMSysFlavor;
                format = ProjectExplorer::Abi::PEFormat;
            } else if (p == QLatin1String("apple")) {
                os = ProjectExplorer::Abi::MacOS;
                flavor = ProjectExplorer::Abi::GenericMacFlavor;
                format = ProjectExplorer::Abi::MachOFormat;
            } */else {
                ++unknownCount;
            }
        }

        if(arch == ProjectExplorer::Abi::UnknownArchitecture)
            continue;

        result.append(ProjectExplorer::Abi(arch, os, flavor, format, width));
    }
    return result;
}

static QList<ProjectExplorer::Abi> guessGoAbi (const Utils::FileName &goGc, const QStringList &env)
{
    QMap<QString, QString> goEnv = getGoEnv(goGc, env);

    QString rootDirKey = QStringLiteral("GOROOT");
    if(goEnv.isEmpty() || !goEnv.contains(rootDirKey))
        return QList<ProjectExplorer::Abi>();

    return guessGoAbi(goEnv[rootDirKey]);
}

GoToolChain::GoToolChain(Detection detect)
    : ToolChain(QLatin1String(Constants::GO_TOOLCHAIN_ID),detect)
{

}

GoToolChain::GoToolChain(const GoToolChain &other)
    : ToolChain(other),
      m_compilerCommand(other.m_compilerCommand),
      m_goRoot(other.m_goRoot),
      m_targetAbi(other.m_targetAbi),
      m_headerPaths(other.m_headerPaths),
      m_supportedAbis(other.m_supportedAbis)
{
}

void GoToolChain::setCompilerCommand(const Utils::FileName &path, const Utils::FileName &goRoot)
{
    if (path == m_compilerCommand && goRoot == m_goRoot)
        return;

    bool resetDisplayName = displayName() == defaultDisplayName();

    m_compilerCommand = path;
    m_goRoot = goRoot;

    ProjectExplorer::Abi currentAbi = m_targetAbi;
    m_supportedAbis = detectSupportedAbis();

    m_targetAbi = ProjectExplorer::Abi();
    if (!m_supportedAbis.isEmpty()) {
        if (m_supportedAbis.contains(currentAbi))
            m_targetAbi = currentAbi;
        else
            m_targetAbi = m_supportedAbis.at(0);
    }

    if (resetDisplayName)
        setDisplayName(defaultDisplayName()); // calls toolChainUpdated()!
    else
        toolChainUpdated();
}

QString GoToolChain::type() const
{
    return QString::fromLatin1(Constants::GO_TOOLCHAIN_ID);
}

QString GoToolChain::typeDisplayName() const
{
    return QStringLiteral("Go toolchain");
}

ProjectExplorer::Abi GoToolChain::targetAbi() const
{
    return m_targetAbi;
}

bool GoToolChain::isValid() const
{
    if (m_compilerCommand.isNull() || m_goRoot.isNull())
        return false;

    QFileInfo fi = compilerCommand().toFileInfo();
    QFileInfo dir = m_goRoot.toFileInfo();
    return fi.isExecutable() && dir.isDir();
}

QList<ProjectExplorer::HeaderPath> GoToolChain::systemGoPaths() const
{
    return QList<ProjectExplorer::HeaderPath>();
}

void GoToolChain::addToEnvironment(Utils::Environment &env) const
{
    addCommandPathToEnvironment(m_compilerCommand,env);
    env.set(QStringLiteral("GOROOT"),goRoot().toString());
    env.set(QStringLiteral("GOOS"),QStringLiteral("linux"));
    env.set(QStringLiteral("GOARCH"),toString(m_targetAbi.architecture(),m_targetAbi.wordWidth()));
}

Utils::FileName GoToolChain::compilerCommand() const
{
    return m_compilerCommand;
}

void GoToolChain::setTargetAbi(const ProjectExplorer::Abi &abi)
{
    if(m_targetAbi == abi)
        return;

    m_targetAbi = abi;
    toolChainUpdated();
}

QList<ProjectExplorer::Abi> GoToolChain::supportedAbis() const
{
    if(m_supportedAbis.isEmpty())
        updateSupportedAbis();
    return m_supportedAbis;
}

Utils::FileName GoToolChain::goRoot() const
{
    return m_goRoot;
}

ProjectExplorer::IOutputParser *GoToolChain::outputParser() const
{
    return 0;
}

ToolChainConfigWidget *GoToolChain::configurationWidget()
{
    return new ToolChainConfigWidget(this);
}

bool GoToolChain::canClone() const
{
    return true;
}

ToolChain *GoToolChain::clone() const
{
    return new GoToolChain(*this);
}

QVariantMap GoToolChain::toMap() const
{
    QVariantMap map = ToolChain::toMap();
    map.insert(QLatin1String(compilerCommandKeyC), m_compilerCommand.toString());
    map.insert(QLatin1String(goRootKeyC), m_goRoot.toString());
    map.insert(QLatin1String(targetAbiKeyC), m_targetAbi.toString());
    QStringList abiList;
    foreach (const ProjectExplorer::Abi &a, m_supportedAbis)
        abiList.append(a.toString());
    map.insert(QLatin1String(supportedAbisKeyC), abiList);
    return map;
}

bool GoToolChain::fromMap(const QVariantMap &data)
{
    if(!ToolChain::fromMap(data))
        return false;

    m_compilerCommand = Utils::FileName::fromString(data.value(QLatin1String(compilerCommandKeyC)).toString());
    m_goRoot = Utils::FileName::fromString(data.value(QLatin1String(goRootKeyC)).toString());
    m_targetAbi = ProjectExplorer::Abi(data.value(QLatin1String(targetAbiKeyC)).toString());
    QStringList abiList = data.value(QLatin1String(supportedAbisKeyC)).toStringList();
    m_supportedAbis.clear();
    foreach (const QString &a, abiList) {
        ProjectExplorer::Abi abi(a);
        if (!abi.isValid())
            continue;
        m_supportedAbis.append(abi);
    }
    return true;
}

QList<ProjectExplorer::Task> GoToolChain::validateKit(const ProjectExplorer::Kit *k) const
{
    QList<ProjectExplorer::Task> result;

    ProjectExplorer::ToolChain *tc = ProjectExplorer::ToolChainKitInformation::toolChain(k);
    if(tc) {
        ProjectExplorer::Abi targetAbi = tc->targetAbi();
        bool compMatch = targetAbi.isCompatibleWith(m_targetAbi);
        bool fullMatch = (targetAbi == m_targetAbi);

        QString message;
        if (!fullMatch) {
            if (!compMatch)
                message = QCoreApplication::translate("GoLang::GoToolChain",
                                                      "The compiler '%1' (%2) cannot produce code for the Go version '%3' (%4).");
            else
                message = QCoreApplication::translate("GoLang::GoToolChain",
                                                      "The compiler '%1' (%2) may not produce code compatible with the Go version '%3' (%4).");
            message = message.arg(tc->displayName(), targetAbi.toString(),
                                  displayName(), m_targetAbi.toString());
            result << ProjectExplorer::Task(compMatch ? ProjectExplorer::Task::Warning : ProjectExplorer::Task::Error, message, Utils::FileName(), -1,
                           ProjectExplorer::Constants::TASK_CATEGORY_BUILDSYSTEM);
        }
    }
    return result;
}

QString GoToolChain::defaultDisplayName() const
{
    if (!m_targetAbi.isValid())
        return typeDisplayName();
    return QCoreApplication::translate("GoLang::GoToolChain",
                                       "%1 (%2 %3 in %4)").arg(typeDisplayName(),
                                                               ProjectExplorer::Abi::toString(m_targetAbi.architecture()),
                                                               ProjectExplorer::Abi::toString(m_targetAbi.wordWidth()),
                                                               compilerCommand().parentDir().toUserOutput());
}

void GoToolChain::updateSupportedAbis() const
{
    if (m_supportedAbis.isEmpty())
        m_supportedAbis = detectSupportedAbis();
}

QList<ProjectExplorer::Abi> GoToolChain::detectSupportedAbis() const
{
    Utils::Environment env = Utils::Environment::systemEnvironment();
    addToEnvironment(env);
    return guessGoAbi(m_compilerCommand, env.toStringList());
}

void GoToolChain::addCommandPathToEnvironment(const Utils::FileName &command, Utils::Environment &env)
{
    if (!command.isEmpty())
        env.prependOrSetPath(command.parentDir().toString());
}

QString GoToolChain::toString(ProjectExplorer::Abi::Architecture arch, int width)
{
    switch (arch) {
        case ProjectExplorer::Abi::ArmArchitecture:
            return QStringLiteral("arm");
        case ProjectExplorer::Abi::X86Architecture:{
            if(width == 64)
                return QStringLiteral("amd64");
            return QStringLiteral("386");
        }
        case ProjectExplorer::Abi::UnknownArchitecture: // fall through!
        default:
            return QLatin1String("unknown");
    }
}

GoToolChainFactory::GoToolChainFactory()
{
    setId(Constants::GO_TOOLCHAIN_ID);
    setDisplayName(QStringLiteral("Go Toolchain"));
}

GoToolChain *GoToolChainFactory::createToolChain(bool autoDetect)
{
    return new GoToolChain(autoDetect ? ToolChain::AutoDetection : ToolChain::ManualDetection);
}

QList<GoLang::ToolChain *> GoToolChainFactory::autoDetect()
{

    QList<ToolChain *> result;

    QString compiler = QStringLiteral("go");
    Utils::Environment systemEnvironment = Utils::Environment::systemEnvironment();
    const Utils::FileName compilerPath = Utils::FileName::fromString(systemEnvironment.searchInPath(compiler));
    if (compilerPath.isEmpty())
        return result;

    GoToolChain::addCommandPathToEnvironment(compilerPath, systemEnvironment);

    QString rootKey = QStringLiteral("GOROOT");
    QMap<QString, QString> goEnv = getGoEnv(compilerPath,systemEnvironment.toStringList());
    if(!goEnv.contains(rootKey))
        return result;

    QList<ProjectExplorer::Abi> abiList = guessGoAbi(goEnv[rootKey]);
    foreach (const ProjectExplorer::Abi &abi, abiList) {
        QScopedPointer<GoToolChain> tc(createToolChain(true));
        if (tc.isNull())
            return result;

        tc->setCompilerCommand(compilerPath,Utils::FileName::fromString(goEnv[rootKey]));
        tc->setTargetAbi(abi);
        tc->setDisplayName(tc->defaultDisplayName()); // reset displayname

        result.append(tc.take());
    }
    return result;
}

bool GoToolChainFactory::canCreate()
{
    return true;
}

ToolChain *GoToolChainFactory::create()
{
    return createToolChain(false);
}

bool GoToolChainFactory::canRestore(const QVariantMap &data)
{
    return idFromMap(data).startsWith(QLatin1String(Constants::GO_TOOLCHAIN_ID) + QLatin1Char(':'));
}

ToolChain *GoToolChainFactory::restore(const QVariantMap &data)
{
    if(!canRestore(data))
        return 0;

    //use Manual detection here it will be overwritten anyway
    GoToolChain *tc = new GoToolChain(ToolChain::ManualDetection);
    if (tc->fromMap(data))
        return tc;

    delete tc;
    return 0;
}

} // namespace Internal
} // namespace GoLang
