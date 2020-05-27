///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author    Dr. Thomas Leitner, t.leitner@avibit.com
    \author    QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief
*/

#include "avenvironment.h"

// Qt inlcudes
#include <qdir.h>
#include <QSet>

// avlib includes
#include "avconfig.h" // TODO CM remove after QTP-10 has been resolved
#include "avconfig2.h"
#include "avpackageinfo.h"

///////////////////////////////////////////////////////////////////////////////

const QString AVEnvironment::APP_ASTOS    = "ASTOS";
const QString AVEnvironment::APP_AVCOMMON = "AVCOMMON";
const QString AVEnvironment::APP_AVIMET   = "AVIMET";
const QString AVEnvironment::APP_OPTAMOS  = "OPTAMOS";
const QString AVEnvironment::APP_TBS      = "TBS";
const QString AVEnvironment::APP_DIFLIS   = "DIFLIS";
const QString AVEnvironment::APP_MUSTRAC  = "MUSTRAC";
const QString AVEnvironment::APP_DECLOS   = "DECLOS";
const QString AVEnvironment::APP_DMAN     = "DMAN";
const QString AVEnvironment::APP_MLAT     = "MLAT";
const QString AVEnvironment::APP_INFOMAX  = "INFOMAX";
const QString AVEnvironment::APP_IM2      = "IM2";
const QString AVEnvironment::APP_HDPS     = "HDPS";
const QString AVEnvironment::APP_VVRMAX   = "VVRMAX";
const QString AVEnvironment::APP_SAFECONTROL = "SAFECONTROL";
const QString AVEnvironment::APP_SAM      = "SAM";
const QString AVEnvironment::APP_ADMAX    = "ADMAX";

#if defined (_OS_WIN32_) || defined(Q_OS_WIN32)
const QChar AVEnvironment::PATH_SEPARATOR = ';';
#else
const QChar AVEnvironment::PATH_SEPARATOR = ':';
#endif

QString AVEnvironment::process_name             = "";
QString AVEnvironment::application_name         = "";
QString AVEnvironment::application_home         = "";
QString AVEnvironment::application_bin          = "";
QString AVEnvironment::application_config       = "";
QString AVEnvironment::application_log          = "";
QString AVEnvironment::application_run          = "";
QString AVEnvironment::application_maps         = "";
QString AVEnvironment::application_data         = "";
QString AVEnvironment::application_cache        = "";
QString AVEnvironment::application_title        = "";
QString AVEnvironment::application_info         = "";

///////////////////////////////////////////////////////////////////////////////

void AVEnvironment::setApplicationName(const QString &dan, int argc, char **argv)
{
    // This may be set only once...
    if (AVConfigBase::process_uses_avconfig2) AVASSERTNOLOGGER(application_name.isEmpty());

    application_name = dan;

    bool application_name_overridden = false;

    if (!AVConfigBase::process_uses_avconfig2)
    {
        //----- override with potential -an parameter
        QStringList& params = AVConfigBase::params(argc, argv);
    int index = 1;
        while (index < params.count() - 1)
        {
            if (params[index] == "-an")
            {
            params.removeAt(index);
                application_name = params[index];
                application_name_overridden = true;
            params.removeAt(index);
            }
            else if (params[index] == "-cfgdir")
            {
            params.removeAt(index);
                application_config = params[index];
            params.removeAt(index);
            }
            else
            {
                ++index;
            }
        }
    }

    //----- override with potential APPLICATION_NAME environment parameter

    if (!application_name_overridden)
    {
        QString env_application_name = getEnv("APPLICATION_NAME", false, NoPrefix);
        if (!env_application_name.isEmpty())
        {
            application_name = env_application_name;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVEnvironment::setApplicationConfig(const QString& cfgdir)
{
    application_config = cfgdir;
}

///////////////////////////////////////////////////////////////////////////////

void AVEnvironment::setApplicationTitle(const QString &applicationTitle)
{
    application_title = applicationTitle;
}

///////////////////////////////////////////////////////////////////////////////

void AVEnvironment::setApplicationInfo(const QString &applicationInfo)
{
    application_info = applicationInfo;
}

///////////////////////////////////////////////////////////////////////////////

QString AVEnvironment::getProcessName()
{
    if (process_name.isEmpty())
    {
        qFatal("ERROR: request for process name, "
               "but it has not been set");
        return QString();
    }

    return process_name;
}

///////////////////////////////////////////////////////////////////////////////

QString AVEnvironment::getApplicationName()
{
    if (application_name.isEmpty())
    {
        qFatal("ERROR: request for application name, "
               "but it has not been set");
        return QString();
    }

    return application_name;
}

///////////////////////////////////////////////////////////////////////////////

QString AVEnvironment::getApplicationTitle()
{
    if (application_title.isEmpty())
    {
        qFatal("ERROR: request for application title, "
               "but it has not been set");
        return QString();
    }

    return application_title;
}

///////////////////////////////////////////////////////////////////////////////

QString AVEnvironment::getApplicationInfo()
{
    if (application_info.isEmpty())
    {
        qFatal("ERROR: request for application information, "
               "but it has not been set");
        return QString();
    }

    return application_info;
}
///////////////////////////////////////////////////////////////////////////////

QString AVEnvironment::getApplicationCaption(const QString& appname)
{
    QString caption;
    if (appname.isEmpty())
        caption = getApplicationTitle();
    else
        caption = appname;

    caption += " ";
    caption += AVPackageInfo::getVersion();
    caption += "-";
    caption += AVPackageInfo::getRevision();

    return caption;
}

///////////////////////////////////////////////////////////////////////////////

QString AVEnvironment::getApplicationHome()
{
    if (application_name.isEmpty())
    {
        qFatal("ERROR: request for application home path, "
               "but app name has not been set");
        return QString();
    }

    if (!application_home.isEmpty())
        return application_home;

    application_home = QDir(getEnv("HOME", true)).absolutePath();
    return application_home;
}

///////////////////////////////////////////////////////////////////////////////

void AVEnvironment::setApplicationHome(const QString& directory)
{
    application_bin =
        application_config =
            application_log =
                application_run =
                    application_maps =
                        application_data =
                            application_cache =
                                application_title =
                                    application_info =
                                        QString::null;

    AVASSERT(!directory.isEmpty());
    application_home = directory;

    AVLogInfo << "AVEnvironment::setApplicationHome: "
        "manually setting application home directory to: " << application_home;
}

///////////////////////////////////////////////////////////////////////////////

QString AVEnvironment::getApplicationBin()
{
    if (application_name.isEmpty())
    {
        qFatal("ERROR: request for application bin path, "
               "but app name has not been set");
        return QString();
    }

    if (!application_bin.isEmpty())
        return application_bin;

    QString p = getEnv("BIN", false);
    if (p.isNull()) {
        QString ld = getApplicationHome();
        if (!ld.isNull()) return ld + "/bin";
        return ld;
    }
    application_bin = QDir(p).absolutePath();
    return application_bin;
}

///////////////////////////////////////////////////////////////////////////////

QString AVEnvironment::getApplicationConfig()
{
    if (application_name.isEmpty())
    {
        qFatal("ERROR: request for application config path, "
               "but app name has not been set");
        return QString();
    }

    if (!application_config.isEmpty())
        return application_config;

    QString p = getEnv("CONFIG", false);
    if (p.isNull()) {
        QString ld = getApplicationHome();
        if (!ld.isNull()) return ld + "/config";
        return ld;
    }
    application_config = QDir(p).absolutePath();
    return application_config;
}

///////////////////////////////////////////////////////////////////////////////

QString AVEnvironment::getApplicationLog()
{
    if (application_name.isEmpty())
    {
        qFatal("ERROR: request for application log path, "
               "but app name has not been set");
        return QString();
    }

    if (!application_log.isEmpty()) {
        createDirectoryIfNonExistent(application_log);
        return application_log;
    }

    QString p = getEnv("LOG", false);
    if (p.isNull()) {
        QString ld = getApplicationHome();
        if (!ld.isNull()) ld = ld + "/log";
        createDirectoryIfNonExistent(ld);
        return ld;
    }
    application_log = QDir(p).absolutePath();
    createDirectoryIfNonExistent(application_log);
    return application_log;
}

///////////////////////////////////////////////////////////////////////////////

QString AVEnvironment::getApplicationRun()
{
    if (application_name.isEmpty())
    {
        qFatal("ERROR: request for application run path, "
               "but app name has not been set");
        return QString();
    }

    if (!application_run.isEmpty()) {
        createDirectoryIfNonExistent(application_run);
        return application_run;
    }

    QString p = getEnv("RUN", false);
    if (p.isNull()) {
        QString ld = getApplicationHome();
        if (!ld.isNull()) ld = ld + "/run";
        createDirectoryIfNonExistent(ld);
        return ld;
    }
    application_run = QDir(p).absolutePath();
    createDirectoryIfNonExistent(application_run);
    return application_run;
}

///////////////////////////////////////////////////////////////////////////////

QString AVEnvironment::getApplicationData()
{
    if (application_name.isEmpty())
    {
        qFatal("ERROR: request for application data path, "
               "but app name has not been set");
        return QString();
    }

    if (!application_data.isEmpty()) {
        createDirectoryIfNonExistent(application_data);
        return application_data;
    }

    QString p = getEnv("DATA", false);
    if (p.isNull()) {
        QString ld = getApplicationHome();
        if (!ld.isNull()) ld = ld + "/data";
        createDirectoryIfNonExistent(ld);
        return ld;
    }
    application_data = QDir(p).absolutePath();
    createDirectoryIfNonExistent(application_data);
    return application_data;
}

///////////////////////////////////////////////////////////////////////////////

QString AVEnvironment::getApplicationCache()
{
    if (application_name.isEmpty())
    {
        qFatal("ERROR: request for application cache path, "
               "but app name has not been set");
        return QString();
    }

    if (!application_cache.isEmpty()) {
        createDirectoryIfNonExistent(application_cache);
        return application_cache;
    }

    QString p = getEnv("CACHE", false);
    if (p.isNull()) {
        QString ld = getApplicationHome();
        if (!ld.isNull()) ld = ld + "/cache";
        createDirectoryIfNonExistent(ld);
        return ld;
    }
    application_cache = QDir(p).absolutePath();
    createDirectoryIfNonExistent(application_cache);
    return application_cache;
}

///////////////////////////////////////////////////////////////////////////////

QString AVEnvironment::getApplicationMaps()
{
    if (application_name.isEmpty())
    {
        qFatal("ERROR: request for application map path, "
               "but app name has not been set");
        return QString();
    }

    if (!application_maps.isEmpty())
        return application_maps;

    QString p = getEnv("MAPS", false);
    if (p.isNull()) {
        QString ld = getApplicationHome();
        if (!ld.isNull()) return ld + "/maps";
        return ld;
    }
    application_maps = QDir(p).absolutePath();
    return application_maps;
}

///////////////////////////////////////////////////////////////////////////////

QString AVEnvironment::findMap(const QString &path)
{
    return findMap(AVPath(path));
}

///////////////////////////////////////////////////////////////////////////////

QString AVEnvironment::findMap(const AVPath &path)
{
    const QString expanded_path = path.expandPath();

    if (expanded_path.isEmpty())
        return path.path(); // in case a not defined env-variable is used but no file is specified

    const QFileInfo file(expanded_path);
    if (!file.isRelative())
        return expanded_path;

    QStringList results;
    if (file.exists())
        results.append(file.absoluteFilePath());

    const QString maps_dir = getApplicationMaps();
    if (!maps_dir.isNull())
    {
        const QFileInfo map_file(maps_dir + "/" + expanded_path);
        if (map_file.exists())
            results.append(map_file.absoluteFilePath());
    }

    if (AVConfigBase::process_uses_avconfig2)
    {
        const QFileInfo map_file(AVConfig2Global::singleton().findUserConfig(expanded_path));
        if (map_file.exists())
            results.append(map_file.absoluteFilePath());
    }

    if (results.isEmpty())
    {
        return expanded_path;
    }
    else
    {
        const QString result = results.first();
        results.removeDuplicates();

        if (results.size() > 1)
        {
            AVLogError << "AVEnvironment::findMap: found ambiguous paths " << results
                       << " for '" << path << "' using '" << result << "'.";

        }
        return result;
    }
}

///////////////////////////////////////////////////////////////////////////////

QString AVEnvironment::getProjectsHome()
{
    QString p = getEnv("PROJECTS_HOME", false, NoPrefix);
    if (p.isNull()) return QString::null;
    return QDir(p).absolutePath();
}

///////////////////////////////////////////////////////////////////////////////

QString AVEnvironment::getEnv(const QString &var, bool verbose, EnvVarPrefix prefix)
{
    QString env_var = var;
    switch (prefix)
    {
    case AppNamePrefix:
    {
        env_var.prepend(application_name + "_");
        break;
    }
    case ProcessNamePrefix:
    {
        env_var.prepend(process_name.toUpper() + "_");
        break;
    }
    case NoPrefix:
        break;
    default:
       AVASSERT(false);
    }
    QByteArray q = qgetenv(env_var.toLatin1().constData());
    if (!q.isNull()) {
        return QString::fromLocal8Bit(q);
    } else {
        if (verbose) {
            qWarning() << "WARNING: could not read environment variable " << env_var;
        }
        return QString();
    }
}

///////////////////////////////////////////////////////////////////////////////

bool AVEnvironment::setEnv(const QString& var_name,
                      const QString& var_value,
                      bool overwrite)
{
    QByteArray variable_name = var_name.toLatin1();
    if (!overwrite && qEnvironmentVariableIsSet(variable_name.data()))  return true;
    return qputenv(variable_name.data(), var_value.toUtf8().data());
}

///////////////////////////////////////////////////////////////////////////////

bool AVEnvironment::extendPath(const QString &new_path, const QString &variable_name)
{
    QStringList path_list = getPath(variable_name);
    if (path_list.contains(new_path))
    {
        return false;
    }
    path_list.push_back(new_path);
    setEnv(variable_name, path_list.join(PATH_SEPARATOR), true);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

QStringList AVEnvironment::getPath(const QString &variable_name)
{
    return getEnv(variable_name, false, NoPrefix).split(PATH_SEPARATOR, QString::SkipEmptyParts);
}

///////////////////////////////////////////////////////////////////////////////

QStringList AVEnvironment::getAllApplicationNames()
{
    QStringList appnames;
    appnames.push_back(APP_ASTOS);
    appnames.push_back(APP_AVCOMMON);
    appnames.push_back(APP_AVIMET);
    appnames.push_back(APP_OPTAMOS);
    appnames.push_back(APP_TBS);
    appnames.push_back(APP_DIFLIS);
    appnames.push_back(APP_MUSTRAC);
    appnames.push_back(APP_DECLOS);
    appnames.push_back(APP_DMAN);
    appnames.push_back(APP_MLAT);
    appnames.push_back(APP_INFOMAX);
    appnames.push_back(APP_IM2);
    appnames.push_back(APP_HDPS);
    appnames.push_back(APP_VVRMAX);
    appnames.push_back(APP_SAM);
    appnames.push_back(APP_ADMAX);
    return appnames;
}

///////////////////////////////////////////////////////////////////////////////

void AVEnvironment::createDirectoryIfNonExistent(const QString &dname)
{
    QDir dir(dname);
    if (dir.exists()) return;
    if (!dir.mkdir(dname))
    {
        if (AVLogger == nullptr)
        {
            std::cerr << "** ERROR: AVEnvironment::createDirectoryIfNonExistent: failed to create "
                      << qPrintable(dname) << std::endl;
        } else
        {
            AVLogError << AV_FUNC_PREAMBLE << "failed to create " << dname;
        }
    }
}

// end of file
