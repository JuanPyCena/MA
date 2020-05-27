///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author    Dr. Thomas Leitner, t.leitner@avibit.com
    \author    QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief     Functionality to query application specific environment variables
*/

#if !defined(AVENVIRONMENT_H_INCLUDED)
#define AVENVIRONMENT_H_INCLUDED

#include "avlib_export.h"
#include "avlog.h"

// forward declarations
class AVPath;

///////////////////////////////////////////////////////////////////////////////

/**
 * @brief The AVEnvironment class provides environment variable utility functions and
 *        functionality to query application specific environment variables.
 */
class AVLIB_EXPORT AVEnvironment
{
public:
    enum EnvVarPrefix
    {
        NoPrefix,
        AppNamePrefix,
        ProcessNamePrefix
    };

    static void setProcessName(const QString& pn)
    {
        process_name = pn;
    }

    /*! sets the global application name and config dir by parsing the argc, argv and
        checking if the option "-an appname" and/or "-cfgdir configdir" is specified.
        \param dan  default application name to be used when an is not specified
        \param argc the number of command line parameters in argv. This should
                    only be used when qApp is not available (e.g. in unit tests)
        \param argv the command line parameters. This should only be used when
                    qApp is not available (e.g. in unit tests)
        \note if the option "-cfgdir configdir" is specified the global configuration
              directory is set
     */
    static void setApplicationName(const QString &dan, int argc=0,
                                   char** argv=0);

    //! Directly set the directory where configs are searched. Used for AVEnvironment2 transition only.
    static void setApplicationConfig(const QString& cfgdir);

    /*! sets the application title.
        \param applicationTitle the app title or visible name to be set
     */
    static void setApplicationTitle(const QString &applicationTitle);

    //! Sets the application description
    /*! The text serving as the application info can be any multi line text,
        lines separated by \n.
        The application name is usually something generic, like ASTOS,
        while the application info is something application specific,
        like "ASTOS Display".

        \param applicationInfo the application information string
    */
    static void setApplicationInfo(const QString &applicationInfo);

    static QString getProcessName();

    /*! gets the application name.
        \return the application name
     */
    static QString getApplicationName();

    //! Returns the application title
    /*! \return the title or visible name of the application
    */
    static QString getApplicationTitle();

    //! Returns the application description
    /*! \return the info string set with setApplicationInfo
    */
    static QString getApplicationInfo();

    //! Returns the standard caption for this application
    /*! The standard caption is a string created from application name,
        verison and build number.
        If appname is empty, the string returned by getApplicationName() is
        used instead.

        \param appname overrides set application name if not empty

        \return the standard caption of the application
    */
    static QString getApplicationCaption(
        const QString& appname = QString::null);

    /*! gets the home path of the current application.
        \return the path to the application home directory
     */
    static QString getApplicationHome();

    //! TODO discuss with CM as soon as he is available again!!!
    //! Override the default application home directory.
    /*! IMHO this should only be used for testing!
     WARNING: this call only makes sense *before* any other call to any
     getApplicationXXX method and will clear all of the other application_XXX
     variables!
     */
    static void setApplicationHome(const QString& directory);

    /*! gets the bin path of the current application.
        \return the path to the application bin directory
     */
    static QString getApplicationBin();

    /*! gets the config path of the current application.
        \return the path to the application config directory
     */
    static QString getApplicationConfig();

    /*! gets the log file path of the current application.
        \return the path to the application log directory
     */
    static QString getApplicationLog();

    /*! gets the "run file"  path of the current application.
        \return the path to the application "run" directory
     */
    static QString getApplicationRun();

    /*! gets the data file path of the current application.
        \return the path to the application data directory
     */
    static QString getApplicationData();

    /*! gets the cache file path of the current application.
        \return the path to the application cache directory
    */
    static QString getApplicationCache();

    /*! gets the maps file path of the current application.
        \return the path to the application maps directory
     */
    static QString getApplicationMaps();

    /*!
     *  \brief Searches for map using the provided path.
     *  \details The following search order is used:
     *   # If path is an aboslute path return expanded path.
     *   # If path is a relative path to an existing file return an aboslute
     *     path pointing to that file.
     *   # If getApplicationMaps()/path exists return an aboslute path to that file.
     *   # If AVConfig2 is used search for path in the configuration directories
     *     and return an absolute path on success.
     *   # Otherwise the given path expanded is returned.
     *  \return Absolute path to the map.
     *          If no existing file was found then the given path will be returned.
     *  \note If the same file is found multiple times an error will be logged.
     *        The first found path to that file is returned.
     *        For example when "loww.xml" is found both in the map dir and in
     *        the config dir which have different paths.
     */
    static QString findMap(const AVPath &path);

    /*!
     *  \brief same as findMap(const AVPath &path) taken a QString
     *  \note Also supports environment variables with the same rules as for AVPath.
     *  \see findMap(const AVPath &path)
     */
    static QString findMap(const QString &path);

    /*!
     *  Returns the path of the project home directory. This will be valid only for a development
     *  setup. This method should therefore only be called driven by configuration
     *  data. Check for QString::null as return value if you need to, no error will be generated if
     *  PROJECTS_HOME is not set.
     *
     *  \return The absolute PROJECTS_HOME, or QString::null if the environment variable is not set.
     */
    static QString getProjectsHome();

    /*! Gets the specified environment variable.
     *  Per default, prepends the application name (e.g. "AVCOMMON_") to the given variable name.
     *  Other options are to prepend the process name (always uppercase, e.g. "FDP2_") or
     *  to prepend nothing and use the name as supplied.
     *
     *  \param var     The environment variable excluding "APPNAME_" e.g. getEnv("HOME").
     *  \param verbose If true a warning is generated if the variable is not found.
     *  \param prefix  Which prefix (if any) to prepend to var.
     *
     *  \return QString::null if the environment variable was not found, the value of the
     *          specified environment variable otherwise.
     */
    static QString getEnv(const QString &var, bool verbose = true,
                          EnvVarPrefix prefix = AppNamePrefix);

    //! Sets the environment variable with the given name to the given value,
    //! optionally overwriting any existing value. This method also works on WIN32.
    //! TODO: Maybe add prefix parameter like in getEnv() ??
    /*!
     * @param var_name name of the environment variable to set.
     * @param var_value value to set to the env. variable.
     * @param overwrite whether to overwrite the value if the env. variable already exists.
     * @return returns true on success, false otherwise. If the variable already exists and
     *         overwrite ist false, true is returned as well.
     */
    static bool setEnv(const QString& var_name,
                       const QString& var_value,
                       bool overwrite = true);

    /**
     * @brief extendPath adds a new path to an existing environment variable, or sets it if this is the first path.
     *
     * The path separator is determined based on the underlying OS.
     *
     * @param new_path      The path to add or set.
     * @param variable_name The environment variable to modify (e.g. PATH or PYTHONPATH)
     * @return              True if the path was added, false if it already was there.
     */
    static bool extendPath(const QString& new_path, const QString& variable_name="PATH");
    /**
     * @brief getPath returns an environment variable which is split based on the OS-specific separator (":" vs ";").
     *
     * @param variable_name The name of the environment variable (e.g. PYTHONPATH, AV_PLUGIN_PATH etc)
     * @return              The list of paths.
     */
    static QStringList getPath(const QString& variable_name);

    //! retrieves list of all application names
    //! \returns QStringList including all application names
    static QStringList getAllApplicationNames();

    //! application name for app ASTOS
    static const QString APP_ASTOS;

    //! application name for app AVCOMMON (for test programs)
    static const QString APP_AVCOMMON;

    //! application name for app AVIMET
    static const QString APP_AVIMET;

    //! application name for app OPTAMOS
    static const QString APP_OPTAMOS;

    //! application name for app AVTBS
    static const QString APP_TBS;

    //! application name for app DIFLIS
    static const QString APP_DIFLIS;

    //! application name for app MUSTRAC
    static const QString APP_MUSTRAC;

    //! application name for app DECLOS
    static const QString APP_DECLOS;

    //! application name for app DMAN
    static const QString APP_DMAN;

    //! application name for app MLAT
    static const QString APP_MLAT;

    //! application name for app INFOMAX
    static const QString APP_INFOMAX;

    //! application name for app INFOMAX2
    static const QString APP_IM2;

    //! application name for app HDPS
    static const QString APP_HDPS;

    //! application name for app VVRMAX
    static const QString APP_VVRMAX;

    //! application name for app SAFECONTROL (old)
    static const QString APP_SAFECONTROL;

    //! application name for app SAM
    static const QString APP_SAM;

    //! application name for app ADMAX
    static const QString APP_ADMAX;

    //! OS-specific env variable path separator
    static const QChar PATH_SEPARATOR;
private:
    //! disable the default constructor
    AVEnvironment();

    //! create a directory if it does not exist yet
    static void createDirectoryIfNonExistent(const QString &dname);

    //! Provided via AVApplicationInitParams; potentially overridden by the <an>_PROCESS_NAME environment variable.
    static QString process_name;
    static QString application_name;
    static QString application_home;
    static QString application_bin;
    static QString application_config;
    static QString application_log;
    static QString application_run;
    static QString application_data;
    static QString application_maps;
    static QString application_cache;
    static QString application_title;
    static QString application_info;
    static QString application_version;
    static QString application_build_number;
    static QString application_build_date;
};

#endif

// End of file
