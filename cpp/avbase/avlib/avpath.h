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
    \brief   Declaration of the AVPath class
*/

#if !defined(AVPATH_H_INCLUDED)
#define AVPATH_H_INCLUDED

// Qt includes
#include <qdir.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qregexp.h>
#include <qvector.h>
#include <QUrl>

#include "avenvstring.h"
#include "avlib_export.h"
#include "avfromtostring.h"

///////////////////////////////////////////////////////////////////////////////
//! This class can be used to work with file pathes containing environment variables
/*! The main use cases are:

    <ul>
    <li> Expand an existing file path, so that all environment variables are substituted by their
         values.
    </li>
    <li>  Make an existing file path relative to the current application environment.
          For instance, a file was selected in a gui application and the path looks like this:
          "/home/astos_home/data/file.xml". Then the following code will make the path relative
          to \$(ASTOS_DATA):

          \code
            AVPath obj("/home/astos_home/data/file.xml");
            QString newPath = obj.pathRelativeToAppEnv();
            // value of new path  = "$(ASTOS_DATA)/file.xml"
          \endcode
    </li>
    </ul>
*/
class AVLIB_EXPORT AVPath : public AVEnvString
{
public:

    enum AppEnv
    {
        HOME,          //!< <==> AVEnvironment::getApplicationHome
        LOG,           //!< <==> AVEnvironment::getApplicationLog
        DATA,          //!< <==> AVEnvironment::getApplicationData
        RUN,           //!< <==> AVEnvironment::getApplicationRun
        CONFIG,        //!< <==> AVEnvironment::getApplicationConfig
        MAPS,          //!< <==> AVEnvironment::getApplicationMaps
        CACHE,         //!< <==> AVEnvironment::getApplicationCache
        PROJECTS_HOME, //!< <==> AVEnvironment::getProjectsHome
        USR_HOME       //!< <==> user's home directory
    };

    typedef QVector<AppEnv> AppEnvList; //!< list of AppEnv

    //! Standard constructor
    AVPath()
    {
        initialize();
    }

    //! Constructor
    explicit AVPath(const QString& path);

    //! Destructor
    ~AVPath() override {}

    //! Sets the path to work with
    inline void setPath(const QString& new_path)
    {
        setEnvString(new_path);
    }

    //! Returns the set path
    inline QString path() const { return envString(); }

    //! Specifies a list of environment variables that are evaluated when examining the application environment
    /*! The search order, i.e. which variable is checked first, is determined by the order the variables appear
        in the list.

        The AVPath uses the following order as default:
        <ol>
            <li>LOG</li>
            <li>CONFIG</li>
            <li>MAPS</li>
            <li>DATA</li>
            <li>RUN</li>
            <li>HOME</li>
            <li>USR_HOME</li>
        </ol>

        The list can be built using constants of type AppEnv.
        e.g.:
        \code
            AVPath obj("/home/astos_home/data");
            AVPath::AppEnvList list;
            list << AVPath::HOME << AVPath::DATA << AVPath::USR_HOME;
            obj.setEnvSearchOrder(list);
        \endcode

        The default list can be overridden to control the behavior of pathRelativeToAppEnv(), for example.

        TODO AS: The search order is only relevant for pathRelativeToAppEnv() and has no influence on expandPath().
                 It is probably better to make the list a parameter of pathRelativeToAppEnv().

        \sa pathRelativeToAppEnv()
    */
    inline void setEnvSearchOrder(const AppEnvList& list)
    {
        m_env_search_rule = list;
    }

    //! Returns the list of environment variables that are currently evaluated when examining the application environment
    /*! The search order, i.e. which variable is checked first, is determined by the order the variables appear
        in the list.
        \sa setEnvSearchOrder()
    */
    inline AppEnvList envSearchOrder() const { return m_env_search_rule; }

    //! Expands the path, so that all environment variables are substituted with their values.
    /*! Only environmemt variables conforming to the following naming convention are recognized:
        <ul>
        <li> Only captial letters, digits and underscore are allowed </li>
        <li> every variable name starts with a letter </li>
        <li> The whole variable name is enclosed in \$() e.g. \$(AVCOMMON_HOME) </li>
        </ul>
    */
    QString expandPath() const;

    //! Returns the path relative to a standard directory of the application environment
    /*! This method walks through a list of environment variables, see setEnvSearchOrder(), to find the
        best matching directory. The variables are listed in descending order of priority, i.e. the second entry
        in the list will only be evaluated if the first entry does not match.
        \param matchedEnv if passed this parameter will be set to the matching environment
                          variable.
        \sa setEnvSearchOrder()
    */
    QString pathRelativeToAppEnv(AppEnv* matched_env = 0) const;

    //! Converts a local file name to an file url. If pathName is a resource idenfier (e.g.: :/resource/file.txt) it is
    //! converted to an url having the qrc schema. (qrc:///resource/file.txt)
    //! \param pathName The local file name or a resource identifier.
    //! \return An url which representes the pathName if pathName was an valid pathname or resource identifier
    //! \sa QUrl::fromLocalFile()
    //! \sa expandUrl()
    static QUrl pathNameToUrl(const QString& path_name);

    //! Expands the path using expandPath and then convertes the result to a url using pathNameToUrl.
    //! \sa pathNameToUrl()
    //! \sa expandPath()
    QUrl expandUrl() const;

protected:

    //! Initializes the AVPath
    void initialize();

    //! Gets the value of the given environment variable
    /*! \return true if no errors occurred, which does not mean that \e<value>
                may not be empty
    */
    bool getEnv(const QString& name, QString& value) const override;

protected:

    //! Translates the given constant \c env to an environment variable suffix.
    inline const char* mapEnv(AppEnv env) const
    {
        switch(env)
        {
            case HOME:   return "HOME";
            case DATA:   return "DATA";
            case LOG:    return "LOG";
            case RUN:    return "RUN";
            case CONFIG: return "CONFIG";
            case MAPS:   return "MAPS";
            case CACHE:  return "CACHE";

            // not APPNAME dependent suffixes, explict "case" instead of "default"
            // so compiler can warn about new enum values
            case USR_HOME:     return 0;
            case PROJECTS_HOME: return 0;
        }
        return 0;
    }

    //! Gets the value of environment variable "APP_NAME_ + \c suffix"
    /*! \return true if no errors occured. For instance if the given suffix
                is supported.
    */
    bool getAppEnv(const QString& suffix, QString& value) const;

protected:

    AppEnvList m_env_search_rule; //!< list of env names, with the best matching path first
};

QDebug AVLIB_EXPORT operator<<(QDebug dbg, const AVPath& path);

template<> QString AVLIB_EXPORT AVToString(const AVPath& arg, bool enable_escape);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, AVPath& arg);

Q_DECLARE_METATYPE(AVPath)

///////////////////////////////////////////////////////////////////////////////

#endif

// End of file

