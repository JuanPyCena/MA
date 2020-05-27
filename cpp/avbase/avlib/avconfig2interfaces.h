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
 */

#ifndef AVCONFIG2INTERFACES_INCLUDED
#define AVCONFIG2INTERFACES_INCLUDED

#include <QString>

#include "avlib_export.h"
#include "avmacros.h"
#include "avconfig2metadata.h"
#include "avconfig2storagelocation.h"


///////////////////////////////////////////////////////////////////////////////
/**
 *  This is the interface for all classes which want to process imported config parameters
 *  ("builder"). It is used by AVConfig2ImporterBase implementations to build a representation
 *  of the loaded configuration.
 *
 *  The types of importer client currently are the AVConfig2Container, for ordinary parameter
 *  loading, and the AVConfig2ExporterBase implementations for the different config formats.
 *  Exporters are "pre-loaded" with the existing configuration before adopting the application
 *  config and saving it out again.
 *
 *  The source location of each added entity is directly passed to the methods as a parameter. This
 *  avoids issues with nested loading (as occurs when handling includes).
 */
class AVLIB_EXPORT AVConfig2ImporterClient
{
public:
    virtual ~AVConfig2ImporterClient() {}

    virtual void addParameter(const QString& value, const AVConfig2Metadata& metadata,
                              const AVConfig2StorageLocation& location) = 0;
    virtual void addReference(const QString& ref,   const AVConfig2Metadata& metadata,
                              const AVConfig2StorageLocation& location) = 0;
    //! Add a path to be searched when looking for config files
    virtual void addSearchPath(
            const QString& path, const AVConfig2StorageLocation& location) = 0;
    //! Tell the importer client that an include directive was encountered.
    //!
    //! It is the responsibility of the client to actually load this included file (if so desired).
    //!
    //! Note that this will be called with the non-mapped include file, which is the desirable
    //! behaviour, as the ensuing import will resolve the mapping.
    virtual void addInclude(const QString& include, const AVConfig2StorageLocation& location) = 0;
    //! Tell the importer client that a config mapping was encountered.
    //! For "ordinary" config files, the mapping just maps the name (without any file extension), to
    //! be flexible regarding the exact format of the file / source.
    //! For any other mapping (e.g. map files, action button rules, etc.), the whole file name is
    //! mapped.
    //!
    //! Theoretically, it would be possible to avoid mappings for ordinary config files and use
    //! includes instead. This mechanism is implemented nevertheless for ordinary configs for the
    //! following reasons:
    //! - Consistency
    //! - Backwards compatibility with the "-config" mechanism
    //! - Supports the usage scenario in which each process still loads "his" files, and there is no
    //!   "master include" file which loads all required parameters.
    //!
    //! Note that there are several possibilities to represent the same mapping, using a combination
    //! of mapped_dir and mapped_name, e.g. the following are equivalent (using cstyle syntax):
    //!
    //! #map lowi
    //! euilib
    //! #endmap
    //!
    //! #map
    //! euilib : lowi/euilib
    //! #endmap
    //!
    //! \param map_from    The name of the config which should be mapped, as requested by a process
    //!                    in a call to loadConfig (see AVConfig2StorageLocation::m_name)
    //! \param mapped_dir  The directory the config is mapped to. Can be absolute or relative, or
    //!                    empty to specify no dir.
    //! \param mapped_name The name of the mapped config. Can be relative or empty to use
    //!                    map_from as name.
    //! \param location    The location where the config file mapping was read from.
    virtual void addConfigMapping(const QString& map_from, const QString& mapped_dir,
                                  const QString& mapped_name,
                                  const AVConfig2StorageLocation& location) = 0;

    //! Developer feature: has the same effect as specifying "--name value" on the command line.
    //! This is fatal if the parameter already has been loaded.
    virtual void addOverride(const QString& name, const QString& value,
                             const AVConfig2StorageLocation& location) = 0;

    //! Developer feature: set a fact directly in a config file
    virtual void addDefine(const QString& name, const QString& value,
                           const AVConfig2StorageLocation& location) = 0;

    //! Copies parameters from one section to another (=duplicates them with different names).
    /**
     *  Duplicated parameters are marked as such, so they can be treated specially when saving back.
     *
     *  \param from All parameters with this prefix will be copied ("parent section")
     *  \param to   The copied parameters are stripped of their "from" prefix, and the "to" prefix is prepended.
     */
    virtual void addInheritedSection(const QString& from, const QString& to, const AVConfig2StorageLocation& ) = 0;

    //! Adds subconfig template. The name of subconfig template parameter must contain at least one '*' instead of
    //! a namespace. It means that in that place there could be any namespace. Example: alertchecker.filters.*.enabled
    virtual void addSubconfigTemplateParameter(const QString& value, const AVConfig2Metadata& metadata,
                                               const AVConfig2StorageLocation& location) = 0;

    class AVLIB_EXPORT LoadedElement
    {
    public:
        LoadedElement() {}
        explicit LoadedElement(const AVConfig2StorageLocation& location);
        virtual ~LoadedElement() {}

        virtual bool operator==(const LoadedElement& other) const = 0;
        bool operator!=(const LoadedElement& other) const { return !operator==(other); }

        //! \return The name under which the element is accessible in AVConfig2LoadedData.
        virtual QString getName() const = 0;
        //! Call the importer's addXXX method for this loaded element
        virtual void addToImporterClient(AVConfig2ImporterClient& client) const = 0;

        virtual bool isLoadedParameter() const { return false; }
        virtual bool isLoadedReference() const { return false; }

        //! See AVConfig2StorageLocation member docs for explanation of values.
        void setLocation(const QString& loc, const QString& name, AVConfig2Format format);
        const AVConfig2StorageLocation &getLocation() const;
        AVConfig2StorageLocation &getLocationForModification();

    protected:
        AVConfig2StorageLocation m_location;
    };

	class AVLIB_EXPORT LoadedParameter : public LoadedElement
    {
    public:

        LoadedParameter();
        LoadedParameter(const QString& value, const AVConfig2Metadata& metadata,
                        const AVConfig2StorageLocation& location);
        virtual ~LoadedParameter() {}

        bool operator==(const LoadedElement& other) const override;

        void setOverrideValue(const QString& value, AVConfig2StorageLocation::ParameterSource source);

        QString getName() const override;
        void setName(const QString& name);

        void addToImporterClient(AVConfig2ImporterClient& client) const override;

        bool isLoadedParameter() const override final { return true; }

        const QString getValue() const;
        void setValue(const QString& value);

        const AVConfig2Metadata& getMetadata() const;
        /**
         * *ATTENTION* This method leaves the parameter's name (which is also part of the metadata) untouched,
         *             it is not meant to be used for renaming. Also see SWE-5787.
         */
        void updateMetadata(const AVConfig2Metadata& metadata);

        //! TODO CM document why this is necessary, and what it does!
        QString getOriginalValue() const;
        //! Only use this if you know what you are doing...
        //! TODO CM document why this is necessary, and what it does!
        void setOriginalValue(const QString& value);

    private:
        QString m_value;
        //! The original value loaded from file.
        //! TODO CM document why this is necessary, and what it does!
        QString m_original_value;
        AVConfig2Metadata m_metadata;
    };

	class AVLIB_EXPORT LoadedReference : public LoadedElement
    {
    public:
        LoadedReference();
        LoadedReference(const QString& referenced_param, const AVConfig2Metadata& metadata,
                        const AVConfig2StorageLocation& location);
        virtual ~LoadedReference() {}

        bool operator==(const LoadedElement& other) const override;

        QString getName() const override;
        void    addToImporterClient(AVConfig2ImporterClient& client) const override;

        bool isLoadedReference() const override final { return true; }

        QString m_referenced_param;
        AVConfig2Metadata m_metadata;
    };

	class AVLIB_EXPORT LoadedSearchPath : public LoadedElement
    {
    public:
        LoadedSearchPath();
        LoadedSearchPath(const QString& path, const AVConfig2StorageLocation& location);
        virtual ~LoadedSearchPath() {}

        bool operator==(const LoadedElement& other) const override;

        QString getName() const override;
        void    addToImporterClient(AVConfig2ImporterClient& client) const override;

        //! This is the path prepared for direct usage: environment variables are resolved, the path is absolute.
        //! If the path is specified as relative in the config dir, the location of the containing config file is
        //! relevant for building the absolute path.
        QString expandPath() const;
        //! This is the path as specified in the config file. Can be absolute or relative, can contain environment
        //! variables.
        QString getPath() const;

    private:
        QString m_path;
    };

	class AVLIB_EXPORT LoadedInclude : public LoadedElement
    {
    public:
        LoadedInclude();
        LoadedInclude(const QString& include, const AVConfig2StorageLocation& location);
        virtual ~LoadedInclude() {}

        bool operator==(const LoadedElement& other) const override;

        QString getName() const override;
        void    addToImporterClient(AVConfig2ImporterClient& client) const override;

        QString m_include;
    };

    //! See AVConfig2ImporterClient::addConfigMapping for documentation of this class.
	class AVLIB_EXPORT LoadedMapping : public LoadedElement
    {
    public:
        LoadedMapping();
        //! Note that both mapped_dir and mapped_name can be empty for identity mappings.
        //! See AVConfig2Container::getAndFlagMappedConfigFile.
        LoadedMapping(const QString& from, const QString& mapped_dir, const QString& mapped_name,
                      const AVConfig2StorageLocation& location);
        virtual ~LoadedMapping() {}

        bool operator==(const LoadedElement& other) const override;

        QString getName() const override;
        void    addToImporterClient(AVConfig2ImporterClient& client) const override;

        //! Constructs the full mapping by combining m_mapped_dir and m_mapped_name/m_from.
        QString getFullMapping() const;

        QString m_from;
        QString m_mapped_dir;
        QString m_mapped_name;
    };

	class AVLIB_EXPORT LoadedOverride : public LoadedElement
    {
    public:
        LoadedOverride();
        LoadedOverride(const QString& name, const QString& value, const AVConfig2StorageLocation& location);
        virtual ~LoadedOverride() {}

        bool operator==(const LoadedElement& other) const override;

        QString getName() const override;
        void    addToImporterClient(AVConfig2ImporterClient& client) const override;

        QString m_name;
        QString m_value;
    };

	class AVLIB_EXPORT LoadedDefine : public LoadedElement
    {
    public:
        LoadedDefine();
        LoadedDefine(const QString& name, const QString& value, const AVConfig2StorageLocation& location);
        virtual ~LoadedDefine() {}

        bool operator==(const LoadedElement& other) const override;

        QString getName() const override;
        void    addToImporterClient(AVConfig2ImporterClient& client) const override;

        QString m_name;
        QString m_value;
    };

	class AVLIB_EXPORT LoadedInheritance : public LoadedElement
    {
    public:
        LoadedInheritance();
        LoadedInheritance(const QString& parent, const QString& section, const AVConfig2StorageLocation& location);
        virtual ~LoadedInheritance() {}

        bool operator==(const LoadedElement& other) const override;

        QString getName() const override;
        void    addToImporterClient(AVConfig2ImporterClient& client) const override;

        QString m_parent;
        QString m_section;
    };

    class AVLIB_EXPORT LoadedSubconfigTemplateParameter : public LoadedElement
    {
    public:
        LoadedSubconfigTemplateParameter();
        LoadedSubconfigTemplateParameter(const QString& value, const AVConfig2Metadata& metadata,
                                         const AVConfig2StorageLocation& location);
        ~LoadedSubconfigTemplateParameter() override;

        bool operator==(const LoadedElement& other) const override;

        QString getName() const override;
        void    addToImporterClient(AVConfig2ImporterClient& client) const override;

        const QString& getValue() const;
        void setValue(const QString& value);

        const AVConfig2Metadata& getMetadata() const;
        AVConfig2Metadata& getMetadata();
        void setMetadata(const AVConfig2Metadata& metadata);

    private:
        QString           m_value;
        AVConfig2Metadata m_metadata;
    };
};

///////////////////////////////////////////////////////////////////////////////
/**
 *  Create a subclass to read the parameters for AVConfig2Container from a new data source
 *  ("director").
 *
 *  Note that importers are not created with full file paths, but rather "config source names". This
 *  allows easier saving in a different format.
 */
class AVLIB_EXPORT AVConfig2ImporterBase
{
    AVDISABLECOPY(AVConfig2ImporterBase);
    friend class AVConfig2Test;

public:
    explicit AVConfig2ImporterBase(const AVConfig2StorageLocation& location) :
        m_client(0), m_location(location) {}
    virtual ~AVConfig2ImporterBase() {}

    //! In the implementation, subclasses should read their config and call m_client's corresponding
    //! methods for each parameter / reference.
    //! Note that failure to read the config should not be fatal, or the "-save" mechanism won't
    //! work as expected.
    //! TODO CM specify whether import failure should be fatal? return value?
    virtual void import() = 0;

    //! Sets the object which will receive the imported parameters / references.
    void setClient(AVConfig2ImporterClient *client) { m_client = client; }

    //! Loads the file from the given location to the given importer client.
    //! \return True if a file could be found, false otherwise. True does not imply that the importer client
    //!         successfully loaded the config.
    static bool import(AVConfig2ImporterClient *client, const AVConfig2StorageLocation& location);

    //! Convenience method, behaves as the above method.
    //!
    //! Currently works for absolute paths only. Will return 0 if the given file is not an avconfig2 file.
    static bool import(AVConfig2ImporterClient *client, const QString& filename);

protected:

    AVConfig2ImporterClient *m_client;
    AVConfig2StorageLocation m_location;

private:

    //! Factory method to create an importer to read from the given location.
    //! It is an error to pass a location with CF_LAST.
    //! This method does not check whether the location actually is valid and the file exists.
    //! The returned pointer must be deleted by the user.
    //!
    //! \return An importer primed to read from the given location.
    static AVConfig2ImporterBase *createImporter(const AVConfig2StorageLocation& location);
};

///////////////////////////////////////////////////////////////////////////////
/**
 *  Implementations of this interface should build an internal representation of a config file
 *  which then can be e.g. saved to disk or displayed via stdout.
 */
class AVLIB_EXPORT AVConfig2ExporterBase : public AVConfig2ImporterClient
{
    AVDISABLECOPY(AVConfig2ExporterBase);

public:

    //! Controls how parameters are handled which are added multiple times (this is the case
    //! when saving parameters to a preloaded config file).
    //!
    //! This is used by the AVConfig2Saver class to control whether parameters are stored in loaded or
    //! registered order.
    //!
    //! ATTENTION: currently this is respected by the cstyle config implementation only.
    enum OrderPolicy
    {
        //! Existing parameters are replaced. This means the initial order
        //! is kept. Default behaviour.
        OP_REPLACE,
        //! Existing parameters are removed and added at the end.
        OP_REMOVE_AND_APPEND
    };

    AVConfig2ExporterBase();

    //! \return A string representation of the config.
    virtual QString doExport() = 0;
    //! TODO CM remove?
    virtual void reset()       = 0;
    //! TODO CM this currently doesn't work correctly in conjunction with facts.
    //!
    //! \return Whether the parameter was found and removed.
    virtual bool removeParameter(
        const QString& full_name, const AVConfig2StorageLocation::FactConditionContainer& facts)
    {
        Q_UNUSED(full_name);
        Q_UNUSED(facts);
        return false;
    }

    void setOrderPolicy(OrderPolicy policy);

    //! Sets the list of removed parameter names. These parameters won't be added to the exporter.
    //! \param removed_param_names The list of full parameter names
    void setRemovedParamNames(const QStringList& removed_param_names);

protected:
    OrderPolicy m_order_policy;
    //! The list of full parameter names. They were removed so they shouldn't be added to the exporter.
    QStringList m_removed_param_names;
};

#endif // AVCONFIG2INTERFACES_INCLUDED

// End of file
