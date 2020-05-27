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
    \author  Christian Muschick, c.muschick@avibit.com
    \author  QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief   Classes to handle config parameters which can be freely partitioned among config files.
             See https://confluence.avibit.com:2233/display/AVC/avconfig2 for more extensive documentation.
 */

#if !defined AVCONFIG2_H_INCLUDED
#define AVCONFIG2_H_INCLUDED

#include <typeinfo>
#include <iostream>
#include <memory>
#include <typeindex>

#include <QPointer>
#include <QVector>

#include "avlib_export.h"
#include "avconfig2interfaces.h"
#include "avconfig2loadeddata.h"
#include "avconfig2restrictions.h"
#include "avconfig2types.h"
#include "avexplicitsingleton.h"
#include "avfromtostring.h"


class AVConfig2;
class AVConfig2ProjectFactsConfig;

//! Use this class for your nested config containers.
//! Not a typedef because template typedefs are impossible in C++.
template <typename T>
class AVConfig2Map : public QMap<QString, QSharedPointer<T> >
{
};

///////////////////////////////////////////////////////////////////////////////
/**
 *  This is the central container of config parameters. It holds parameter name-value
 *  pairs, the associated metadata and possibly the parameter registrations from user config
 *  classes. Most applications will want to use the AVConfig2Global singleton instead of this class.
 */
class AVLIB_EXPORT AVConfig2Container : public QObject, public AVConfig2ImporterClient
{
    Q_OBJECT

    // Allow access to registerParameter method
    friend class AVConfig2;
    friend class AVConfig2Saver;

    friend class AVConfig2Test;
    friend class AVConfig2SubconfigTemplateTest;

    // use assignLoadedDataTo() to copy the content of an AVConfig2Container.
    AVDISABLECOPY(AVConfig2Container);

public:

    // Don't use static QStrings below so AVConfig2Container instances can be created without
    // undefined initialization order.

    //! Environment variable to disable strict checking in CDash
    static const char* AVCONFIG2_NO_STRICT_CHECKING;
    static const char* AVCONFIG2_NO_STRICT_CHECKING_AUTOSAVE;

    enum class CheckingMode
    {
        CM_STRICT,
        CM_AUTOSAVE,
        CM_LENIENT
    };

    //! See refreshParams()
    struct AVLIB_EXPORT RefreshResult
    {
        RefreshResult();

        //! \return Whether all member error lists are empty.
        bool refreshSuccessful() const;

        //! \return An error message suitable for output via logger. QString::null if no error
        //!         occurred.
        QString getErrorMessage() const;

        //! Contains the names of the missing parameters.
        QStringList m_missing_parameters;
        //! Contains the full names of deprecated parameters.
        //! See AVConfig2Container::m_loaded_deprecated_names for more details.
        QStringList m_loaded_deprecated_names;
        //! Contains the names of parameters for which the metadata from registration differed
        //! from the metadata in the config file.
        QStringList m_metadata_mismatch;
        //! Contains the names of parameters which were present more than once in the loaded
        //! configuration.
        QStringList m_loaded_duplicates;
        //! Contains the names of configs which were missing while loading the configuration.
        QStringList m_missing_configs;
        //! Contains the names of parameters which refer to unknown other parameters.
        //! This is a set for technical reasons (resolveReferences is repeatedly called when
        //! loading subconfigs, we want to remember unresolved references in case the return
        //! value is not always checked, but also want to avoid multiple occurences in the error
        //! message).
        QSet<QString> m_unresolved_references;
        //! Contains a list of error messages relating to command line switches. Errors of this kind are also fatal
        //! e.g. if -save is specified.
        QStringList m_cmdline_errors;
        //! Contains human-readable error messages (parsing failed, restrictions not respected,
        //! post refresh failure etc...)
        QStringList m_errors;

    private:
        template <typename T>
        QString errorHelper(const QString& msg, const T& params) const
        {
            if (params.isEmpty()) return "";
            return "\n" + msg + "\n" + params.join("\n") + "\n";
        }
    };

    //! TODO CM check whether restrictions hold / parameter is missing directly at registration
    //! time?
    //! Better not:
    //! * Would require some magic in Info object destructor
    //! * Cannot detect multiple problems simultaneously
    //! -> do it when refreshCoreParameters is called.
    class AVLIB_EXPORT RegisteredParameterBase
    {
    public:
        explicit RegisteredParameterBase(const AVConfig2Metadata& metadata) :
            m_disable_saving(false),
            m_metadata(metadata),
            m_pure_cmdline_option(false),
            m_settable(false), m_notification_object(0),
            m_was_successfully_loaded(false) {}
        virtual ~RegisteredParameterBase() {}

        //! \return The string representation of the current value of this registered parameter.
        virtual QString toString()                     const = 0;
        //! Converts the given string to the proper datatype and stores it in the registered
        //! parameter.
        //! \return Whether the conversion was successful.
        virtual bool    fromString(const QString& str)       = 0;
        //! Checks whether the data represented by the two strings is the same (so they differ only
        //! in whitespace and formatting).
        //! As we cannot rely on operator== being implemented for all datatypes we are interested
        //! in, do it by converting both into the datatype and back to a string, and comparing the
        //! resulting strings.
        //! False is returned if a fromString() conversion fails for either string.
        virtual bool areParamStringsEqual(const QString& s1, const QString& s2) const = 0;
        //! \return True if this registered variable refers to the given memory location,
        //!         false otherwise.
        virtual bool pointsToVariable(void* var) const = 0;
        //! \return True if the given string can be converted into the registered data type, false otherwise.
        virtual bool isSyntacticallyValid(const QString& value) const = 0;

              AVConfig2Metadata& getMetadata()        { return m_metadata; }
        const AVConfig2Metadata& getMetadata()  const { return m_metadata; }
        bool isPureCmdlineOption()              const { return m_pure_cmdline_option; }
        bool isOptional()                       const { return m_metadata.m_optional; }
        //! For pure command line options, the command line switch is identical to the param name.
        const QString& getCommandlineSwitch() const
        {
            return m_pure_cmdline_option ? m_metadata.m_name : m_metadata.m_command_line_switch;
        }

        //! \return Either the empty list if no deprecated names are configured, or all combinations of proper and
        //! deprecated parameter prefix and name. This list is ensured to never contain the "real" parameter name.
        QStringList getDeprecatedNames() const;

        //! See member doc.
        void disableSaving();
        //! See member doc.
        bool isSavingEnabled() const;

        void setSuccessfullyLoaded();
        bool wasSuccessfullyLoaded() const;

    protected:
        //! True if this parameter was registered more than once. If it was, all but the first registered
        //! parameter are ignored for saving to avoid ambiguities if the values are changed by the program.
        bool m_disable_saving;

        AVConfig2Metadata m_metadata;

        //! If true, the parameter will not be read from or written to a config file, but can be
        //! specified on the command line only. If the parameter is not present on the command line,
        //! the default value will be used for it.
        bool m_pure_cmdline_option;

        //! List of deprecated names without prefixes.
        QStringList m_deprecated_names;
        //! Only the prefix, without name.
        QString m_deprecated_prefix;

        //! Whether the parameter can be changed at runtime, either by reloading a config file or by
        //! AVConsole.
        //! If this is true, the application can specify a slot which is called whenever the
        //! parameter changes.
        //! TODO CM currently unused, implement or remove
        bool m_settable;
        //! The owner of the slot which should be called if the parameter is changed, or 0 if no
        //! notification is required.
        //! TODO CM currently unused, implement or remove
        QObject *m_notification_object;
        //! The slot which should be called the parameter is changed, or an empty string if no
        //! notification is required.
        //! TODO CM currently unused, implement or remove
        QString m_notification_slot;
        //! If the member variable differs from the loaded value, the member var is saved out again (so the program can
        //! change its configuration). This is inhibited if the value never was actually loaded correctly from the file.
        //! Otherwise, params in config files are replaced with their default value when saving an incompletely loaded
        //! configuration (as currently is the case in AVLayerFactory::refreshConfig()).
        bool m_was_successfully_loaded;
    };

    //! This class represents a sub config container registered by the client. There is a common
    //! base class so the different types can be handled uniformly via the following interface.
    class AVLIB_EXPORT RegisteredSubconfigBase
    {
    public:
        RegisteredSubconfigBase(const QString& prefix, AVConfig2Container& config) :
            m_prefix(prefix), m_config(config) {}
        virtual ~RegisteredSubconfigBase() {}

        //! Method which creates a new config container object and adds it to the client's
        //! subconfig map or returns the existing one with the same name.
        //!
        //! \param name The config container will be added to the subconfig map under this name if it odes not exist.
        virtual AVConfig2 *getOrCreateSubconfig(const QString& name) const = 0;
        //! \return The number of subconfigs in the map.
        virtual uint getSubconfigCount()                        const = 0;

        const QString& getPrefix() const { return m_prefix; }

    protected:

        //! This prefix is passed to all newly created subconfig objects (without the trailing ".")
        QString m_prefix;

        //! The config object this subconfig operates on
        AVConfig2Container& m_config;
    };

    AVConfig2Container();
    ~AVConfig2Container() override;

    //! This must be called first, before loading any files and before registering any params.
    //!
    //! Adds the given command line params to m_loaded_cmdline_param and m_loaded_cmdline_overrides.
    //! Note that there is a special handling for the Qt qmljsdebugger parameter.
    //!
    //! \param argc Standard unix command line arg count.
    //! \param argv Standard unix command line arg vector.
    //! \param allow_extra_cmdline If this is true, allow trailing command line parameters which
    //!                            do not conform to avconfig2 syntax (-X or --A.B.C). Optionally,
    //!                            extra command line parameters can be introduced via "--", if this
    //!                            is not done, the first parameter which doesn't conform to
    //!                            avconfig2 syntax is considered the start of the extra command
    //!                            line.
    //!                            Use getExtraCmdline() to retrieve it.
    //!                            Using this parameter makes the application responsible for
    //!                            checking the extra command line for valid syntax!
    void parseCmdlineParams(int argc, const char* const* argv, bool allow_extra_cmdline);
    //! Same as above, for unit tests etc.
    //! *ATTENTION* In contrast to argc/argv, the program name is omitted and *not* the first element of args.
    void parseCmdlineParams(const QStringList& args, bool allow_extra_cmdline);

    //! Returns the extra cmd line parameters. Will always be QString::null if the
    //! allow_extra_cmdline was not given when calling parseCmdlineParams.
    QStringList getExtraCmdline() const;

    //! Also see AVConfig2Global::loadMainConfig(), which should be used in most cases instead of
    //! this method.
    //!
    //! Creates an importer in the correct format for the given config and
    //! uses it to load parameters into this container. Takes into account any loaded config file
    //! mappings and config search paths.
    //! Also see loadConfigAbsolute().
    //!
    //! See locateConfig() for details about how the config is found. It is fatal if the config
    //! is ambigous (same file in different formats, same file in different config paths).
    //!
    //! \param  config   The name of the config (without extension, without exact location).
    //! \param  optional If true, a missing config file will only be logged as INFO, but will cause no further trouble.
    //! \return          Whether the config was successfully loaded.
    bool loadConfig(const QString& config, bool optional = false);

    //! Loads a config file for which the full path is given.
    //!
    //! \param  filename The full absolute path to the config file, including extension.
    //! \param  optional If true, a missing config file will only be logged as INFO, but will cause no further trouble.
    //! \return          Whether the file was successfully loaded.
    bool loadConfigAbsolute(const QString& filename, bool optional = false);

    //! TODO CM this does not really belong here...
    //!
    //! Loads all supported config files of the given directory.
    //! \param directory The path where to look for config files.
    //! \return Whether the files were successfully loaded.
    bool loadConfigTree(const QString& directory);

    //! This method refreshes all yet unrefreshed registered parameters from the set of loaded
    //! params.
    //! Note that it must refrain from logging anything, because it is called right at program
    //! startup, when there is no logger initialized.
    //!
    //! Note that only registered parameters are handled here (e.g. there is no restriction checking
    //! done for loaded but not registered parameters because of performance and isolation
    //! considerations).
    //!
    //! This method can be called at any time; consider calling checkCmdlineParams to detect
    //! multiple parameters with conflicting command line switches, as well as unknown command line
    //! params (as is done by AVConfig2Global::refreshCoreParameters())
    //!
    //! This method triggers postRefresh() calls for the AVConfig2 implementations.
    //!
    //! *Attention* Calling this method again after an unsuccessful call results in undefined behaviour.
    //!
    //! TODO CM design & implement notification mechanism
    //!
    //! \return A report object for errors which occured. If any list is not empty, the state of the
    //!         registered config parameters is undefined and they should not be used.
    //!         If this happens for the global parameters, this should be fatal for the program.
    //!         TODO CM this is not exactly true, document exact post-condition in RefreshResult
    RefreshResult refreshAllParams();

    /**
     * @brief findUserConfig searches all config paths for the given user (=non-avconfig2) config file, taking into
     *                       account any config file mappings.
     *
     * This method is non-const because it creates an identity mapping if no mapping was loaded for
     * the given config (\see getAndFlagMappedConfigFile).
     *
     * @param config    The name of the config file, either relative to the
     *                  config search path or an absolute path (discouraged, probably useful in ad-hoc test scenarios
     *                  only).
     *                  The file extension can optionally be omitted if it is provided in the separate "extension"
     *                  parameter.
     *
     *                  Examples: "workflow/transferrules", "fdp2_fpl_metadata.xml"
     *
     *                  Environment variables are automatically substituted, it is not necessary to use
     *                  AVPath in the application.
     *
     *                  If this parameter empty, it will be returned unchanged.
     *
     * @param extension The full file extension (with leading '.'), or QString::null.
     *
     *                  If the file extension is explicitly provided here, config file mappings are resolved without
     *                  the extension; user config mappings thus look similiar to avconfig2 mappings which also do not
     *                  have an extension.
     *
     *                  Example: map "transferrules" to "xyz/transferrules" instead of
     *                           "transferrules.xfg" to "xyz/transferrules.xfg"
     *                           by providing the extension ".xfg" here.
     *
     *                  TODO: making this optional is broken API design - configurators need to know whether a extension
     *                        was provided here when configuring mappings.
     *
     * @return          If config is empty, the supplied config.
     *                  Otherwise: config+extension if the config file could not be found; the full path to the config
     *                  otherwise.
     *
     *                  TODO: broken API design:
     *                  - to determine whether this method was successful, users must
     *                    check whether the returned value is a relative path, or try to actually read the file.
     *                  - Why allow empty config parameter?
     */
    QString findUserConfig(const QString& config, const QString& extension = QString::null);

    /**
     * @brief findConfig returns the location of the avconfig2 file which would be loaded via loadConfig().
     *
     * Use this method if you want to find a config using the container's search path and config mappings,
     * but want to load the configuration file into a different container (e.g. in a tool working on
     * configuration files).
     *
     * Call getFullFilePath() on the result if you are only interested in the path.
     *
     * @param config see loadConfig().
     *
     * @return The location of the found config. m_name and getFullFilePath() are QString::null if the config could
     *         not be found.
     */
    AVConfig2StorageLocation findConfig(const QString& config);

    //! Adds the facts to the ones already present. It is a fatal error to specify a fact more than
    //! once.
    //!
    //! \return An error string or QString::null.
    QString addFacts(const AVConfig2StorageLocation::FactContainer& facts);
    const AVConfig2StorageLocation::FactContainer& getFacts() const;

    /**
     * @brief setStorageLocation creates and/or updates loaded parameters for all *registered*
     *        parameters matching the given regexp.
     *
     * This method primarily is useful to save an application-generated config (which was not
     * necessarily first loaded from a configuration file).
     * Updates the storage location of parameters, so a subsequent "save" will use the new location.
     * If no loaded parameter exists, it is created. The registered member variable of the
     * config object is used to determine the parameter value in this case.
     *
     * Note: parameters which were registered twice and are exempt from saving are ignored - see
     * RegisteredParameterBase::isSavingEnabled.
     *
     * @param location The storate location to adopt.
     * @param regexp   Restricts the operation to all registered parameters matching this regexp.
     */
    void setStorageLocation(const AVConfig2StorageLocation& location, QRegExp regexp = QRegExp(".*"));

    //! Checks if any loaded parameter or reference begins with prefix (full parameter path)
    /**
     *  *Warning*: Usage of this method is discouraged. Usage of this method in conjunction with optional parameters is
     *  even more discouraged (it makes a difference whether the parameter was specified in a config file or not).
     *
     *  Ignores override parameters (incomplete metadata) which are in the "loaded"
     *  list.
     */
    bool containsLoadedParameterWithPrefix(const QString& prefix) const;

    /**
     * @brief replaceParameterValue changes the value of a loaded parameter.
     *
     * *Attention* usage of this method is discouraged, please consult CM before using it.
     */
    bool replaceParameterValue(const QString& param_name, const QString& new_value);

    //! Returns information about the loaded parameter with the given (fully qualified) name, or 0 if no such
    //! parameter exists.
    //! \see m_loaded_param for types of parameter returned.
    const LoadedParameter *getLoadedParameter(const QString& name) const;

    //! Maps complete parameter name (including section) to value and metadata.
    //! TODO remove this data type when it is no longer used
    typedef QMap<QString, LoadedParameter> LoadedParameterContainer;

    //! *Attention* this method is DEPRECATED.
    //! Use getLoadedData() instead or directly us AVConfig2LoadedData.
    //!
    //! \see m_loaded_param for types of parameter returned.
    const LoadedParameterContainer getLoadedParameters() const;

    //! Returns a map of loaded parameters conforming to the given regular expression
    //! \see m_loaded_param for types of parameter returned.
    LoadedParameterContainer getLoadedParameters(const QRegularExpression& reg_exp) const;

    const AVConfig2LoadedData &getLoadedData() const;

    //! DEPRECATED, do not use in new code. Also see findConfig().
    //! If you feel the desire to use this, please contact CM.
    //! Removal tracked in SWE-5577.
    void assignLoadedMetadataTo(AVConfig2Container& target) const;

    //! DEPRECATED, do not use in new code. Also see findConfig().
    //! If you feel the desire to use this, please contact CM.
    //! Removal tracked in SWE-5577.
    void assignLoadedParamsTo(AVConfig2Container& target) const;

    /**
     * @brief assignDataTo copies "everything except registrations" from this container to
     *                           the given target container.
     *
     * The target container must be empty/unused before this operation, otherwise the call will assert.
     *
     * After this operation, parameter registration on the target container shall yield identical results
     * as registrations on this container (including errors for calls to refreshAllParams()).
     *
     * Use case for introducing this method: dynamically load and discard user configs in AceMax HMI.
     * Discussed design alternative: use the global config container as "parent" container, find missing
     * parameters in the parent. Discarded because of time constraints.
     * Also see SWE-4450.
     *
     * @param target The empty target container.
     */
    void assignDataTo(AVConfig2Container& target) const;

    //! Returns whether the given config was successfully refreshed. It is fatal to pass a config
    //! object which is not associated with this config container.
    bool isConfigRefreshed(const AVConfig2 *config) const;

    //! Convenience method. Load a config from a string in cstyle format.
    void loadFromCstyleString(const QString &config_string);

    void setCheckingMode(CheckingMode mode);
    CheckingMode getCheckingMode() const;

    //! Dumps all loaded parameters and assorted information to the logfile. This method solely
    //! exists to aid in debugging.
    //!
    //! TODO CM make this non-void and don't write to stdout for more general usage...
    void dumpConfig() const;
    void dumpConfigHelper(
        QTextStream& stream, const QString& text1, const QString& text2,
        const AVConfig2StorageLocation& location, QString override_text = QString::null) const;

    //! Print the version string to stdout
    void printBuildVersion() const;

    //! Implements the AVConfig2ImporterClient interface
    void addParameter(const QString& value, const AVConfig2Metadata& metadata,
                      const AVConfig2StorageLocation& location) override;
    //! Implements the AVConfig2ImporterClient interface
    void addReference(const QString& ref, const AVConfig2Metadata& metadata,
                      const AVConfig2StorageLocation& location) override;

    //! Implements the AVConfig2ImporterClient interface
    void addSearchPath(const QString& path, const AVConfig2StorageLocation& location) override;

    //! Implements the AVConfig2ImporterClient interface
    //! First, determine the mapped include file, then loads the included file.
    void addInclude(const QString& include, const AVConfig2StorageLocation& location) override;

    //! Implements the AVConfig2ImporterClient interface
    void addConfigMapping(const QString& map_from, const QString& mapped_dir, const QString& mapped_name,
                          const AVConfig2StorageLocation& location) override;

    //! Implements the AVConfig2ImporterClient interface
    void addOverride(const QString& name, const QString& value, const AVConfig2StorageLocation& location) override;

    //! Implements the AVConfig2ImporterClient interface
    void addDefine(const QString& name, const QString& value, const AVConfig2StorageLocation& location) override;

    /**
     * @brief addInheritedSection implements the AVConfig2ImporterClient interface.
     *
     * Copies all parameters and references from a given parent section into a target section.
     *
     * @param parent   The section name of the parent section.
     * @param section  The target section name
     * @param location The location where the inheritance directive was loaded from.
     */
    void addInheritedSection(const QString& parent, const QString& section,
                             const AVConfig2StorageLocation& location) override;

    //! Implements the AVConfig2ImporterClient interface
    void addSubconfigTemplateParameter(const QString& value, const AVConfig2Metadata& metadata,
                                       const AVConfig2StorageLocation& location) override;

protected:

    template <typename T>
    class RegisteredParameterManipulator;

    template <typename T>
    class RegisteredParameter : public RegisteredParameterBase
    {
        friend class RegisteredParameterManipulator<T>;
    public:
        RegisteredParameter(T& ref, AVConfig2Metadata& metadata) :
            RegisteredParameterBase(metadata), m_reference(ref) {}

        ~RegisteredParameter() override {}

        QString toString() const override;
        bool    fromString(const QString& str) override;
        bool    areParamStringsEqual(const QString& s1, const QString& s2) const override;
        bool    pointsToVariable(void* var) const override;
        bool    isSyntacticallyValid(const QString& value) const override;
        void setValue(const T& value);

    private:
        //! The reference to the actual config parameter in the user's class.
        T& m_reference;
    };

    //! This is the class returned by a registerParameter call. It can be used to specify additional properties of the
    //! registered parameter; its destructor triggers additional processing:
    //! - registration plausi checks
    //! - filling in metadata on yet unspecified entries in m_loaded_param (originating from config file and cmdline
    //!   overrides).
    template <typename T>
    class RegisteredParameterManipulator
    {
    public:
        explicit RegisteredParameterManipulator(RegisteredParameter<T>& param, AVConfig2Container& container);
        RegisteredParameterManipulator(const RegisteredParameterManipulator<T>& other);
        virtual ~RegisteredParameterManipulator();

        //! Use this method to load parameters which do not conform to the naming convention.
        //! Startup of the program will still fail, but "-save" is possible (and uses the proper name).
        //! This method can be called more than once to add several deprecated names for a parameter.
        //! ATTENTION: Adding the same name twice will lead to an assert!
        //!
        //! \param name the parameter name without prefix.
        RegisteredParameterManipulator<T>& setDeprecatedName(const QString& name);
        //! It shouldn't be necessary for client code to use this method. See AVConfig2::m_deprecated_prefix instead.
        RegisteredParameterManipulator<T>& setDeprecatedPrefix(const QString& prefix);

        //! Makes the parameter settable during runtime, either by reloading a config file or via
        //! AVConsole. Optionally registers a slot which will be called whenever the parameter
        //! is changed via one of those mechanisms.
        //! TODO CM currently unused, implement or remove
        //!
        //! \param obj  The object to which the slot belongs. Can be 0 if no notification is
        //!             required.
        //! \param slot This slot will be called whenever the parameter changes. Can be the empty
        //!             string if no notification is required.
        RegisteredParameterManipulator<T>& setSettable(QObject *obj = 0, const char *slot = "");
        //! Sets the suggested value for the parameter.
        /**
         *  Note that the suggested value will normally not used for missing parameters (the exception being if the
         *  AVCONFIG2_NO_STRICT_CHECKING environment variable is set).
         *  If the program terminates due to a missing parameter, this value is logged as a suggestion, additionally the
         *  value is clearly visible as "suggested" in the config file after saving.
         *
         *  When calling this method, the parameter's value is immediately changed to reflect the suggested value
         *  (useful for unit tests).
         *
         *  Using this method together with setOptional or setPureCmdlineOption is not allowed.
         */
        RegisteredParameterManipulator<T>& setSuggestedValue(const T& value);
        /**
         * Note that parameters with this property currently need to be present in the configuration even if
         * strict checking is disabled (this is subject to discussion however).
         *
         * @param reference The name of the parameter to be referred, e.g. "ports.fdp2.command"
         */
        RegisteredParameterManipulator<T>& setSuggestedReference(const QString& reference);
        //! Make this a pure command line parameter. It will not be loaded from and won't be saved
        //! to a config file.
        /**
         *  Using this method together with setOptional or setSuggestedValue is not allowed.
         *
         *  When calling this method, the parameter's value is immediately changed to reflect the default value
         *  (useful for unit tests).
         *
         *  \param default_value This value is used if this parameter is not specified on the
         *                       command line.
         */
        RegisteredParameterManipulator<T>& setPureCmdlineOption(const T& default_value);

        //! Declares a parameter to be optional.
        /**
         *  If a parameter is optional, it does not have to be specified in the config file. If it
         *  is not present, the default value is used.
         *  Furthermore, it is omitted when saving if it has the default value and the target config
         *  file does not already include the parameter.
         *
         *  When calling this method, the parameter's value is immediately changed to reflect the default value
         *  (useful for unit tests).
         *
         *  Using this method together with setPureCmdlineOption or setSuggestedValue is not allowed.
         */
        RegisteredParameterManipulator<T>& setOptional(const T& default_value);

        //! Makes this parameter changeable via the command line.
        RegisteredParameterManipulator<T>& setCmdlineSwitch(const QString& cmd_line);

        RegisteredParameterManipulator<T>& setRestriction(
                std::unique_ptr<const AVConfig2TypedRestriction<T>> restriction);

        //! Convenience method to create a AVConfig2RangeRestriction on this parameter.
        //! See https://confluence.avibit.com:2233/x/soQiAQ for a list of port ranges. Adapt this method if necessary.
        //! TODO CM implement restriction with several ranges?
        RegisteredParameterManipulator<T>& setAvPortRestriction();
        //! Convenience method to create a AVConfig2RangeRestriction on this parameter.
        RegisteredParameterManipulator<T>& setMin   (const T& min);
        //! Convenience method to create a AVConfig2RangeRestriction on this parameter.
        RegisteredParameterManipulator<T>& setMax   (const T& max);
        //! Convenience method to create a AVConfig2RangeRestriction on this parameter.
        RegisteredParameterManipulator<T>& setMinMax(const T& min, const T& max);
        //! Convenience method to create a AVConfig2ValueListRestriction on this parameter.
        RegisteredParameterManipulator<T>& setValidValues(const QList<T>& values);
        //! Convenience method to create a AVConfig2RegexRestriction on this parameter
        RegisteredParameterManipulator<T>& setRegexPattern(const QString &regex_pattern);

        //! Convenience method to create a AVConfig2ContainerValueRestriction on this parameter.
        template<typename V>
        RegisteredParameterManipulator<T>& setValidContainerValues(const QList<V>& values)
        {
            return setRestriction(std::make_unique<AVConfig2ContainerValueRestriction<T>>(values));
        }
    private:
        //! Defined but not implemented.
        RegisteredParameterManipulator& operator=(const RegisteredParameterManipulator<T>& other);

        template<typename V>
        static typename std::enable_if<!std::is_enum<V>::value, void>::type
        applyDefaultRestriction(RegisteredParameterManipulator<V>*)
        {
        }

        template<typename V>
        static typename std::enable_if<std::is_enum<V>::value, void>::type
        applyDefaultRestriction(RegisteredParameterManipulator<V>* that)
        {
            if (that->m_parameter.m_metadata.m_restriction == nullptr)
            {
                QList<T> default_restriction = AVGetEnumValues(that->m_parameter.m_reference);

                if (!default_restriction.isEmpty())
                    that->setValidValues(default_restriction);
            }
        }

        //! Don't rely on the fact that copy operations are optimized away by the compiler for
        //! inline template code.
        //! TODO CM put assertion into copy constructor and reply on compiler optimzation, this
        //! would eliminate the need for refcounting.
        //!
        //! Note that this is not threadsafe as manipulators are short-lived and not designed to
        //! be shared across threads.
        uint                   *m_refcount;
        RegisteredParameter<T>& m_parameter;

        AVConfig2Container& m_container;
    };

    //! Helper type to avoid code duplication with RegisteredSubconfigDeprecated below
    template<typename T>
    class RegisteredSubconfigTyped : public RegisteredSubconfigBase
    {
    public:
        RegisteredSubconfigTyped(
            const QString& prefix, AVConfig2Map<T>& container, AVConfig2Container& config) :
            RegisteredSubconfigBase(prefix, config),
            m_client_container(container) {}

        AVConfig2* getOrCreateSubconfig(const QString& name) const override
        {
            if (this->m_client_container.contains(name))
                return this->m_client_container[name].data();
            return createSubconfig(name);
        }

        virtual AVConfig2* createSubconfig(const QString& name) const = 0;

        uint getSubconfigCount() const override { return m_client_container.count(); }

    protected:
        //! A reference to the subconfig container in the client config class
        AVConfig2Map<T>& m_client_container;
    };

    template<typename T>
    class RegisteredSubconfig : public RegisteredSubconfigTyped<T>
    {
    public:
        RegisteredSubconfig(
            const QString& prefix, AVConfig2Map<T>& container, AVConfig2Container& config) :
            RegisteredSubconfigTyped<T>(prefix, container, config) {}

        AVConfig2* createSubconfig(const QString& name) const override
        {
            T *ret = new (LOG_HERE) T(this->m_prefix + "." + name, this->m_config);
            ret->setConfigMapKey(name);
            // Add the new config object to the client's container
            QSharedPointer<T> config_pointer(ret);
            this->m_client_container[name] = config_pointer;
            return ret;
        }
    };

    //! Refresh a specific config class.
    //! Note that this call will not trigger a postRefresh() call.
    //!
    //! TODO CM make result pointer mandatory?
    //!
    //! \param config Refresh parameters registered by this config
    //! \param result Pass this along to get detailed error reports. Can be 0.
    //! \return       True if successful, false if any errors occured while refreshing the config (matches result->refreshSuccessful())
    bool refreshParams(AVConfig2 *config, RefreshResult *result = 0);

    //! Returns the mapped file name specified for the given config in a loaded parameter file, or
    //! returns config_name if no such mapping was loaded.
    //! It is an error if a config file mapping is loaded after a mapping for the same file already
    //! was requested by the application (this means that mappings have to be loaded first).
    //!
    //! This is a different thing than including configs, and allows
    //! a different usage pattern (an application is not forced to load the entire project
    //! configuration at startup; backwards compatibility with -config mechanism!).
    //!
    //! Rationale: Many programs use "own" config or data files, which do not fit into the AVConfig2
    //! mechanism of including files and distributing parameters among files. Implementing this
    //! functionality here is justified because it avoids that each program has to make those
    //! filenames configurable manually.
    //!
    //! If there is no mapping for this file, store the identity mapping. This is neccessary
    //! to detect illegal inversion of order (mapping requested first, and loaded afterwards)
    //! in addConfigMapping.
    //!
    //! \param  config_name The name of the config file, including the extension for "non-standard"
    //!                     files, excluding it for avconfig2 files.
    //! \return             The mapped config name.
    QString getAndFlagMappedConfigFile(const QString& config_name);

    //! Same as getAndFlagMappedConfigFile, but does not add the identity mapping if there was no mapping.
    QString getMappedConfigFile(const QString& config_name) const;

    //! See loadConfig(). Additional config search paths can be specified when using this method.
    //!
    //! \param  config   The name of the config (without extension, without exact location).
    //! \param  optional If true, a missing config file will only be logged as INFO, but will cause no further trouble.
    //! \param  extra_search_paths Extra paths to search for the given config. May contain
    //!                            env. variables. Used e.g. to specify extra paths for the initial
    //!                            configuration to allow placing those into their respective
    //!                            project directory. \sa getInitialConfigSearchPaths().
    //! \return          Whether the config was successfully loaded.
    bool loadConfigInternal(
            const QString& config, bool optional, const QStringList& extra_search_paths = EmptyQStringList);

    /**
     * @brief resolveConfig determines the storage location of an avconfig2 file.
     *
     * It resolves any config file mapping first, then delegates the work of searching all search paths
     * to locateConfig.
     * See loadConfigInternal for parameter documentation.
     */
    AVConfig2StorageLocation resolveConfig(const QString& config, const QStringList& extra_search_paths = EmptyQStringList);

    //! Searches for a config with the given name. The config may be in any supported format and
    //! may reside in any of the current config search paths (if a relative config is specified).
    //!
    //! It's fatal if the config is ambiguous, i.e. multiple configs with the same name (and perhaps
    //! different formats) exist.
    //!
    //! This method won't resolve any config file mapping! This functionality is performed by
    //! getAndFlagMappedConfigFile().
    //!
    //! This method resolves environment variables, and handles absolute paths as well.
    //!
    //! \param name               The name of the config. Can be absolute or relative to a config
    //!                           search path. Can have an extension. If avibit_config is false,
    //!                           it must have its extension.
    //! \param avibit_config      If true, assume that this is an AviBit config file, and set the
    //!                           format in the result location accordingly. If false, the format
    //!                           always will be CF_LAST.
    //! \param extra_search_paths See loadConfig() and getInitialConfigSearchPaths().
    //! \return                   The location of the located config. Format will always be CF_LAST
    //!                           for user configs.
    //!                           m_name and m_location will be QString::null if no config could be
    //!                           found.
    AVConfig2StorageLocation locateConfig(
            const QString& name, bool avibit_config,
            const QStringList& extra_search_paths = EmptyQStringList) const;

    //! Attention: if a parameter is registered multiple times for the same class, it is undefined which of the
    //! parameters is returned by this method. This is by design and can be exploited when optimizing this method.
    //!
    //! \return Information about the registered parameter with the given name, in linear time. (0,0)
    //!         if no param with the name can be found.
    QPair<const AVConfig2*, const RegisteredParameterBase*> getRegisteredParameterInfo(const QString& param_name) const;

    //! \return Information about the registered subconfig with the given name, in linear time. (0,0)
    //!         if no subconfig with the name can be found.
    QPair<const AVConfig2*, const RegisteredSubconfigBase*> getRegisteredSubconfigInfo(const QString& subconfig_name) const;

    //! Checks whether any command line switch is used by two different parameters in this program,
    //! and whether there were are any unknown command line switches specified.
    //!
    //! Note that command line params for ordinary parameters are already adopted at parameter
    //! loading time (see AVConfig2Container::addParameter()
    //! and AVConfig2Container::addReference()).
    //!
    //! Command line parameters are the first thing and the last thing handled:
    //! Certain things have to be done right at program startup (set debug level, disable logging
    //! if "-help" is used etc.).
    //! Then, after all configs and subconfigs had their parameters registered, this method is
    //! called to check for command line switch conflicts.
    //!
    //! Command line params are sufficiently different from params loaded from other sources so
    //! no AVConfig2ImporterBase implementation is used:
    //!
    //! - Params can be overwritten
    //! - Pure command line params require special treatment
    //! - Command line params can not replace a missing config param
    //!
    //! \param errors Any occuring errors will be added to this list.
    void checkCmdlineParams(QStringList& errors) const;

    //! Must be called by any AVConfig2 object associated with this container.
    //! Used for postRefresh() calls and to remove any registered parameters and subconfigs if the
    //! config object is deleted.
    void addConfigObject(AVConfig2 *config);

    //! Protected so this can only be used by AVConfig2. Registers a new parameter member
    //! with the config system.
    //!
    //! \param name   The name for this parameter, including all prefixes.
    //! \param ref    A reference to the client member. Will be set to the parameter's value when
    //!               refreshParams() is called.
    //! \param help   The help text displayed if the command line option "-help" is used.
    //! \param config The owner of the reference. If it is destroyed, the registration is
    //!               automatically undone.
    template <typename T>
    RegisteredParameterManipulator<T> registerParameter(
            const QString& name, T& ref, const QString& help, AVConfig2 *config);

    //! Protected so this can only be used by AVConfig2. Registers a new subconfig
    //! with the config system.
    //!
    //! \param prefix Sub config classes will automatically register their variables under this
    //!               prefix plus the subconfig name (e.g. "prefix.first", "prefix.second", ...)
    //! \param config_container This container will be filled with the loaded subconfigs when
    //!                         refreshParams is called. The element in the parameter's name
    //!                         following the prefix will be used as key (e.g. "first", "second").
    //!                         For usage examples, see
    //!                         avconfig2test.cpp, NestedSubConfig.
    //! \param config           The owner of the config container. If it is destroyed, the
    //!                         registration is automatically undone.
    template <typename T>
    void registerSubconfig(
        const QString& prefix, AVConfig2Map<T>& config_map,
        AVConfig2 *config);

    //! See AVConfig2::changeSuggestedValue.
    template <typename T>
    void changeSuggestedValue(T& ref, const QString& suggested, AVConfig2 *config);

    //! After successfully calling this method, m_unresolved_references is empty and the
    //! corresponding entries are present in m_loaded_param with their adopted values.
    //!
    //! TODO CM redesign to avoid this method? Resolve references as parameters get loaded?
    //!
    //! \param result Error messages are accumulated in this object.
    void resolveReferences(RefreshResult& result);

    //! Creates subconfig class T.
    //! There are private classes that have defined friend AVConfig2Container so only AVConfig2Container can
    //! create subconfig classes. Used in subconfig template creation.
    template <typename T>
    T* createSubconfigClass(const QString& prefix);

    typedef QVector<QSharedPointer<RegisteredParameterBase> > RegisteredParameterList;
    typedef QPair<const AVConfig2*, RegisteredParameterList> ConfigWithRegisteredParameters;
    struct AVLIB_EXPORT RegisteredParameterContainer : public QList<ConfigWithRegisteredParameters >
    {
        void addConfig(const AVConfig2* config);
        RegisteredParameterList& getParameterList(const AVConfig2* config);
        bool containsConfig(const AVConfig2* config) const;
        void removeConfig(const AVConfig2* config);
    };
    //! Pairs AVConfig2 object with the list of registered parameters for this object.
    //! Also see m_config_object.
    RegisteredParameterContainer m_registered_param;

    //! List of paramater names that were removed from the config.
    //! It is used to exclude these parameters when a config file is preloaded. That way the removed paramaters won't be
    //! saved back to the file.
    QStringList m_removed_param_names;

    typedef std::multiset<QString> RegisteredParameterByNameContainer;
    //! Redundant to m_registered_param, introduced to avoid O(n^2) when checking for existing parameter registrations
    //! via calls to AVConfig2Container::getRegisteredParameterInfo. See SWE-5716.
    RegisteredParameterByNameContainer m_registered_param_names;

    typedef QVector<QSharedPointer<RegisteredSubconfigBase> > RegisteredSubconfigList;
    typedef QMap<const AVConfig2*, RegisteredSubconfigList> RegisteredSubconfigContainer;
    //! Maps config object to vector of registered subconfigs.
    RegisteredSubconfigContainer m_registered_subconfig;

    typedef std::multiset<QString> RegisteredSubconfigByNameContainer;
    //! Redundant to m_registered_subconfig, introduced to avoid O(n^2) when checking for existing subconfig registrations
    //! via calls to AVConfig2Container::getRegisteredSubconfigInfo.
    RegisteredSubconfigByNameContainer m_registered_subconfig_names;

    typedef QSet<AVConfig2*> ConfigObjectContainer;
    //! All config objects which are associated with this container.
    //! This is used to call their postRefresh() method and to handle config class versioning.
    //! Redundant with m_registered_param.
    ConfigObjectContainer m_config_object;

    //! Contains all loaded avconfig2 entities.
    /**
     *  The loaded parameters in this class contain values obtained via the various override mechanisms in addition to
     *  "ordinary" loaded parameters from files.
     *
     *  Several forces influence the design here:
     *
     *  - There are accessors to loaded config data. This means that lazy evaluation of parameter values based on stored
     *    overrides is not an option.
     *
     *  - The order of availability is not fixed (new registrations can happen when subconfigs are created, config
     *    files can be loaded based on configuration).
     *
     *  - Params can be optional, so loading from file cannot be a valid hook to handle overrides.
     *
     *  The solution is to fill everything known into m_loaded_param as early as possible. Refer to
     *  AVConfig2ImporterClient::LoadedParameter::LoadedParameterSource for the different sources for entries in this
     *  list.
     */
    AVConfig2LoadedData m_loaded_data;

    typedef QMap<QString, LoadedReference> LoadedReferenceContainer;
    //! See resolveReferences().
    LoadedReferenceContainer m_unresolved_references;

    typedef QMap<QString, QString> CmdLineParamContainer;
    //! Command line params live in a different "namespace" than ordinary parameter names, so
    //! store them in a different container.
    //! This is here and not in AVConfig2Global (at least) for easier unit testing.
    //! Kept for dumpConfig usage.
    CmdLineParamContainer m_loaded_cmdline_param;
    //! Kept for dumpConfig usage.
    CmdLineParamContainer m_loaded_cmdline_overrides;

    /**
     * @brief m_save_required indicates that a problem was detected which can should solved by saving the configuration.
     *
     * This can be triggered by a version translation or the AUTOSAVE option for AVCONFIG2_NO_STRICT_CHECKING (SWE-1356).
     *
     * This cannot be stored in refresh result: it is needed in refreshCoreParameters and
     * refreshing the config will only be performed for the first call to refresh (which typically happens
     * in a config's constructor).
     */
    bool m_save_required;

private:
    void configDeleted(void *config);

    //! Helper method for parameter loading. Checks whether the parameter was already loaded and
    //! whether it is a pure command line parameter (which is forbidden)
    bool checkNewLoadedParam(const QString& name, const AVConfig2StorageLocation& location);

    //! For the given config object, check whether it has a different version than the one loaded
    //! from the config file; if so, calls the AVConfig2::translateToNextVersion() method
    //! accordingly.
    //!
    //! \param errors Any errors which occured will be added to this list.
    //!
    //! \return Wheter any translation was done and a "-save" should be performed if
    //!         everything else is OK.
    bool handleVersionTranslation(AVConfig2 *config, QStringList& errors);

    typedef QSet<AVConfig2*> UnrefreshedConfigContainer;
    UnrefreshedConfigContainer m_unrefreshed;

    //! Keeps track of all parameters which were loaded more than once (which is an error). Note
    //! that for now, the config object will be in an unsuable state if this occurs (refreshAllParams
    //! will always fail).
    QStringList m_loaded_duplicates;
    //! Keeps track of all config files which were missing at loading time.
    QStringList m_missing_configs;
    //! Keeps track of parameters which still use their deprecated names.
    /*!
     * This is here and not only in RefreshResult because we don't want parameter loading to fail for individual calls
     * to refreshParams(), but only for refreshAllParams() (and implicitly refreshCoreParameters).
     * Deprecated names shouldn't be reported as errors to client code as this would interrupt startup processing (e.g.
     * in the DAQ framework).
     */
    QStringList m_loaded_deprecated_names;

    //! The current facts which will be used for filtering loaded parameters.
    AVConfig2StorageLocation::FactContainer m_facts;

    /*! if allow_extra_cmdline line is set via AVInitParams this list will save all cmd line
     * arguments that are no cmdline option (has no -<name> <param> or --<name> <param> syntax)
     */
    QStringList m_extra_cmdline;

    CheckingMode m_checking_mode;
};

///////////////////////////////////////////////////////////////////////////////
/**
 *  This is the single parameter container which loads all available parameters at program startup
 *  and provides them to the user-defined config classes for registration.
 */
class AVLIB_EXPORT AVConfig2Global : public AVConfig2Container, public AVExplicitSingleton<AVConfig2Global>
{
    Q_OBJECT
public:

    //! The design goal here is to have the params available first thing in an application,
    //! to be more flexible (e.g. suppress logging if -help is called, set -debug right away
    //! before any other initialization is done, use -add_facts when loading parameters etc.)
    //! For this reason, the constructor already does some serious work:
    //!
    //! - handle command line params
    //! - set the facts depending on command line params
    //! - set application name
    //!
    //! Note that the AVConfig2Global singleton and logger are not yet available. For this reason,
    //! any work not immediately neccessary is performed in the initialize() method.
    AVConfig2Global(
            int argc, char**argv,bool allow_extra_cmdline,
            const QString& default_app_name, const QString& default_process_name);
    ~AVConfig2Global() override;

    //! \param app_name TODO CM
    static AVConfig2Global& initializeSingleton(
            int argc, char**argv, bool allow_extra_cmdline,
            const QString& default_app_name, const QString& default_process_name);

    //! This method is called automatically as part of the default process initialization, don't
    //! call it "manually".
    //!
    //! Performs some initialization tasks:
    //! - evaluates some environment variables (facts)
    //! - add facts specified on the command line
    //! - load initial config specified via "-config" command line param or env variable
    //! - load the project-specific fact configuration
    //!
    //! This is a separate initialization method because we want the logger to be already
    //! instantiated, and instantiating the logger has to happen after this object was constructed.
    void initialize();

    //! The help text set here will be displayed at the beginning of "-help" output.
    void setApplicationHelpText(const QString& text);

    //! Same as AVConfig2Container::loadConfig(), except that the "-cfg" command line parameter can
    //! be used to override the config name which is used by this method.
    //! Must be called at most once in a program.
    //!
    //! For command line utilities, the main config can also be specified as "optional". This only is useful if the
    //! corresponding config class has only optional parameters.
    bool loadMainConfig(const QString& config, bool optional = false);

    //! This method must be called after all user-defined config classes which need access to the
    //! global configuration have been instantiated, and after all configs have been loaded. It may
    //! be called only once.
    //!
    //! Refreshing params is done separately from actually loading the config to allow for
    //! application-custom loading using a AVConfig2ImporterBase implementation.
    //!
    //! TODO CM describe default way of loading params
    //!
    //! Unused parameters will be cleared from the parameter caches, and merged with the list
    //! of unused parameters given with the "-unused_params" command line option. TODO CM implement
    //!
    //! If "-help" was specified on the command line, prints a list of the help text of all used
    //! parameters. Additionally, it will schedule the program termination (TODO CM how exactly?).
    //!
    //! TODO CM completely clear parameter cache here?
    //! TODO CM warn for unused command-line options here, store them somewhere intermediately.
    //!
    //! \return Whether the refresh was successful and the application may continue. Returns
    //!         false if the user specified -save, -help and the like.
    //!         Don't attempt to start up the application if this returns false!
    bool refreshCoreParameters();
    bool getRefreshCoreParametersCalled() const;

    //! Call this if a configuration error should inhibit application startup.
    //!
    //! This method is immediately fatal if refreshCoreParameters already was called; otherwise, the error
    //! is printed in the error summary.
    void registerConfigurationError(const QString& error);

    //! If the program is started with -help, -save etc, it shuts down immediately after performing
    //! the requested action. This method allows the program to react accordingly (e.g. don't do
    //! some initialization).
    bool isApplicationStartupAllowed() const;

    //! \return The configuration given with "-config" on the command line, or QString::null if
    //!         there is no such config.
    QString getInitialConfig() const;
    bool isSaveRequested() const;
    bool isHelpRequested() const;
    bool isDumpConfigRequested() const;
    bool isVersionRequested() const;
    /**
     * @brief getStartupDebugLevel returns the debug level for the default logger at startup time. Use
     *        AVLog::isLevelSuppressed(), or preferably the stream-based logging macros (AVLogInfo etc)
     *        to decide whether to log anything.
     */
    uint getStartupDebugLevel() const;
    bool isColdstartRequested() const;
    AVConfig2Format getSaveFormat() const;

    /**
     * @brief internalSave is used if a process is started with "-save". It uses the builtin
     *                     parameters to dermine save order, format and directory.
     *
     * Usually you don't want to call this directly in application code. Instead, instantiate the AVConfig2Saver or
     * AVConfig2SimpleSaver yourself and set their behaviour as required.
     */
    void internalSave();

    //! ** Don't use this method unless you know what you are doing **
    //! Attention, passed configs will be deleted when refreshCoreParameters is called, or immediately if
    //! refreshCoreParameters already has been called.
    //! Passes ownership of the new'ed config to the AVConfig2Global singleton.
    //!
    //! This mechanism exists to extend the lifetime of "throw-away"-configs until refreshCoreParameters is called,
    //! which is neccessary to make "-save" etc work.
    //!
    //! The mechanism is used e.g. for the connection layer configuration.
    void keepOrDeleteConfigInstance(std::unique_ptr<AVConfig2> config);
    //! See keepConfigInstance();
    void clearKeptConfigInstances();

    //! TODO CM when registering params in console, use help text group for grouping as well?
    //! TODO CM fix help so it also can be shown in AVConsole.
    //!
    //! \param group "all" or empty. TODO CM fix this up...
    //! \return A help text for all registered parameters.
    QString getHelp(const QString& group = "") const;

    /**
     * \internal
     * \addtogroup Internal helper methods for REGISTER_CONFIG_SINGLETON
     * @{
     */
    using ConfigInitializerFunc = void(*)();
    static void registerConfigInitializer(ConfigInitializerFunc func);
    static void callConfigSingletonInitializers();
    /** @}*/

    //! legacy method to allow using this class as AVExplicitSingleton
    const char *className() const { return metaObject()->className(); }

    //! This is the directory searched for initial configs in a development setup (must reside in
    //! $PROJECTS_HOME). \see getInitialConfigSearchPaths().
    static const QString DEV_CONFIG_SUBDIR;
    //! Environment variable name to add additional facts
    /**
     *  This mechanism was introduced to control the behaviour of a given software package depending on the context it
     *  is running in. For instance, the GACA software can be run in any of the following contexts:
     *
     *  - developer PC
     *  - FAT setup
     *  - Madrid Demo setup
     *  - Jeddah Test System (in the modes replay, live, sim)
     *  - operationally
     */
    static const QString AVCONFIG2_ADD_FACTS;
    //! Environment variable to extend the avconfig2 search path.
    /**
     *  This variable contains a ":" or ";" separated list (depending on OS)
     *  of absolute paths which are searched for configuration files.
     */
    static const QString AVCONFIG2_INITIAL_CONFIG_PATH;
    //! Environment variable to specify an initial config (as with config switch "-config")
    static const QString AVCONFIG2_INITIAL_CONFIG;

private:

    enum
    {
        //! See getInitialConfigSearchPaths
        INITIAL_CONFIG_SEARCH_DEPTH = 5
    };

    //! Maps type and source to a list of unused parameters. Used internally by updateUnusedParams
    //! and purgeUnusedParams. This representation was chosen to make it easy to find all unused
    //! params for a certain parameter file.
    typedef QMap<QPair<QString, QString>, QSet<QString> > UnusedParameterContainer;

    //! \return Whether application startup is allowed.
    bool handleBuiltinParams();

    //! TODO CM additional functionality:
    //!         - Save the master config which was used
    //!         - Keep track of used params too? Rename to updateParameterUsage?
    void updateUnusedParams(const QString& file_name);
    //! TODO CM implement
    void purgeUnusedParams(const QString& file_name);

    void loadProjectFactsConfig();
    //! Postpone this check until refreshCoreParameters, so parameters in the initial config are also
    //! checked (which are added before the project config is loaded).
    //!
    //! Config is updated only if "-save" was used.
    bool checkAndUpdateProjectFactsConfig();

    /**
     * @brief getInitialConfigSearchPaths determines the locations which are searched for files specified via "-config".
     *
     * This method has two ways to fill the list (results are merged):
     * 1. Recursively searches $PROJECTS_HOME for directories with the name DEV_CONFIG_SUBDIR.
     *    The recursive search depth is limited by INITIAL_CONFIG_SEARCH_DEPTH.
     * 2. Evaluates AVCONFIG2_INITIAL_CONFIG_PATH.
     *
     * This allows starting a program with e.g. "-config gaca", and have the initial config file
     * located in the corresponding project directory.
     *
     * @return The list of paths.
     */
    static QStringList getInitialConfigSearchPaths();

    //! An application-specific usage help text.
    QString m_application_help;

    //! A pointer is used for dependency reasons.
    class AVConfig2BuiltinParams *m_builtin_params;
    class AVConfig2QtBuiltinParams *m_qt_builtin_params;
    //! nullptr if there is no project config.
    std::unique_ptr<AVConfig2ProjectFactsConfig> m_project_config;

    bool m_main_config_loaded;
    bool m_refresh_core_parameters_called;
    //! Empty if m_refresh_core_parameters_called is true; the errors to list when calling refreshCoreParameters
    //! otherwise (the application will quit in this case).
    QStringList m_configuration_errors;

    //! See keepOrDeleteConfigInstance();
    std::vector<std::unique_ptr<AVConfig2> > m_kept_config_instances;

    //! Allow thread-safe access to global config parameters.
    //! TODO CM Use this! Document thread safety for all methods...
    QMutex m_mutex;
};

///////////////////////////////////////////////////////////////////////////////
/**
 *  The base class of all user-defined parameter classes.
 *  Provides access to the AVConfig2Container config parameter registration functions via a friend
 *  mechanism.
 */
class AVLIB_EXPORT AVConfig2
{
public:

    enum
    {
        NO_VERSION = -1
    };

    //! This is public so it can also be used as constructor parameter in AVConfig2 subclasses.
    enum UnitTestConstructorSelector
    {
        UNIT_TEST_CONSTRUCTOR
    };

    ///////////////////////////////////////////////////////////////////////////////
    /**
     *  This is the container used in registerSubconfig method to register subconfig
     *  class parameters and use them to create subconfig template parameters. It keeps
     *  track of subconfig classes to prevent infinite recursion.
     */
    class AVLIB_EXPORT SubconfigTemplateContainer: public AVConfig2Container
    {
        AVDISABLECOPY(SubconfigTemplateContainer);
    public:
        SubconfigTemplateContainer();
        ~SubconfigTemplateContainer() override;

        //! Adds subconfig_class to the container.
        //!
        //! We need to keep track of subconfig classes to detect infinite recursion
        //! when creating subconfig templates.
        template <typename T>
        void registerSubconfigClass();

        //! Returns whether a config of type T was already added via addSubconfigClass.
        //! Used to detect subconfig registration infinite recursions.
        template <typename T>
        int getSubconfigClassRegistrationCount() const;

        //! Creates subconfig template parameters from the registered parameters.
        void createSubconfigTemplateParameters();

        //! Transfers subconfig template parameters from this container to the given AVConfig2LoadedData.
        //!
        //! \param loaded_data Destination where subconfig template parameters will be added.
        void transferSubconfigTemplateData(AVConfig2LoadedData& loaded_data);

    private:
        typedef std::multiset<std::type_index> SubconfigClassContainer;

        //! Container of the subconfig classes. It is used to determine what types of subconfig classes
        //! were created so that infinite recursions can be prevented.
        SubconfigClassContainer m_subconfig_class_container;
    };

protected:
    /**
     * @param prefix    The parameter prefix, separated with ".", but without a trailing dot.
     *
     *                  All parameters registered for this config object will
     *                  automatically have "<prefix>." prepended (except if
     *                  "use_prefix=false" is specified at registration time).
     *
     *                  Note that the parameter prefix corresponds to the namespaces
     *                  in the cc storage format, and is also relevant for determining the
     *                  target file when saving new parameters (see
     *                  AVConfig2Saver::findStorageLocationForMissingParameter).
     *
     * @param container The config container object to use for registering parameters.
     */
    AVConfig2(const QString& prefix, AVConfig2Container& container = AVConfig2Global::singleton());

    /**
     * Use this constructor if you want to build your configuration manually in unit tests, and an association with
     * a config container is not desired.
     *
     * Registering parameters, refreshing and all other operations which require a container association will be
     * fatal. Also see hasAssociatedContainer.
     *
     * The UnitTestConstructorSelector parameter is required to avoid accidential use of this constructor, which
     * otherwise would have no parameters.
     */
    AVConfig2(UnitTestConstructorSelector);

public:
    virtual ~AVConfig2();

    //! \return the non-negative version or NO_VERSION.
    int getVersion() const;

    //! \return It is important for the implementation to check the preconditions for any
    //!         translation, because a successful translation triggers "-save".
    //!         Returns true if the operation succeeded.
    virtual bool translateToNextVersion(uint prev_version);

    //! Call this method if any config-related action depends on already read config parameters
    //! (e.g. deciding on which config objects to instantiate).
    //!
    //! *Attention* If this method returns false, do not make any assumptions about the state of
    //!             registered config variables (some might be refreshed, others might not).
    //!
    //! Note that this call will not trigger a postRefresh() call.
    //!
    //! \param result        Pass this along to get detailed error reports. Can be 0.
    //! \return              Whether any errors occured while refreshing the config.
    bool refreshParams(AVConfig2Container::RefreshResult *result = 0);

    //! \return Whether the last call to refreshParams (if any) was successful.
    bool refreshSuccessful() const;

    /**
     * @brief postRefresh is called for every call to AVConfig2Container::refreshAllParams() (which also includes refreshCoreParameters()),
     *                    and can be implemented by clients to check / postprocess parameters directly after refresh time.
     *
     * *Attention* This method is called only via calls to refreshAllParams / refreshCoreParameters!
     *             This ensures that other config objects (e.g. any content of a subconfig map) can be accessed in
     *             the implementation.
     *
     * *Attention* Calling this method manually is not recommended:
     *
     * - It is not ensured that all references have been resolved.
     * - It is not ensured that the config is complete (e.g. sub configs might be present in files which have not yet been loaded).
     *   Even if the subconfigs have already been loaded, the subconfig map will be recreated from scratch at refresh time, rendering the
     *   previous content obsolete.
     * - The method will be called again at refresh time, and the implementation has to be able to cope with it.
     *
     * *Note* This method is only called if there was no error loading the configuration up to this point (e.g. missing references etc).
     *
     * *Note* Configuration errors which do not "belong" to a single configuration class can also
     *        be registered via AVConfig2Global::registerConfigurationError().
     *
     * *Note* This method is also called for objects created in other postRefresh() implementations or subconfig map constructors,
     *        see SWE-5437.
     *
     * @return An empty string if the config is OK, an error message otherwise (which will be displayed in the config error summary; this
     *         inhibits startup of the process when handling the global config container).
     */
    virtual QString postRefresh();

    //! \see m_config_map_key
    void setConfigMapKey(const QString& key);
    //! \see m_config_map_key
    QString getConfigMapKey() const;

    QString getPrefix() const;

    /**
     * @brief setHelpGroup changes the group in which command line parameters are displayed when using "-help".
     *
     * @param help_group The group name for the help text (just the name, e.g. "General",
     *                   not "General options").
     */
    void setHelpGroup(const QString& help_group);
    QString getHelpGroup() const;
    //! Whether to always show the help text for this config class, or only when "-help all" is
    //! used.
    bool isShowHelpPerDefaultEnabled() const;

    //! Enables version translation for this config class.
    /*!
     *  To be called by the config subclass. If newly introducing versions for a config class, use
     *  the version number 1 for the initial version.
     *  AVConfig2::translateToNextVersion will be called if an outdated version of this class is
     *  encountered in a config file while doing a refresh().
     *
     *  See the methods starting with "translate" in this class for the currently available
     *  translation steps.
     *
     *  Version translation allows restructuring the configuration, adapting it to the
     *  possibilities offered by the AVConfig2 implementation while still maintaining backwards
     *  compatibility. Additionally, it provides a more controlled tool than the brute-force
     *  "-save" when adding parameters or changing the metadata.
     *
     *  Translation currently is in stage "proof-of-concept" and has the following shortcomings:
     *  - Rename does not remove the old parameter
     *  - No translation for removing parameters
     *  - No generic API for translation
     *  - Renaming of namespace not yet supported (as the version cannot be retrieved from the old
     *    namespace, it will be neccessary to additionally specify the prefixes of previous
     *    versions).
     *  - Currently, the translation API only works for registered params (which is assumed to be
     *    the most common case and is done for convenience). So if e.g. a parameter is renamed
     *    twice, the current API will not work (also see generic API).
     *  - It currently is not supported for multiple config classes to have the same prefix.
     *
     *  Those missing features will be implemented on request.
     */
    void setVersion(uint version);

    //! Sets the flag to add the registered parameters to the removed parameters list when this config is destroyed.
    void setRemoveRegisteredParamsOnDestruction(bool remove);

    //! Returns true if all the registered paramaters for this config should be added to the removed paramaters list
    //! when this config is destroyed.
    bool shouldRemoveRegisteredParamOnDestruction() const;

    //! Registers a parameter with the config system.
    //!
    //! Note that calling this method only assigns a default value to registered variable!
    //! If you need to load the correct value before the application calls refreshCoreParameters(),
    //! use refreshParams().
    //!
    //! \param name The parameter name. Note that the config object's prefix will appended to this
    //!             name unless use_prefix is false.
    //! \param ref  A pointer to the variable which will be set to the loaded parameter's value.
    //!             Must be a member variable of this class.
    //! \param help The help text which is displayed if the cmd line option "-help" is used.
    //! \param use_prefix Whether to prepend the prefix of the config object to the parameter name.
    //!                   Set this to false e.g. for pure command line switches contained in some
    //!                   config class.
    template <typename T>
    AVConfig2Container::RegisteredParameterManipulator<T> registerParameter(
            const QString& name, T *pointer, const QString& help, bool use_prefix=true);

    //! Registers a new subconfig container with the config system.
    //!
    //! Note that calling this method does not immediately change the registered container!
    //! If you need to load the value before the application calls refreshCoreParameters(), use
    //! refreshParams().
    //!
    //! Internal note: this method is called in two contexts:
    //!
    //! - By ordinary application code registering subconfigs.
    //! - By avconfig2 code which creates subconfig templates. In this case, the container of the config object is of
    //!   type AVConfig2::SubconfigTemplateContainer.
    //!
    //! \param prefix     The prefix which must be present for all subconfigs.
    //!                   The element following the prefix in loaded parameter names will be used as key
    //!                   for the config_container, e.g.
    //!
    //!                   some.prefix.first.value = 1
    //!
    //!                   some.prefix.second.value = 2
    //!
    //!                   will result in two entries in config_container, named "first" and "second".
    //!                   See avconfig2test.cpp for examples.
    //!                   Note that it is not supported to pass an empty string for the prefix.
    //! \param config_map A reference to the container which will hold the loaded subconfigs.
    //!                   Must be a member of the registering config class. For usage examples, see
    //!                   avconfig2test.cpp, NestedSubConfig.
    //! \param container  Usage of this parameter is discouraged. It is possible to register a subconfig map on a
    //!                   different container than the parent configuration. This is e.g. used in the DSW configuration
    //!                   to work around the limitation that it is not possible to register two parameters with the
    //!                   identical name on the same container.
    //!                   On the DSW, it is possible to configure the same connection profile for different server
    //!                   configs, which would lead exactly to this situation. Instead, the first connection profile is
    //!                   registered on the global config container, and the remaining profiles are loaded separately
    //!                   from a temporary container.
    //!                   The assumption is that either all subconfigs are loaded successfully or none, so no error
    //!                   checking is done.
    template <typename T>
    void registerSubconfig(
        const QString& prefix, AVConfig2Map<T> *config_map, AVConfig2Container *container = 0);

protected:
    //! For an already registered parameter, change the suggested / default value in its metadata.
    /**
     *  For this to be useful, it must be called before saving / loading parameters.
     *
     *  \param parameter A pointer to the member variable which was registered via registerParameter before.
     *  \param suggested The new suggested value to be used in the parameter's metadata.
     */
    template <typename T>
        void changeSuggestedValue(T *parameter, const T& suggested);

    //! Configs created with UnitTestConstructorSelector do not have (or need) an associated
    //! container. This method can be used when creating additional subconfigs to select the
    //! correct constructor flavour.
    bool hasAssociatedContainer() const;
    //! Only call this method if you know what you are doing (the container must still exist).
    AVConfig2Container& getContainer();
    //! Only call this method if you know what you are doing (the container must still exist).
    const AVConfig2Container& getContainer() const;

    //! Helper method for config version translation.
    //! Adds a new parameter - just specify the pointer to the member which was also passed to the
    //! registerParameter() call.
    //!
    //! \param parameter A pointer to the member of the config class which was previously registered.
    //! \param value If this is QString::null, the default/suggested value will be used.
    //!              TODO CM is this parameter really useful?
    //!
    //! \return Wheter the translation was successfully performed.
    bool translateAddParameter(void *parameter, QString value = QString::null);
    //! Helper method for config version translation.
    //!
    //! \param parameter A pointer to the member of the config class which was previously registered.
    //! \param previous_name The previous full name of the parameter (eg "fdp2.distport")
    //!
    //! \return Wheter the translation was successfully performed.
    bool translateRenameParameter(void *parameter, QString previous_name);
    //! Helper method for config version translation.
    //!
    //! \param parameter A pointer to the member of the config class which was previously registered.
    //!
    //! \return Wheter the translation was successfully performed.
    bool translateChangeMetadata(void *parameter);

    //! The value NO_VERSION indicates that this config class is not interested in versioning.
    int m_version;

    QString m_help_group;
    //! This flag controls whether help for this config class is always shown, or only if
    //! explicitly requested via "-help all".
    //! Per default it is on, subclasses can turn this flag off in their constructor.
    bool m_show_help_per_default;

    //! Caches the result of refreshParams.
    bool m_last_refresh_params_successful;

    //! The prefix without leading or trailing '.'
    QString m_prefix;

    //! If this is a subconfig stored in a config map, this is they key under which the config
    //! is stored. QString::null otherwise.
    //!
    //! This is purely for the convenience of client code.
    QString m_config_map_key;

    //! The prefix to be used for parameters for which setDeprecatedName() is called.
    QString m_deprecated_prefix;

private:
    const AVConfig2Container::RegisteredParameterBase *getParameter(void *parameter) const;

    //! If this is true, the registered parameters will be added to the removed parameters list when this config is
    //! destroyed. Default value is false.
    bool m_remove_registered_params_on_destruction;

    //! We'd like to allow configs which live longer than their config container -> use guarded
    //! pointer.
    //! Note that there is no associated container for configs created using UnitTestConstructorSelector.
    QPointer<AVConfig2Container> m_container;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
QString AVConfig2Container::RegisteredParameter<T>::toString() const
{
    return AVToString(m_reference, true);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
bool AVConfig2Container::RegisteredParameter<T>::fromString(const QString& str)
{
    return AVFromString(str, m_reference);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
bool AVConfig2Container::RegisteredParameter<T>::areParamStringsEqual(
        const QString& s1, const QString& s2) const
{
    T t1;
    T t2;
    if (!AVFromString(s1, t1)) return false;
    if (!AVFromString(s2, t2)) return false;
    return AVToString(t1) == AVToString(t2);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
bool AVConfig2Container::RegisteredParameter<T>::pointsToVariable(void * var) const
{
    return &m_reference == var;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
bool AVConfig2Container::RegisteredParameter<T>::isSyntacticallyValid(const QString& value) const
{
    T dummy;
    return AVFromString(value, dummy);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
void AVConfig2Container::RegisteredParameter<T>::setValue(const T& value)
{
    m_reference = value;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
AVConfig2Container::RegisteredParameterManipulator<T>::RegisteredParameterManipulator(
        RegisteredParameter<T>& param, AVConfig2Container& container) :
        m_refcount(new (LOG_HERE)uint(1)),
        m_parameter(param),
        m_container(container)
{
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
AVConfig2Container::RegisteredParameterManipulator<T>::RegisteredParameterManipulator(
        const AVConfig2Container::RegisteredParameterManipulator<T>& other) :
        m_refcount(other.m_refcount),
        m_parameter(other.m_parameter),
        m_container(other.m_container)
{
    ++*m_refcount;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
AVConfig2Container::RegisteredParameterManipulator<T>::~RegisteredParameterManipulator()
{
    --*m_refcount;
    if (*m_refcount != 0) return;
    delete m_refcount;

    const AVConfig2Metadata& metadata = m_parameter.m_metadata;
    const QString& name = metadata.m_name;

    // Plausi checks
    // TODO CM why have plausi checks in setSuggestedValue etc? Why not move them all to this place?
    {
        if (m_parameter.m_pure_cmdline_option && !metadata.m_command_line_switch.isNull())
        {
            AVLogFatal << "Cannot have a pure and a simple command line option at the same time ("
                       << name << ")";
        }
        if (m_parameter.m_pure_cmdline_option)
        {
            const LoadedParameter* existing_loaded = m_container.m_loaded_data.getElementByName<LoadedParameter>(name);
            // If multiple instances of a config object are created, loaded params from previous registrations may be
            // present - ignore those. See AC-633.
            if (existing_loaded != nullptr &&
                existing_loaded->getLocation().m_source != AVConfig2StorageLocation::PS_DEFAULT_PURE &&
                existing_loaded->getLocation().m_source != AVConfig2StorageLocation::PS_CMDLINE_PARAM)
            {
                AVLogFatal << name
                           << " was registered as pure cmdline option, but already is present in loaded params with value \""
                           << existing_loaded->getValue()
                           << "\", metadata "
                           << existing_loaded->getMetadata()
                           << "\nand storage location "
                           << existing_loaded->getLocation().getFullFilePath();
            }
        }
    }

    // Perform updates to m_loaded_param
    // TODO CM move this logic into AVConfig2Container method?
    if (m_parameter.m_pure_cmdline_option)
    {
        // If this is a pure command line option, we know that the parameter name is also the command line switch
        // name. Fill in the data from any existing command line switch in m_loaded_cmdline_param. If there is none,
        // adopt the default value.
        QString value;
        AVConfig2StorageLocation::ParameterSource source;
        if (m_container.m_loaded_cmdline_param.contains(name))
        {
            value = m_container.m_loaded_cmdline_param[name];
            source = AVConfig2StorageLocation::PS_CMDLINE_PARAM;
        } else
        {
            value = metadata.m_default_value;
            AVASSERT(!value.isNull()); // because it was created via AVToString
            source = AVConfig2StorageLocation::PS_DEFAULT_PURE;
        }

        // By specifying the full metadata here, it is also ensured that there is no conflict with a param loaded
        // from file. See AVConfig2Container::checkNewLoadedParam.
        m_container.m_loaded_data.addElement(LoadedParameter(value, metadata, AVConfig2StorageLocation(source)));
    } else if (!metadata.m_command_line_switch.isNull())
    {
        // If this registration specified a command line switch, check whether the switch was given. If so, add the
        // value to m_loaded_param immediately.
        // Note that the parameter might also already have been loaded from a file (e.g. if registration is in a subconfig).
        CmdLineParamContainer::const_iterator cmdline_it = m_container.m_loaded_cmdline_param.find(metadata.m_command_line_switch);
        if (cmdline_it != m_container.m_loaded_cmdline_param.end())
        {
            QString cmdline_value = cmdline_it.value();

            LoadedParameter * existing_loaded = m_container.m_loaded_data.getElementByName<LoadedParameter>(name);
            if (existing_loaded != nullptr)
            {
                // Simply replace the used value
                existing_loaded->setOverrideValue(cmdline_value, AVConfig2StorageLocation::PS_CMDLINE_PARAM);
            } else
            {
                // Add entry to m_loaded_param.
                // Although we know the full metadata here, don't specify it. This allows detecting metadata mismatch and
                // duplicates when actually loading the parameter from file (see AVConfig2Container::checkNewLoadedParam).
                m_container.m_loaded_data.addElement(
                    LoadedParameter(cmdline_value, AVConfig2Metadata(name),
                                    AVConfig2StorageLocation(AVConfig2StorageLocation::PS_CMDLINE_PARAM)));
            }
        }
    }

    if (m_parameter.isOptional())
    {
        // If this is an optional parameter, immediately adopt the default value (if there is no override or
        // loaded parameter yet).
        // Specify incomplete metadata for the same reasons as when handling command line params.
        // Note that metadata checks are disabled for optional params if the metadata is not complete.
        LoadedParameter * existing_loaded = m_container.m_loaded_data.getElementByName<LoadedParameter>(name);
        if (existing_loaded == nullptr)
        {
            m_container.m_loaded_data.addElement(
                LoadedParameter(metadata.m_default_value, AVConfig2Metadata(name),
                                AVConfig2StorageLocation(AVConfig2StorageLocation::PS_DEFAULT_OPTIONAL)));
        }
    }
    applyDefaultRestriction(this);
    // TODO CM refresh the parameter here?
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
AVConfig2Container::RegisteredParameterManipulator<T>&
AVConfig2Container::RegisteredParameterManipulator<T>::setDeprecatedName(const QString& name)
{
    AVASSERT(!name.isEmpty());
    AVASSERT(!m_parameter.m_deprecated_names.contains(name));
    m_parameter.m_deprecated_names.append(name);
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
AVConfig2Container::RegisteredParameterManipulator<T>&
AVConfig2Container::RegisteredParameterManipulator<T>::setDeprecatedPrefix(const QString& prefix)
{
    m_parameter.m_deprecated_prefix = prefix;
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
AVConfig2Container::RegisteredParameterManipulator<T>&
AVConfig2Container::RegisteredParameterManipulator<T>::setSuggestedValue(
        const T& value)
{
    if (m_parameter.m_pure_cmdline_option)
    {
        AVLogFatal << "Cannot have a suggested value and be pure command line option at "
                   << "the same time (" << m_parameter.m_metadata.m_name << ")";
    }
    if (m_parameter.isOptional())
    {
        AVLogFatal << "Cannot have a suggested value and be optional parameter at "
                   << "the same time (" << m_parameter.m_metadata.m_name << ")";
    }
    if (!m_parameter.m_metadata.m_suggested_reference.isNull())
    {
        AVLogFatal << "Cannot set both suggested value and reference ("
                   << m_parameter.m_metadata.m_name;
    }
    m_parameter.m_metadata.m_default_value = AVToString(value, true);
    m_parameter.setValue(value);
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
AVConfig2Container::RegisteredParameterManipulator<T>&
AVConfig2Container::RegisteredParameterManipulator<T>::setSuggestedReference(
        const QString &reference)
{
    if (m_parameter.m_pure_cmdline_option)
    {
        AVLogFatal << "Cannot have a suggested reference and be pure command line option at "
                   << "the same time (" << m_parameter.m_metadata.m_name << ")";
    }
    if (m_parameter.isOptional())
    {
        AVLogFatal << "Cannot have a suggested reference and be optional parameter at "
                   << "the same time (" << m_parameter.m_metadata.m_name << ")";
    }
    if (!m_parameter.m_metadata.m_default_value.isNull())
    {
        AVLogFatal << "Cannot set both suggested value and reference ("
                   << m_parameter.m_metadata.m_name;
    }
    m_parameter.m_metadata.m_suggested_reference = reference;
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
AVConfig2Container::RegisteredParameterManipulator<T>&
AVConfig2Container::RegisteredParameterManipulator<T>::setPureCmdlineOption(
        const T& default_value)
{
    if (!m_parameter.m_metadata.m_default_value.isEmpty())
    {
        AVLogFatal << "Cannot have a suggested value and be pure command line option at "
                   << "the same time (" << m_parameter.m_metadata.m_name << ")";
    }
    if (!m_parameter.m_metadata.m_suggested_reference.isNull())
    {
        AVLogFatal << "Cannot have a suggested reference and be pure command line option at "
                   << "the same time (" << m_parameter.m_metadata.m_name << ")";
    }
    m_parameter.m_pure_cmdline_option = true;
    m_parameter.m_metadata.m_default_value = AVToString(default_value);
    m_parameter.setValue(default_value);
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
AVConfig2Container::RegisteredParameterManipulator<T>&
AVConfig2Container::RegisteredParameterManipulator<T>::setOptional(
        const T& default_value)
{
    if (!m_parameter.m_metadata.m_default_value.isEmpty())
    {
        AVLogFatal << "Cannot have a suggested value and be an optional parameter at "
                   << "the same time (" << m_parameter.m_metadata.m_name << ")";
    }
    if (!m_parameter.m_metadata.m_suggested_reference.isNull())
    {
        AVLogFatal << "Cannot have a suggested reference and be an optional parameter at "
                   << "the same time (" << m_parameter.m_metadata.m_name << ")";
    }
    m_parameter.m_metadata.m_optional = true;
    m_parameter.m_metadata.m_default_value = AVToString(default_value);
    m_parameter.setValue(default_value);

    // note that optional parameters are added to the list of loaded parameters in the destructor of this manipulator.

    return *this;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
AVConfig2Container::RegisteredParameterManipulator<T>&
AVConfig2Container::RegisteredParameterManipulator<T>::setCmdlineSwitch(
        const QString& cmd_line)
{
    AVASSERT(!cmd_line.isEmpty());
    m_parameter.m_metadata.m_command_line_switch = cmd_line;
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
AVConfig2Container::RegisteredParameterManipulator<T>&
AVConfig2Container::RegisteredParameterManipulator<T>::setRestriction(
        std::unique_ptr<const AVConfig2TypedRestriction<T> > restriction)
{
    if (m_parameter.m_metadata.m_restriction != 0)
    {
        AVLogFatal << "Tried to set two restrictions on " << m_parameter.m_metadata.m_name;
    }
    m_parameter.m_metadata.m_restriction = std::move(restriction);
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
AVConfig2Container::RegisteredParameterManipulator<T>&
AVConfig2Container::RegisteredParameterManipulator<T>::setAvPortRestriction()
{
    // TODO CM use values from https://confluence.avibit.com:2233/x/soQiAQ here...
    return setRestriction(std::make_unique<AVConfig2RangeRestriction<T>>(7000, 9999));
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
AVConfig2Container::RegisteredParameterManipulator<T>&
AVConfig2Container::RegisteredParameterManipulator<T>::setMin(const T& min)
{
    std::unique_ptr<AVConfig2RangeRestriction<T>> r(std::make_unique<AVConfig2RangeRestriction<T>>());
    r->setMin(min);
    return setRestriction(std::move(r));
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
AVConfig2Container::RegisteredParameterManipulator<T>&
AVConfig2Container::RegisteredParameterManipulator<T>::setMax(const T& max)
{
    std::unique_ptr<AVConfig2RangeRestriction<T>> r(std::make_unique<AVConfig2RangeRestriction<T>>());
    r->setMax(max);
    return setRestriction(std::move(r));
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
AVConfig2Container::RegisteredParameterManipulator<T>&
AVConfig2Container::RegisteredParameterManipulator<T>::setMinMax(
        const T& min, const T& max)
{
    return setRestriction(std::make_unique<AVConfig2RangeRestriction<T>>(min, max));
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
AVConfig2Container::RegisteredParameterManipulator<T>&
AVConfig2Container::RegisteredParameterManipulator<T>::setValidValues(
        const QList<T>& values)
{
    return setRestriction(std::make_unique<AVConfig2ValueListRestriction<T>>(values));
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
AVConfig2Container::RegisteredParameterManipulator<T>&
AVConfig2Container::RegisteredParameterManipulator<T>::setRegexPattern(
        const QString& regex_pattern)
{
    std::unique_ptr<AVConfig2RegexRestriction<T>> r = std::make_unique<AVConfig2RegexRestriction<T>>();
    r->setRegexPattern(regex_pattern);
    return setRestriction(std::move(r));
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
AVConfig2Container::RegisteredParameterManipulator<T> AVConfig2Container::registerParameter(
        const QString& name, T& ref, const QString& help, AVConfig2 *config)
{
    AVASSERT(m_config_object.contains(config));
    AVASSERT(!name.isEmpty());
    if (name.contains(' '))
    {
        AVLogFatal << "AVConfig2Container::registerParameter: " << name << " contains whitespace";
    }

    if (help.trimmed().startsWith("\\"))
    {
        // See AVConfig2ImporterCstyle::readHelp().
        AVLogFatal << "Help text must not start with \\";
    }

    // Prepare initial metadata. Note that this is not the final metadata, as clients can still
    // operate on the returned registered parameter object.
    AVConfig2Metadata metadata(name, AVConfig2Types::getTypeName<T>(), help);
    // Verify that the type is known to the config system. If it's not, it is missing a
    // specialization in AVConfig2Types.
    //
    // TODO CM is it OK to call exit here? This might be called after program initialization.
    // Make behaviour dependent on whether logger is initialized?
    if (metadata.m_type.isNull())
    {
        std::cout << "AVConfig2::registerParameter: "
                  << "Parameter \"" << qPrintable(name)
                  << "\" has a type not known to the config system.\n";
        exit (-1);
    }

    m_unrefreshed.insert(config);

    RegisteredParameter<T> *new_param = new (LOG_HERE) RegisteredParameter<T>(ref, metadata);

    // Now check whether this is a duplicate - bail ungracefully if yes.
    // We don't want duplicate names since parameter values are potentially saved back to the configuration files.
    QPair<const AVConfig2*, const RegisteredParameterBase*> existing_parameter = getRegisteredParameterInfo(name);
    if (existing_parameter.second != 0)
    {
        if (existing_parameter.first == config)
        {
            // This most likely is an accident, e.g. copy&paste error
            AVLogFatal << "AVConfig2::registerParameter: A parameter named \"" << name
                       << "\" already was registered for this config object!";
        } else if (strcmp(typeid(*config).name(),
                          typeid(*existing_parameter.first).name()) != 0)
        {
            // A config object which has nothing to do with ours already used the name
            //
            // *ATTENTION* This can also be triggered when using inheritance and registering parameters in the config constructor
            //             (since the type is not yet the same if construction is ongoing). See eg DIFLIS-5909.
            //             Workaround: don't register parameters in the constructor, but in a separate method call.
            //             Better solution TBD.
            AVLogFatal << "AVConfig2::registerParameter: A parameter named \"" << name
                       << "\" already was registered in " << typeid(*existing_parameter.first).name()
                       << "(this is " << typeid(*config).name() << ")";
        } else
        {
            AVLogInfo << "AVConfig2::registerParameter: A parameter named \"" << name
                      << "\" already was registered for this config. Disabling saving of duplicate.";
            new_param->disableSaving();
        }
    }

    // Check if the same variable is not registered using different names
    RegisteredParameterList& parameter_list = m_registered_param.getParameterList(config);
    for (const QSharedPointer<RegisteredParameterBase>& parameter : parameter_list)
    {
        if (parameter->pointsToVariable(&ref))
        {
            AVLogFatal << "AVConfig2Container::registerParameter: trying to register a parameter " << name
                       << " but the class member is already registered using a different name: "
                       << parameter->getMetadata().m_name;
        }
    }

    QSharedPointer<RegisteredParameterBase> param_pointer(new_param);
    m_registered_param.getParameterList(config).push_back(param_pointer);
    m_registered_param_names.insert(name);
    m_removed_param_names.removeOne(name);

    return RegisteredParameterManipulator<T>(*new_param, *this);
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
void AVConfig2Container::registerSubconfig(
        const QString& prefix, AVConfig2Map<T>& config_map,
        AVConfig2 *config)
{
    AVASSERT(m_config_object.contains(config));

    m_unrefreshed.insert(config);

    RegisteredSubconfigBase *new_subconfig =
        new (LOG_HERE) RegisteredSubconfig<T>(prefix, config_map, *this);

    // Now check whether this is a duplicate - bail ungracefully if yes.
    QPair<const AVConfig2*, const RegisteredSubconfigBase*> existing_subconfig = getRegisteredSubconfigInfo(prefix);
    if (existing_subconfig.second != 0)
    {
        if (existing_subconfig.first == config)
        {
            // This most likely is an accident, e.g. copy&paste error
            AVLogFatal << "AVConfig2::registerSubconfig: A subconfig named \"" << prefix
                       << "\" already was registered for this config object!";
        } else if (strcmp(typeid(*config).name(),
                          typeid(*existing_subconfig.first).name()) != 0)
        {
            // A config object which has nothing to do with ours already used the name
            //
            // *ATTENTION* This can also be triggered when using inheritance and registering subconfigs in the config constructor
            //             (since the type is not yet the same if construction is ongoing). See eg DIFLIS-5909.
            //             Workaround: don't register subconfigs in the constructor, but in a separate method call.
            //             Better solution TBD.
            AVLogFatal << "AVConfig2::registerSubconfig: A subconfig named \"" << prefix
                       << "\" already was registered in " << typeid(*existing_subconfig.first).name()
                       << "(this is " << typeid(*config).name() << ")";
        }
    }

    QSharedPointer<RegisteredSubconfigBase> subconfig_pointer(new_subconfig);
    m_registered_subconfig[config].push_back(subconfig_pointer);
    m_registered_subconfig_names.insert(prefix);

    // create subconfig template entries

    if (m_loaded_data.listSubconfigTemplates(prefix).isEmpty())
    {
        // check whether we already are in a "create subconfig template" call stack. If yes, just add our stuff.
        // Otherwise, create the helper container, recursively register subconfigs, then extract the templates and
        // clean up.

        // this owning pointer is only used in top-level call which initiates the "create subconfig template" call stack
        std::unique_ptr<AVConfig2::SubconfigTemplateContainer> top_level_owning_container;
        AVConfig2::SubconfigTemplateContainer* template_container = dynamic_cast<AVConfig2::SubconfigTemplateContainer*>(this);
        if (template_container == nullptr)
        {
            // top level call
            top_level_owning_container = std::make_unique<AVConfig2::SubconfigTemplateContainer>();
            template_container = top_level_owning_container.get();
        }

        // recursively create any subconfig templates
        // limit recursion depth, allow two instances of the same class (this is a bit arbitrary)
        if (template_container->getSubconfigClassRegistrationCount<T>() < 2)
        {
            template_container->registerSubconfigClass<T>();
            std::unique_ptr<T> subconfig_class(template_container->createSubconfigClass<T>(prefix + ".*"));
            // create subconfig template parameters before subconfig_class is destroyed
            // it can't be done later because registered parameters are removed when subconfig_class is destroyed
            template_container->createSubconfigTemplateParameters();
        }

        // back on top level, transfer templates
        if (top_level_owning_container.get())
        {
            // transfer all created subconfig template parameters from the temporary container
            template_container->transferSubconfigTemplateData(m_loaded_data);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
void AVConfig2Container::changeSuggestedValue(
    T& ref, const QString& suggested, AVConfig2 *config)
{
    AVASSERT(m_config_object.contains(config));
    if (!m_unrefreshed.contains(config))
    {
        AVLogFatal << "Cannot change suggested value after refreshing params";
    }

    // Find the parameter list for the config object
    AVASSERT(m_registered_param.containsConfig(config));
    RegisteredParameterList& parameter_list = m_registered_param.getParameterList(config);

    // Find the registered parameter itself
    RegisteredParameterBase *registered_parameter = 0;
    for (RegisteredParameterList::iterator param_it = parameter_list.begin();
         param_it != parameter_list.end(); ++param_it)
    {
        RegisteredParameterBase& cur_param = **param_it;
        if (!cur_param.pointsToVariable(&ref)) continue;
        registered_parameter = &cur_param;
    }

    // change the suggested value
    AVASSERT(registered_parameter != 0);
    registered_parameter->getMetadata().m_default_value = suggested;

    // Now change the value of any loaded parameter already put into place. Do it only if the default value is of
    // relevance.
    QString param_name = registered_parameter->getMetadata().m_name;
    LoadedParameter * existing_loaded = m_loaded_data.getElementByName<LoadedParameter>(param_name);
    if (existing_loaded != nullptr)
    {
        if (existing_loaded->getLocation().m_source == AVConfig2StorageLocation::PS_DEFAULT_OPTIONAL ||
            existing_loaded->getLocation().m_source == AVConfig2StorageLocation::PS_DEFAULT_PURE)
        {
            *existing_loaded = LoadedParameter(suggested, existing_loaded->getMetadata(), existing_loaded->getLocation());
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
AVConfig2Container::RegisteredParameterManipulator<T> AVConfig2::registerParameter(
        const QString& name, T *pointer, const QString& help, bool use_prefix)
{
    AVASSERT(m_container);
    AVASSERT(pointer != 0);
    AVASSERT(!help.trimmed().isEmpty());
    // See PTS#4918
    *pointer = AVConfig2Types::getInitValue<T>();

    QString full_name;
    if (m_prefix.isEmpty() || !use_prefix) full_name = name;
    else                                   full_name = m_prefix + "." + name;

    AVConfig2Container::RegisteredParameterManipulator<T> ret2 =
            m_container->registerParameter(full_name, *pointer, help, this);

    // Force compiler to use copy constructor.
    // See AVConfig2Container::RegisteredParameterManipulator<T>::m_refcount.
    AVConfig2Container::RegisteredParameterManipulator<T> ret(ret2);

    if (!m_deprecated_prefix.isNull())
    {
        // we want the deprecated prefix to be used even if no deprecated name is explicitly given for the parameter
        ret.setDeprecatedPrefix(m_deprecated_prefix).setDeprecatedName(name);
    } else
    {
        // if the user gives a deprecated name, use the ordinary prefix
        ret.setDeprecatedPrefix(m_prefix);
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
T* AVConfig2Container::createSubconfigClass(const QString& prefix)
{
    T* subconfig_class = new (LOG_HERE) T(prefix, *this);
    return subconfig_class;
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
void AVConfig2::SubconfigTemplateContainer::registerSubconfigClass()
{
    m_subconfig_class_container.insert(std::type_index(typeid(T)));
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
int AVConfig2::SubconfigTemplateContainer::getSubconfigClassRegistrationCount() const
{
    return static_cast<int>(m_subconfig_class_container.count(std::type_index(typeid(T))));
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
void AVConfig2::registerSubconfig(
        const QString& prefix, AVConfig2Map<T> *config_map, AVConfig2Container *container)
{
    AVASSERT(m_container);
    AVASSERT(m_container.data() != container);
    AVASSERT(config_map != 0);
    if (prefix.isEmpty())
    {
        AVLogger->Write(LOG_ERROR, "AVConfig2Container::registerSubconfig: "
                "Refusing to register a subconfig with empty prefix.");
        return;
    }
    if (container == nullptr)
    {
        container = m_container;
    } else
    {
        container->addConfigObject(this);
    }
    container->registerSubconfig(prefix, *config_map, this);
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
void AVConfig2::changeSuggestedValue(T *parameter, const T& suggested)
{
    AVASSERT(m_container);
    AVASSERT(parameter != 0);
    m_container->changeSuggestedValue(*parameter, AVToString(suggested), this);
}

///////////////////////////////////////////////////////////////////////////////

/*!
 Use this macro outside of any definition in the implementation file
 to register code snippet which will be run on application startup
 e.g. REGISTER_CONFIG_INITIALIZER( AVRunwayConfig::initializeSingleton() )
  */
#define REGISTER_CONFIG_INITIALIZER(...)                                          \
static int _config_name_##__COUNTER__ = [] {                                      \
    AVConfig2Global::registerConfigInitializer([]()->void{__VA_ARGS__;});\
    return 0;                                                                     \
}();

/*!
 Use this macro outside of any definition in the implementation file
 to register code snippet which will be run on application startup
 e.g. REGISTER_CONFIG_INITIALIZER( SomeGlobalConfig )
  */
#define REGISTER_CONFIG_SINGLETON(TYPE) REGISTER_CONFIG_INITIALIZER(TYPE::getInstance())

#endif

// End of file
