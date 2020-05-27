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

#ifndef AVCONFIG2METADATA_INCLUDED
#define AVCONFIG2METADATA_INCLUDED

#include <memory>

#include <QString>

#include "avlib_export.h"
#include "avfromtostring.h"

class AVConfig2RestrictionBase;

///////////////////////////////////////////////////////////////////////////////
/**
 *  This contains all metadata which can be read directly from the config file, without the need
 *  for registered parameters.
 *
 *  For easier usage, this class implements value semantics.
 */
class AVLIB_EXPORT AVConfig2Metadata
{
public:
    AVConfig2Metadata();
    //! Constructs an incomplete metadata object. See isIncomplete().
    AVConfig2Metadata(const QString& name);
    AVConfig2Metadata(const QString& name, const QString& type, const QString& help);
    AVConfig2Metadata(const AVConfig2Metadata& other);
    virtual ~AVConfig2Metadata();

    AVConfig2Metadata& operator=(const AVConfig2Metadata& other);

    bool operator==(const AVConfig2Metadata& other) const;

    //! *Attention* Behaviour is different from operator==(), e.g. the parameter name itself and the user
    //!             comment are not compared!
    //!
    //! Used to perform metadata consistency checks (loaded vs registered; identical type metadata
    //! for references).
    //!
    //! For restrictions, the string value are compared, not the restriction themselves.
    //! This makes it possible to compare loaded restrictions (which currently lack type
    //! information and are of type AVConfig2UntypedRestriction) with registered restrictions.
    //! User comments are never compared.
    //!
    //! \param other  Compare to this metadata object.
    //! \param strict For some purposes (e.g. comparing referenced parameter's metadata), a less
    //!               rigid comparison is needed. If this is true, only check type and restrictions.
    //! \return Whether the metadata is equal, given the compare mode.
    bool isEquivalent(const AVConfig2Metadata& other, bool strict) const;

    void setHelp(const QString& help);
    QString getHelp() const;

    //! The metadata can be incomplete if it is (yet) unknown, e.g. for "--" cmdline overrides or config overrides.
    //! Both are added to AVConfig2Container::m_loaded_param as soon as they are encountered.
    //! This is expressed by a null m_type.
    bool isIncomplete() const;

    QString m_name; //!< The full name of the parameter.
    //! The type if the parameter. See AVConfig2Types.
    //! Can be QString::null - see isIncomplete.
    QString m_type;
    //! When adding a new parameter, the default value can be used to indicate the value which
    //! results in unchanged process behaviour. It will be displayed when a process refuses to
    //! start because of a missing parameter.
    //! In addition, this value will be used for pure command-line parameters and optional
    //! parameters if nothing is specified on the command line / in the config file.
    //! If this is QString::null it means that the parameter has no suggested value. This must not
    //! be the case for pure command line options and optional parameters.
    //! Note that this is the representation obtained with AVToString() for the passed value. This
    //! makes it possible e.g. to have QString::null as default value.
    //! Mutally exclusive with m_suggested_reference.
    QString m_default_value;
    //! Only valid for "real" parameters, not for pure command-line parameters or optional parameters.
    //! When saving an unknown parameter, don't use a specific value but save it as reference to this
    //! parameter. Useful e.g. for referring default ports (see SWE-2211 and SWE-2055).
    //! Mutally exclusive with m_default_value.
    QString m_suggested_reference;
    //! See RegisteredParameterManipulator::setOptional().
    bool m_optional;
    //! This is the name of the param on the command line.
    //! For pure command line options, this will be empty, and the parameter name itself is
    //! used as command line switch.
    //! For all other parameters, if this is null, the parameter can't be set via the command
    //! line and won't appear in the application help output.
    QString m_command_line_switch;
    //! This is a user comment which was read from the config file, and should be saved back
    //! again. It always is QString::null for metadata of registered parameters.
    QString m_user_comment;

    //! Will be nullptr if there is no restriction set.
    std::unique_ptr<const AVConfig2RestrictionBase> m_restriction;

private:
    //! Note that the help text is always treated with simplifyWhitespace() to avoid metadata
    //! compare issues (the help text is broken along whitespace in the config file).
    QString m_help;
};

template<>
QString AVLIB_EXPORT AVToString(const AVConfig2Metadata& arg, bool enable_escape);

#endif // AVCONFIG2METADATA_INCLUDED

// End of file
