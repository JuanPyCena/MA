///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \brief   Declaration of the AVEnvString
*/

#if !defined(AVENVSTRING_H_INCLUDED)
#define AVENVSTRING_H_INCLUDED

// Qt includes
#include <qstring.h>
#include <qregexp.h>

#include "avlib_export.h"
#include "avfromtostring.h"

///////////////////////////////////////////////////////////////////////////////
//! This class can be used to work with strings containing environment variables
/*! The main use case is:

    <ul>
    <li> Expand a string, so that all contained environment variables are substituted by their
         values.
    </li>
    </ul>
*/
class AVLIB_EXPORT AVEnvString
{
public:

    //! Default constructor
    AVEnvString();

    //! Constructor
    explicit AVEnvString(const QString& env_string);

    //! Destructor
    virtual ~AVEnvString(){}

    /*!
     *  \brief returns if there is an envString or not
     *  \return true if m_env_string is empty
     */
    bool isEmpty() const;

    //! Sets the envString to work with
    void setEnvString(const QString& new_env_string);

    //! Returns the set envString
    QString envString() const;

    //! Expands the envString, so that all environment variables are substituted with their values.
    /*! Only environmemt variables conforming to the following naming convention are recognized:
        <ul>
        <li> Only captial letters, digits and underscore are allowed </li>
        <li> every variable name starts with a letter </li>
        <li> The whole variable name is enclosed in \$() e.g. \$(AVCOMMON_HOME) </li>
        </ul>
    */
    QString expandEnvString() const;

protected:

    //! Gets the value of the given environment variable
    /*! \return true if no errors occurred, which does not mean that \e<value>
                may not be empty
    */
    virtual bool getEnv(const QString& name, QString& value) const;

protected:

    QString m_env_string; //!< the set envString
    mutable QRegExp m_reg_exp_env_name; //!< regular expression matching environment variables
};

template<> QString AVLIB_EXPORT AVToString(const AVEnvString& arg, bool enable_escape);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, AVEnvString& arg);

Q_DECLARE_METATYPE(AVEnvString)

///////////////////////////////////////////////////////////////////////////////

#endif

// End of file

