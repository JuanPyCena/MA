///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Andreas Schuller, a.schuller@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Implementation of the AVPath class
*/

// Qt includes
#include <QFileInfo>

// local includes
#include "avenvironment.h"
#include "avpath.h"


///////////////////////////////////////////////////////////////////////////////

AVPath::AVPath(const QString& path)
    :AVEnvString(path)
{
    initialize();
}

///////////////////////////////////////////////////////////////////////////////

void AVPath::initialize()
{
    m_env_search_rule << LOG << MAPS << CONFIG  << DATA << RUN << HOME << PROJECTS_HOME << USR_HOME;
}

///////////////////////////////////////////////////////////////////////////////

QString AVPath::expandPath() const
{
    QString expanded_path = expandEnvString();
    return QDir::cleanPath(expanded_path);
}

///////////////////////////////////////////////////////////////////////////////

bool AVPath::getAppEnv(const QString& suffix, QString& value) const
{
    if(suffix == "HOME")
    {
        value = AVEnvironment::getApplicationHome();
        return true;
    }

    if(suffix == "DATA")
    {
        value = AVEnvironment::getApplicationData();
        return true;
    }

    if(suffix == "CONFIG")
    {
        value = AVEnvironment::getApplicationConfig();
        return true;
    }

    if(suffix == "MAPS")
    {
        value = AVEnvironment::getApplicationMaps();
        return true;
    }

    if(suffix == "LOG")
    {
        value = AVEnvironment::getApplicationLog();
        return true;
    }

    if(suffix == "RUN")
    {
        value = AVEnvironment::getApplicationRun();
        return true;
    }
    if(suffix == "CACHE")
    {
        value = AVEnvironment::getApplicationCache();
        return true;
    }

    value = QString::null;
    AVLogError << "AVPath::getAppEnv: unsupported environment variable: "
               << QString(AVEnvironment::getApplicationName() + "_" + suffix);
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool AVPath::getEnv(const QString& name, QString& value) const
{
    bool rc = false;

    int last_underscore_index = name.lastIndexOf('_');
    if(last_underscore_index != -1 && name.left(last_underscore_index) == AVEnvironment::getApplicationName())
    {
        // Remainder = APPNAME + '_'
        QString suffix = name.mid(last_underscore_index + 1);
        rc = getAppEnv(suffix, value);
    }
    else if(name == "HOME")
    {
        value = QDir::homePath();
        return true;
    }
    else
    {
        value = AVEnvironment::getEnv(name, true, AVEnvironment::NoPrefix);
        rc = !value.isNull();
    }

    //call QDir::cleanDirPath here to clean the path contained in envVar. It does also
    //"slashify" the path which would otherwise contain platform dependent dir separators.
    value = QDir::cleanPath(value);

    return rc;
}

///////////////////////////////////////////////////////////////////////////////

QString AVPath::pathRelativeToAppEnv(AppEnv* matched_env) const
{
    AppEnvList::const_iterator it  = m_env_search_rule.begin();
    AppEnvList::const_iterator end = m_env_search_rule.end();

    QString relative_path;
    relative_path.reserve(20);

    // Return the unmodified path if the path is relative (cannot be made relative without knowing
    // the base dir of m_path).
    if (QFileInfo(m_env_string).isRelative())
    {
        return m_env_string;
    }

    QString env;
    QString path = QDir(m_env_string).canonicalPath();

    for(; it != end; ++it)
    {
        QString mapped_env = QString(mapEnv(*it));

        if((*it) == USR_HOME)
        {
            env = QDir::homePath();

            if(path.startsWith(env))
            {
                relative_path += "$(HOME)";
            }
            else
                continue;
        } else if((*it) == PROJECTS_HOME)
        {
            env = QDir(AVEnvironment::getProjectsHome()).canonicalPath();
            if(path.startsWith(env))
            {
                relative_path += "$(PROJECTS_HOME)";
            }
            else
                continue;
        }
        else if(getAppEnv(mapped_env, env))
        {
            env = QDir(env).canonicalPath();

            if (env.isEmpty()) continue;

            if (path.startsWith(env))
            {
                relative_path += "$(";
                relative_path.append(AVEnvironment::getApplicationName() + "_");
                relative_path.append(mapped_env + ")");
            }
            else
                continue;
        }
        else
        {
            LOGGER_ROOT.Write(LOG_FATAL, "AVPath::pathRelativeToAppEnv: "
                              "internal error occured, list entry does not match any "
                              "environment variable.");
        }

        if(path == env)
        {
            relative_path.append("/");
        }
        else
        {
            AVASSERT(path.length() > env.length());
            if(path.at(env.length()) == '/')
                relative_path.append(path.mid(env.length()));
            else
                relative_path.append("/" + path.mid(env.length()));
        }

        if(matched_env) *matched_env = *it;
        return relative_path;
    }

    return m_env_string;
}

///////////////////////////////////////////////////////////////////////////////

QUrl AVPath::pathNameToUrl(const QString &path_name)
{
    //When pathName starts with ":" assume its a correct resouce identifier
    if (path_name.startsWith(":"))
    {
        return QUrl(QStringLiteral("qrc") + path_name);
    }
    else
    {
        return QUrl::fromLocalFile(path_name);
    }
}

///////////////////////////////////////////////////////////////////////////////

QUrl AVPath::expandUrl() const
{
    QString expanded_path = QDir::fromNativeSeparators(expandPath());
    // It is important to keep a trailing '/' (which apparently can be removed by expandPath) to
    // make QUrl::resolved() work correctly.
    if (QDir::fromNativeSeparators(m_env_string).endsWith("/") && !expanded_path.endsWith("/"))
    {
        expanded_path += "/";
    }
    return pathNameToUrl(expanded_path);
}

///////////////////////////////////////////////////////////////////////////////

QDebug operator<<(QDebug dbg, const AVPath& path)
{
    dbg.nospace() << "AVPath(" << path.path() << ')';
    return dbg.maybeSpace();
}

///////////////////////////////////////////////////////////////////////////////

template<> QString AVToString(const AVPath& arg, bool enable_escape)
{
    return AVToString(arg.path(), enable_escape);
}

///////////////////////////////////////////////////////////////////////////////

template<> bool AVFromString(const QString& str, AVPath& arg)
{
    QString path;
    if (!AVFromString(str, path)) return false;
    arg.setPath(path);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

AV_REGISTER_METATYPE(AVPath)

///////////////////////////////////////////////////////////////////////////////

// End of file
