///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Thomas Pock, t.pock@avibit.com
    \author  QT4-PORT: Alexander Randeu, a.randeu@avibit.com
    \brief   Class to check an existing license file
*/

//! This class is used for validating an license file
/*! The class is derived from AVConfigBase. That means, that the license file
    has got the common config file design. The license is always stored in
    the file 'license.cfg'.
*/

#ifndef AVLICENSE_H
#define AVLICENSE_H

// qt includes
#include <qstring.h>
#include <qobject.h>

// avcommon includes
#include "avconfig.h"

///////////////////////////////////////////////////////////////////////////////
//! This class is used to check the validity of one or more licenses
/*! After creating the object the function checkLicense can be used to
    check the validity of a license specified by its feature name.
    If no feature name is given, all features found are validated.
    This Class is derived from AVConfigBas. See this class for further
    information.
*/
class AVLIB_EXPORT AVLicense : public AVConfigBase
{
// Con(De)structors
public:
    //! Standard constructor
    AVLicense();

// Methods
public:
    //! validates a feature of the license
    /*! \param feature The feature to validate. If feature is empty, all
                       features found are checked.
        \return true if the license is valid.
    */
    bool checkLicense(const QString& feature = QString());

    //! This function takes a feature name, an expiry date and the sid and
    //! computes the MD5 Sum of it.
    /*! \param feature Name of the feature
        \param expiry Expiry date in the format YYYYMMDD for the feature
        \param sid System ID of the computer
        \return The computed MD5 sum
    */
    QString makeLicense(const QString& feature, const QString& expiry,
                        const QString& sid);

    //! This function creates a new license entry
    /*  \param feature Name of the feature
        \param expiry Expiry date in the format YYYYMMDD for the feature
        \param sid System ID of the computer on which the program should run
    */
    void createLicenseEntry(const QString& feature, const QString& expiry,
                            const QString& sid);

// Variables
public:
    static const QString m_licenseFileName; //!< Name of the license file
    //!< String that holds the System ID of the Computer, this program is running
    QString m_sid;

private:
    //! Dummy copy constructor
    AVLicense(const AVLicense&);

    //! Dummy assignment operator
    AVLicense& operator=(const AVLicense&);
};

#endif

// End of file
