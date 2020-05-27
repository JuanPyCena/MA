//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2018
//
// Module:    AVLIB - AviBit Library
//
//-----------------------------------------------------------------------------

/*! \file
    \brief   AVVersionReader header
 */

#pragma once

// system/Qt includes
#include <QtGlobal>

// common includes

// local includes
#include "avlib_export.h"

// forward declarations
class QIODevice;

//-----------------------------------------------------------------------------
//! Class for reading APP_HOME/VERSION info
class AVLIB_EXPORT AVVersionReader
{
    //! friend declaration for function level test case
    friend class TstAVVersionReader;
    static const QString VERSION_FILENAME;
public:
    /*!
    * \brief readVersion reads the version info (== first line) from APP_HOME/VERSION
    * If an error occours (file does not exist/readable) or version info is empty AVPackageInfo::getVersion() is returned
    * \param fallback custom fallback version info. If null-QString is given AVPackageInfo::getVersion() is returned
    * \return version info from file or fallback.
    */
    static QString readVersion(const QString& fallback = QString());

private:
    Q_DISABLE_COPY(AVVersionReader)
};


// End of file
