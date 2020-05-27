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
    \brief     Network transparent configuration management classes.

    This file contains the AVConfig class as well as all associated helper
    classes.
*/

#if !defined AVCONFIG_H_INCLUDED
#define AVCONFIG_H_INCLUDED

// System includes
#include <iostream>    // solaris 10 needs this for cxxxx includes to work
#include <cstdlib>

// Qt includes
#include <QColor>
#include <QDataStream>
#include <QFont>
#include <QMap>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QList>
#include <QtDebug>

//#include "avconfiglist.h"

#include "avlib_export.h"
#include "avcvariant.h"
#include "avenvironment.h"
#include "avlog.h"

//TODO class AVClient;


///////////////////////////////////////////////////////////////////////////////
//                               Constants
///////////////////////////////////////////////////////////////////////////////

//! default user name which is used when setAllowDefault is set.

AVLIB_EXPORT extern const char * const AVConfigDefaultUser;

// When using avconfig2, this header will still be included in various places. Avoid inclusion order
// headaches through this #ifndef
#ifndef AVCONFIG2CLASSIC_MACROS_H_INCLUDED

//! short macro to add a new AVConfigEntry in a AVConfigBase derived class.
#define ADDCE(key, var, default_val, desc)       *(var)=(default_val); \
    {AVConfigEntry* cfgEntry = new AVConfigEntry((key), (var), (default_val), (desc)); \
    AVASSERT(cfgEntry != 0); \
    Add(cfgEntry);}

//! short macro to add a new AVConfigEntry in a AVConfigBase derived class
//! and also specifiy a command line parameter.
#define ADDCEP(key, var, default_val, desc, cmdline_param)   *(var)=(default_val); \
    {AVConfigEntry* cfgEntry = \
        new AVConfigEntry((key), (var), (default_val), (desc), (cmdline_param)); \
    AVASSERT(cfgEntry != 0); \
     Add(cfgEntry);}

//! short macro to add a new AVConfigEntry in a AVConfigBase derived class
//! and also specifiy a command line parameter, special parameterrange setting.
//! only maximum value is needed.
#define ADDCEPT(key,var,default_val,desc,cmdline_param,max)    *(var)=(default_val); \
    {AVConfigEntry* cfgEntry = \
        new AVConfigEntry((key),(var),(default_val),(desc),(cmdline_param),(max)); \
    AVASSERT(cfgEntry != 0); \
    Add(cfgEntry);}

//! short macro to add a new AVConfigEntry in a AVConfigBase derived class
//! and also specifiy a command line parameter, special parameterrange setting.
//! maximum and minimum values are needed.
#define ADDCEPTT(key,var,default_val,desc,cmdline_param,min,max) *(var)=(default_val); \
    {AVConfigEntry* cfgEntry = \
        new AVConfigEntry((key),(var),(default_val),(desc),(cmdline_param),(min),(max)); \
    AVASSERT(cfgEntry != 0); \
    Add(cfgEntry);}

//! short macro to add a new AVConfigEntry in a AVConfigBase derived class
//! and also specifiy special parameterrange setting.
//! maximum and minimum values are needed.

#define ADDCETT(key,var,default_val,desc,min,max) *(var)=(default_val); \
    {AVConfigEntry* cfgEntry = \
        new AVConfigEntry((key),(var),(default_val),(desc),0,(min),(max)); \
    AVASSERT(cfgEntry != 0); \
    Add(cfgEntry);}

#endif // #ifndef AVCONFIG2CLASSIC_MACROS_H_INCLUDED

//! short macro to add a helptext to a AVConfigEntry
#define ADDHELP(a,b) AddHelpText(a,b)

///////////////////////////////////////////////////////////////////////////////
//! holds one entry in the AVConfigBase class.
/*! In order to save the configuration data, the AVConfigBase class holds a
    list of configuration variables. Each variable consists of a AVCVariant
    as well as a descriptor which contains more information about this
    variable. This class basically defines this descriptor. It contains
    text strings for the key, the command line option as well as a description
    string for the variable.
*/

class AVLIB_EXPORT AVConfigEntry
{
public:

    static const int    INT_DEFAULT_MAXMIN;
    static const double DOUBLE_DEFAULT_MAXMIN;
    static const int    STRING_DEFAULT_MAX_LENGTH;
    static const int    POINT_DEFAULT_MAXMIN;

    //! construct an empty config entry
    AVConfigEntry();
    //! construct a config entry with an int variable
    AVConfigEntry(const char *key, int *variable, const int default_value,
                  const char *description, const char *option = 0,
                  int vmin = -INT_DEFAULT_MAXMIN, int vmax = INT_DEFAULT_MAXMIN);
    //! construct a config entry with an uint variable
    AVConfigEntry(const char *key, uint *variable, const uint default_value,
                  const char *description, const char *option = 0,
                  uint vmin = 0, uint vmax = INT_DEFAULT_MAXMIN);
    //! construct a config entry with an double variable
    AVConfigEntry(const char *key, double *variable, const double default_value,
                  const char *description, const char *option = 0,
                  double vmin = -DOUBLE_DEFAULT_MAXMIN, double vmax = DOUBLE_DEFAULT_MAXMIN);
    //! construct a config entry with an bool variable
    AVConfigEntry(const char *key, bool *variable, const bool default_value,
                  const char *description, const char *option = 0);
    //! construct a config entry with an QString variable
    AVConfigEntry(const char *key, QString *variable, const QString &default_value,
                  const char *description, const char *option = 0,
                  AVCVariant::Type t = AVCVariant::String, int vmax = STRING_DEFAULT_MAX_LENGTH);
    //! construct a config entry with an QRect variable
    AVConfigEntry(const char *key, QRect *variable, const QRect &default_value,
                  const char *description, const char *option = 0,
                  const QRect &vmin=QRect(QPoint(-POINT_DEFAULT_MAXMIN,-POINT_DEFAULT_MAXMIN),
                                          QPoint(-POINT_DEFAULT_MAXMIN,-POINT_DEFAULT_MAXMIN)),
                  const QRect &vmax=QRect(QPoint( POINT_DEFAULT_MAXMIN, POINT_DEFAULT_MAXMIN),
                                          QPoint( POINT_DEFAULT_MAXMIN, POINT_DEFAULT_MAXMIN)));
    //! construct a config entry with an QColor variable
    AVConfigEntry(const char *key, QColor *variable, const QColor &default_value,
                  const char *description, const char *option = 0);
    //! construct a config entry with an QSize variable
    AVConfigEntry(const char *key, QSize *variable, const QSize &default_value,
                  const char *description, const char *option = 0,
                  const QSize &vmin=QSize(-POINT_DEFAULT_MAXMIN,-POINT_DEFAULT_MAXMIN),
                  const QSize &vmax=QSize( POINT_DEFAULT_MAXMIN, POINT_DEFAULT_MAXMIN));
    //! construct a config entry with an QPoint variable
    AVConfigEntry(const char *key, QPoint *variable, const QPoint &default_value,
                  const char *description, const char *option = 0,
                  const QPoint &vmin=QPoint(-POINT_DEFAULT_MAXMIN,-POINT_DEFAULT_MAXMIN),
                  const QPoint &vmax=QPoint( POINT_DEFAULT_MAXMIN, POINT_DEFAULT_MAXMIN));
    //! construct a config entry with an QStringList variable
    AVConfigEntry(const char *key, QStringList *variable, const QStringList &default_value,
                  const char *description, const char *option = 0,
                  int vmax = 1024);
    //! construct a config entry with an AVIntList variable
    AVConfigEntry(const char *key, AVIntList *variable, const AVIntList &default_value,
                  const char *description, const char *option = 0,
                  int vmin = -INT_DEFAULT_MAXMIN, int vmax = INT_DEFAULT_MAXMIN);
    //! construct a config entry with an QFont variable
    AVConfigEntry(const char *key, QFont *variable, const QFont &default_value,
                  const char *description, const char *option = 0);

    //! construct a config entry based on another config entry.
    AVConfigEntry(AVConfigEntry &e) : defaultIsValid(false)
    {
        copy(e);
        unused = e.unused;
    }
    //! assign a config entry to this config entry.
    AVConfigEntry &operator=(AVConfigEntry &e);
    //! used by the copy constructor and the assignment operator, it copies
    //! the specified config entry to the local config entry.
    void copy(AVConfigEntry &e);
    //! get the variant associated with this config entry
    AVCVariant &getVar()            { return var;  }
    //! get the variant associated with this config entry
    const AVCVariant &getVar() const { return var; }
    //! get the variant associated with this min config entry
    AVCVariant &getMin()            { return min;  }
    //! get the variant associated with this min config entry
    const AVCVariant &getMin() const { return min;  }
    //! get the variant associated with this max config entry
    AVCVariant &getMax()            { return max;  }
    const AVCVariant &getMax() const { return max;  }
    //! get the variant associated with this default config entry
    AVCVariant &getDefault()        { return def;  }
    //! get the variant associated with this default config entry
    const AVCVariant &getDefault() const  { return def;  }
    //! get the helptext associated with this config entry
    QString &getHelp()              { return help; }
    //! set the variant associated with this config entry
    void setVar(AVCVariant &v)      { var = v;     }
    //! set the variant associated with this min config entry
    void setMin(AVCVariant &v)      { min = v;     }
    //! set the variant associated with this max config entry
    void setMax(AVCVariant &v)      { max = v;     }
    //! set the variant associated with this default config entry
    void setDefault(AVCVariant &v)  { def = v; defaultIsValid=true; }
    //! set the helptext associated with this config entry
    void setHelp(const QString &text)     { help = text; }

    QString      key;               //!< variable key (used in the config file)
    QString      desc;             //!< entry description (for help text)
    QString      option;           //!< command line option (if any)
    //! The name of a different parameter which should be used for this parameter, or QString::null.
    //! Only supported with the new config.
    QString reference;

    //! write the config entry to the specified data stream
    bool write(QDataStream &s);
    //! write the config entry to the specified data stream (reduced output length)
    bool writeShort(QDataStream &s);
    //! read the config entry from the specified data stream
    bool read(QDataStream &s);

    //! write the config entry to the specified text stream
    bool write(QTextStream &s);
    //! write the config entry to the specified text stream (reduced output length)
    bool writeShort(QTextStream &s);
    //! read the config entry from the specified text stream
    bool read(QTextStream &s);
    //! returns the status of the default value (true if a default value exsist)
    bool statusOfDefaultValue() { return defaultIsValid; }
    //! if the default value is valid, var is set to the default value
    void setValueToDefault() { if(defaultIsValid) var=def; }
    //! Used for AVConfig2 only.
    void invalidateDefault() { defaultIsValid = false; }
    //! set the unused flag state
    void markUnused() {
        if (AVLogger == 0) qDebug() << "LOG_INFO: Unused param " << key;
        else               AVLogInfo << "Unused param " << key;
        unused=true;
    }
    //! returns the unused state
    bool isUnused() const { return unused; }

private:
    AVCVariant   var;       //!< variant containing the config variable
    AVCVariant   min;       //!< variant containing the minimum value
    AVCVariant   max;       //!< variant containing the maximum value
    AVCVariant   def;       //!< variant containing the default value
    bool defaultIsValid;    //!< indcates the status of the default value
    bool unused;            //!< indcates if this parameter is unused
    QString help;           //!< helptext associated with this configentry
};
///////////////////////////////////////////////////////////////////////////////
//! class derived from QList to contain the single config entries.
/*! As AVConfigEntry holds only one config variable and we most likely need
    more than one, we need a list of config entries to save all variables.
    This class defines this list and also implements the compareItems
    function of the QList class in order to find entries in the list
    according to their key.
*/

typedef QList<AVConfigEntry*> AVConfigEntryList ;

///////////////////////////////////////////////////////////////////////////////
//! Base class to hold arbitrary configuration data.
/*! Applications create classes which inherit from this class and just add
    their variables with the ADDCE and ADDCEP macros. A typical usage of this
    class is as follows

    \code
    class CFGTestConfig : public AVConfigBase
    {
    public:
        CFGTestConfig(const QString &name = "test") :
            AVConfigBase(name) {

            // create config entries

            ADDCEP("xoffset",    &xoffset, 0, "X drawing offset", "xoff");
            ADDCEP("yoffset",    &yoffset, 0, "Y drawing offset", "yoff");
            ADDCEP("servername", &server_name, "localhost", "servername", "s");
            ADDCEP("serverport", &server_port, 9999, "server port", "p");
            ADDCE ("font",       &font, QFont("courier", 12, 50, false),"font");

            // check for help, read config and parse params

            standardInit();
        }
        int         xoffset, yoffset;
        QString     server_name;
        int         server_port;
        QStringList sl;
        AVIntList   il;
        QFont       font;
    };

    main(int argc, char *argv[]) {
        CFGTextConfig cfg;
    }

    \endcode

    When a main program is constructed like this, the following things
    happen:

    \li If the user has specified -help on the command line, a program usage
        help text is written to stdout and the program is terminated. The
        help usage for this program would contain a general help about the
        built-in options (like -help etc., see below) as well as the program
        specific options like -xoff, -yoff, -s and -p for this case.
    \li The a configuration file is read depending on the APP_CONFIG
        enrivonment variable. This environment variable can either point
        to the config directory which is being used to find the config file,
        or to a tcp/ip configuration server which provides access to the
        configuration. For the latter case the APP_CONFIG environment
        variable is supposed to contain the string #server where server is
        the name of the tcp/ip server running the config server.
    \li After the configuration file is read either from the server or from
        a local file, the command line is parsed. Any variable specified
        on the command line, overwrites the value which is read from the
        config file or server.

    The config file which is being read from the configuration directory
    has the name user_app.cfg where "user" is the local username (or empty
    if no user name at all has been supplied) and "app" is the name of the
    particular application program.

    In each configuration file a number of parameter sets with an identical
    set of parameters but different values are supported.

    The following additional build-in command line switches are supported
    by the class:

    \li -cfgdir d .. set configuration directory d
    \li -cfg c .. use this absolute config file name or server in the
                  form #server
    \li -defps s .. set the default parameter set
    \li -save .. save the used configuration
    \li -debug n .. set the debug level to n (default = 0)
*/

class AVLIB_EXPORT AVConfigBase
{
public:
    //! create a configuration class
    /*! \param an             the name of the application
        \param cfgfile        the name of the cfg file to be read.
        \param allowOptionCfg indicates whether the "-cfg" command line option
                              should be observed
        \param argc           the number of command line parameters in argv.
                              this should only be used when qApp is not
                              available (e.g. in unit tests)
        \param argv           the command line parameters. this should only be
                              used when qApp is not available (e.g. in unit
                              tests)
    */
    explicit AVConfigBase(const QString &an = "", const QString &cfgfile = "",
                 bool allowOptionCfg = true, int argc = 0, char **argv = 0);

    //! destroy the configuration class.
    virtual ~AVConfigBase();

    //! create a configuration class which is a copy of another config class.
    AVConfigBase(const AVConfigBase &b);

    //! assign a configuration to the local config class.
    AVConfigBase &operator=(const AVConfigBase &b);

    //! used by the copy constructor and the assignment operator, this method
    //! copies the specified configuration to the local configuration.
    /*! \param b the configuration to be copied
        \param onlyData if true only the configuration entries are copied
                        and the application name, user name, parameter set
                        etc. are left untouched. if false everything is copied
    */
    void copy(const AVConfigBase &b, bool onlyData = false);

    enum UnusedParamsPolicy
    {
        DiscardUnusedParams = 1,
        AddUnusedParams,
        LogAndDiscardUnusedParams,
        LogAndAddUnusedParams
    };

    //! Defines the handling of unused parameters
    //! The policy is set to LogAndAddUnusedParams
    //! as default.
    //! \sa AVConfigBase::UnusedParamsPolicy
    void setUnusedParameterPolicy(UnusedParamsPolicy policy)
    {
        m_unusedParamsPolicy = policy;
    }

    void disableUnusedParameterWarning()
    {
        switch(m_unusedParamsPolicy)
        {
            case LogAndDiscardUnusedParams:
                m_unusedParamsPolicy = DiscardUnusedParams;
            return;
            case LogAndAddUnusedParams:
               m_unusedParamsPolicy = AddUnusedParams;
            return;
            case DiscardUnusedParams:
            case AddUnusedParams: return;
        }
    }

    void enableUnusedParameterWarning()
    {
        switch(m_unusedParamsPolicy)
        {
            case DiscardUnusedParams:
                m_unusedParamsPolicy = LogAndDiscardUnusedParams;
            return;
            case AddUnusedParams:
               m_unusedParamsPolicy = LogAndAddUnusedParams;
            return;
            case LogAndDiscardUnusedParams:
            case LogAndAddUnusedParams: return;
        }
    }

    //! defines different modes of writing the configuration data
    enum writeConfigMode { Normal,            //!< normal config write mode
                           Overwrite,         //!< overwrite config data
                           Append };          //!< append config data.

    //! add a config entry to the class.
    void Add(AVConfigEntry *e);

    //! add a helptext to a config entry.
    bool AddHelpText(const QString &key, const QString &text);

    //! standard init: performs readConfig, checkForHelp and parseParams
    /*! Use parseParams() if you do not want to read a config file, but
        want to parse the command line.
        Use readConfig() if you want to read the config file, but
        do not want to parse the command line (except for -help and -version).
        Do not call any of these methods if you intend to use a AVConfigList.
    */
    bool standardInit(bool check_for_invalid_params = true,
                      bool check4help = true, bool dosave = true);

    //! \see standardInit, but reads a particular parameter set
    bool standardInitFromParaset(const QString& ps,
                                 bool check_for_invalid_params = true,
                                 bool check4help = true, bool dosave = true);

    //! check for the "-help" and "-version" command line switch and output
    //! on-line help.
    void checkForHelp(bool do_exit = true);

    //! force the helptext to be written along with an additional message
    void help(const QString& argv0, const QString &msg);

    //! parse command line parameters and replace all variables with any
    //! parameters found on the command line.
    /*! \sa standardInit()
     */
    void parseParams(bool check_for_invalid_params = true,
                     bool check4help = true, bool dosave = true);

    //! read the configuration variables
    /*! \param ps              the parameter set to be read or NULL to read the
                               default
        \param un              the user name to read the parameters for or NULL
                               for no particular user.
        \param allowOptionSave indicates whether option -save should be
                               observed.
        \param resetEntries    whether to reset all config entries to their default values
                               before reading. If \c false entries not found in the file will
                               keep their current value, which might be the default if they
                               haven't been read before. Default is \c true
        \sa standardInit()
    */
    bool readConfig (const QString &ps = QString::null,
                     const QString &un = QString::null,
                     bool allowOptionSave = true,
                     bool resetEntries = true);

    //! write the configuration variables
    /*! \param ps the paramter set to write or NULL to write the default
        \param un the user name to write the parameters for or NULL for
                  no particular user.
        \param mode the write mode (see writeConfigMode)
    */
    bool writeConfig(const QString &ps = QString::null,
                     const QString &un = QString::null,
                     writeConfigMode mode = AVConfigBase::Normal);

    //! write all config entries to the specified text stream.
    /*! Subclasses which reimplement this method should either completely
        implement the functionality or call the base implementation for
        the full IO operation and only do pre- and/or postprocessing of
        the data.
    */
    virtual void writeEntriesToStream (QTextStream &s);

    //! write all config entries to the specified text stream. (reduced output length)
    virtual void writeEntriesToStreamShort (QTextStream &s);

    //! read all config entries from the specified text stream.
    /*! Subclasses which reimplement this method should either completely
        implement the functionality or call the base implementation for
        the full IO operation and only do pre- and/or postprocessing of
        the data.
    */
    virtual void readEntriesFromStream(QTextStream &s);

    //! write all config entries to the specified data stream.
    /*! Subclasses which reimplement this method should either completely
        implement the functionality or call the base implementation for
        the full IO operation and only do pre- and/or postprocessing of
        the data.
    */
    virtual void writeEntriesToStream (QDataStream &s);

    //! read all config entries from the specified data stream.
    /*! Subclasses which reimplement this method should either completely
        implement the functionality or call the base implementation for
        the full IO operation and only do pre- and/or postprocessing of
        the data.
    */
    virtual void readEntriesFromStream(QDataStream &s);

    //! return the application name associated with this config class.
    const QString &getAppName() const     { return app_name;     }
    //! return the current user name of this configuration class.
    const QString &getUserName() const    { return user_name;    }
    //! return the parameter-set name of this configuration class.
    const QString &getParaSetName() const { return paraset_name; }

    //! get the list of parameter sets in the current config file.
    const QStringList &getParaSetList(const QString &un = QString::null);

    //! get the list of configured user names.
    const QStringList &getUserList() const;

    //! get the parameter with the specified parameter name
    AVConfigEntry* getParam(const QString &key) const;

    //! Set the application name for this config class, taking into account any existing
    //! config file mapping.
    void setAppName(const QString &s);

    //! set the user name for this config class.
    void setUserName(const QString &s)    { user_name    = s;
                                            checkUserName(user_name); }

    //! set the parameter-set name for this config class.
    void setParaSetName(const QString &s) { paraset_name = s; }

    //! Returns the command line arguments
    /*! The list is created when the first instance of this class is constructed.
        All parameters that are recognized by parseParams(), checkForHelp() and
        parseSpecialParams() are removed from the list.
        This member was previously an instance variable of type QStrList.
        \param argc           the number of command line parameters in argv.
                              this should only be used when qApp is not
                              available (e.g. in unit tests)
        \param argv           the command line parameters. this should only be
                              used when qApp is not available (e.g. in unit
                              tests)
    */
    static QStringList& params(int argc=0, char **argv=0);

    static bool option_help;        //!< display usage
    static bool option_version;     //!< display version
    static bool option_save;        //!< save the configuration
    static QString option_cfg;      //!< configuration file name
    static QString option_defps;    //!< default parameter set
    static int option_debug;        //!< the specified debug level

    //! We cannot determine this information based on whether the AVConfig2Global singleton is
    //! initialized, because the info is needed during AVConfig2Global initialization
    //! (call to AVEnvironment::setApplicationName).
    //! TODO CM is this still true? Replace by calls to AVConfig2Global::isSingletonInitialized()?
    //! TODO CM deprecate and remove in Qt4/5
    static bool process_uses_avconfig2;

    //! copy one font to another font class.
    void copyFont(QFont &to, const QFont &from) { to = from; }

    //! The user name must not contain a underline _ character.
    /*! This routine avoids that.
    */
    void checkUserName(QString &un);

    //! Sets the allow_default flag
    void setAllowDefault(bool b) { allow_default = b;    }

    //! gets the current allow_default flag
    bool getAllowDefault() const      { return allow_default; }

    //! remove all unused parameters
    void removeUnusedParams();

    //! get the debug level
    int debugLevel() const            { return option_debug;  }

    //! get the filename of the config file
    const QString& getConfigFileName() const { return cfg_fname; }

    //! overwrite the filename of the config file
    /*! \param fname the desired configuration file name
    */
    void setConfigFileName(const QString &fname);

    //! get pointer to elist (this list containing all parameters)
    AVConfigEntryList* geteList() { return elist; }

    //! get index of config entry in elist depending on Key
    int indexOfConfigEntry(const QString & compareKey);

    //! get the name of this avconfigbase
    const QString& getConfigName() const { return cfg_name; }

    //! Read config file mappings from the files specified
    //! with -config parameters. The -config parameters are consumed
    //! by this function. The specified files contain lines
    //! of the form "originalname:mappedfilename", for example
    //! "windows:windows-1024x768". This causes the config file
    //! "windows-1024x768.cfg" to be used instead of "windows.cfg".
    //! No filename suffixes must be specified in the mappings file.
    //! A line of the form "@filename" includes the specified mappings file.
    static void readConfigFileMappings(const QString& suffix=".config");

    //! Answer the mapped config file name with the specified
    //! file suffix. The result always has the specified suffix,
    //! even if readConfigFileMappings() has not been called beforehand.
    static QString configFileMapping(const QString& fileName, const QString& suffix=".cfg");

    //! Option for verbose logging of config file accesses
    static void setVerboseLogging(bool verboseLogging) { m_verboseLogging = verboseLogging; }

    //! Resets all entries to their default values
    void resetEntriesToDefault();

    //! Read the config file mappings from the specified file.
    static void parseConfigFileMappings(const QString& fileName, const QString& suffix);
protected:
    //! parse special parameters like -save, -cfg etc.
    static void parseSpecialParams();
    //! save the configuration and exit the application
    virtual void saveAndExit();
    //! calls exit(). allows subclasses to override
    virtual void exitApp(int rc) { exit(rc); }
    //! checks for invalid command line options
    virtual bool checkForInvalidParameters();
    //! prepare for I/O
    void prepareIO(const QString &ps = QString::null,
                   const QString &un = QString::null,
                   bool readOnly = true);
    //! read configuration from a file
    bool readConfigFile();
    //! write configuration to a file
    bool writeConfigFile(writeConfigMode mode = AVConfigBase::Normal);
    //! find the specified parameter-set in the current configuration file or
    //! use it to find all possible parameter-sets (while (findParaSet(QString::null, textstream,
    //! newParameterset))).
    bool findParaSet(const QString &psname, QTextStream &s,
                     QString &l);
    //! specified string with the parameter-set spec. in the config file.
    bool compareParaSet(const QString &line, const QString &psname);
    //! output program usage information and terminate the program.
    void usage(const QString& prog) const;
    //! output the usage information of all entries
    virtual void printHelpOfEntries() const;
    //! output program version information and terminate the program.
    void version(const QString& prog) const;
    //! find a parameter in the list of config entries.
    int  findParam(const QString& parm, QString &key);
    //! write IO message
    void writeConfigIOMessage(const QString& action);
private:
    static QStringList m_params;
    // config file name mappings
    static bool m_useConfigFileMappings;
    static QMap<QString, QString> m_configFileMappings;
    static bool m_verboseLogging;
protected:
    QString app_name;               //!< application name
    QString user_name;              //!< user name
    QString paraset_name;           //!< parameter set name
    QString def_paraset_name;       //!< default parameter set name
    QString ioname;                 //!< ioname, e.g. network server or file
    QString cfg_fname;              //!< config filename
    QString cfg_fname_override;     //!< override cfg name.
    QStringList *paraset_list;      //!< paraset list
    QStringList *user_list;         //!< user list
    AVConfigEntryList *elist;       //!< parameter entry list
    bool allow_default;             //!< allow to read default config
    QString cfg_name;               //!< default config name, need to find
                                    //!< helptext associated with this cfg
                                    //!< in the global helpfile
    UnusedParamsPolicy m_unusedParamsPolicy;    //!< policy for handling unused params,
                                    //!< known flags are defined in UnusedParamsPolicy
};

///////////////////////////////////////////////////////////////////////////////

//! This is an empty config class which just honours the -save and -help options and exits.
/*! Usage: Override saveAndExit() in all config classes by a no-op function and have
 *  an instance of this class after all configs have been read.
 *  This removes the requirement to keep track of the "last" read config file (which is prone to
 *  error).
 */
class AVLIB_EXPORT LastConfig : public AVConfigBase
{
public:
    //! Constructor
    LastConfig() : AVConfigBase() { parseParams(); }
protected:
    //! Exit the application
    void saveAndExit() override { exitApp(EXIT_SUCCESS); }

private:
    //! Copy-constructor: defined but not implemented
    LastConfig(const LastConfig& rhs);
    //! Assignment operator: defined but not implemented
    LastConfig& operator=(LastConfig& rhs);
};



#endif

// End of file
