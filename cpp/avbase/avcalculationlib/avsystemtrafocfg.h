///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2011
//
// Module:    AVCALCULATIONLIB - AviBit Calculation Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Thomas Pock, t.pock@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Configuration of the origin for the AVSystemTrafo Class
*/

#if !defined(AVSYSTEMTRAFOCFG_H_INCLUDED)
#define AVSYSTEMTRAFOCFG_H_INCLUDED

// Qt includes
#include <QStringList>

// avcommon includes
#include "avcalculationlib_export.h"
#include "avconfig2saver.h"
#include "avconfig2classic.h"

//! This class is the configuration of the systemtrafo.
/*! It is derived from AVConfigBase. See this class for more information.
    If the configfile exists, the origin will be configured, otherwise it will
    be initialized with invalid values
 */
class AVCALCULATIONLIB_EXPORT AVSystemTrafoConfig : public AVConfig2HybridConfig
{

// Con(De)structors
public:
    //!Constructs the AVSystemTrafoConfig object
    explicit AVSystemTrafoConfig(AVConfig2Container& cfg_container, const QString& name = DEFAULT_ORIGIN_CONFIG_FILENAME);

// Variables
public:
    double m_origin_lat;     //!< Latitude value of the origin
    double m_origin_lon;     //!< Longitude value of the origin
    double m_origin_geod_h;  //!< geodetic height of the origin

    //! List of origin config filenames having been used so far (to avoid repetitive logging)
    static QStringList   ORIGIN_CONFIG_FILENAMES;
    //! Default configuration filename
    static const QString DEFAULT_ORIGIN_CONFIG_FILENAME;

private:
    //! Dummy copy constructor
    AVSystemTrafoConfig(const AVSystemTrafoConfig&);

    //! Dummy assignment operator
    AVSystemTrafoConfig& operator=(const AVSystemTrafoConfig&);
};


#endif


// End of file
