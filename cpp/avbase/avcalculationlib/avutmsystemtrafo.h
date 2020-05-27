///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  DI Alexander Randeu, a.randeu@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief
*/

#ifndef __AVUTMSYSTEMTRAFO_H__
#define __AVUTMSYSTEMTRAFO_H__

// AVLib includes
#include "avcalculationlib_export.h"
#include "avcoordinatetransformbase.h"
#include "avutmtrafo.h"
#include "avearthdatum.h"

///////////////////////////////////////////////////////////////////////////////
//! Implement UTM transformation x/y (based on different Ellispoids) to
//! lat/long (based on WGS84)
/*!
    x/y are in meter, and are defined with geodetic directions (y=east,x=north).
    (Yeah, confuse the others that geodesist can make more money;-)
*/

class AVCALCULATIONLIB_EXPORT AVUtmSystemTrafo : public AVCoordinateTransformBase
{
public:
    // TODO: make common list with AVGaussKruegerSystemTrafo
    enum ReferenceMeridian {
        RM_BESSEL_M28 = 0,     //!< reference Meridian M28 Ferro/Bessel (10deg20min east)
        RM_BESSEL_M31 = 1,     //!< reference Meridian M31 Ferro/Bessel  (13deg20min east)
        RM_BESSEL_M34 = 2,     //!< reference Meridian M34 Ferro/Bessel  (16deg20min east)
        RM_ETRS89_M6  = 3,     //!< reference Meridian M6  Greenwich/ETRS89 (6deg east)
        RM_ETRS89_M9  = 4,     //!< reference Meridian M9  Greenwich/ETRS89 (9deg east)
        RM_ETRS89_M12 = 5,     //!< reference Meridian M12 Greenwich/ETRS89 (12deg east)
        RM_ETRS89_M15 = 6,     //!< reference Meridian M15 Greenwich/ETRS89 (15deg east)
        RM_ETRS89_M12NUERNBERG = 7, //!< reference Meridian M12 Greenwich/ETRS89 (12deg east)
                                    //!< with offset in x=-4000000m and y=-5000000m
        RM_BESSEL_M12NUERNBERG = 8, //!< reference Meridian M12 Greenwich/Bessel (12deg east)
                                    //!< with offset in x=-4000000m and y=-5000000m
        RM_KRASSOVSKI1940_M24_TALLINN = 9 //!< reference Meridian M24 Greenwich/
                                          //! PULKOVO-42/KRASSOVSKI 1940 TALLINN (24deg east)
                                          //!< with offset in x=24000m and y=-6536000m
    };

    //! constructor
    /*! \param ref_mer refernece meridian \sa ReferenceMeridian
        \param ref_ell reference ellispoid. It must be not empty (!ref_ell.isEmpty())
     */
    explicit AVUtmSystemTrafo(ReferenceMeridian ref_mer);

    //! destructor
    ~AVUtmSystemTrafo() override;

    //! convert x/y to lat/long in degrees
    /*! Calculates latitude and longitude (both deg,) from y (north) and x (east)
     */
    bool xy2latlong(double x, double y, double &la, double &lo) const override;

    //! lat/long in degrees to x/y
    /*! Calculates y (north) and x (east) from latitude and logitude
     */
    bool latlong2xy(double la, double lo, double &x, double &y) const override;

    //! return x/y to m resolution scaling factor (meters per pixel)
    /*! res is set to 1.0. But this is only an approximation, because depending
        on the position of 2 points the distance cannot be calculated directly
        from x/y of the 2 points. But for most applications the distance calculated
        from x/y of the 2 points should be accurate enough.
     */
    bool getResFactor(double &res) const override
    {
        res = 1.0;
        return true;
    }

protected:
    AVUtmTrafo          *m_utm_trafo;    //!< UTM Trafo
    AVEarthDatum        *m_earth_datum;  //!< earth datum
    ReferenceMeridian    m_ref_meridian; //!< reference meridian ID

private:
    //! Hidden copy-constructor
    AVUtmSystemTrafo(const AVUtmSystemTrafo&);
    //! Hidden assignment operator
    const AVUtmSystemTrafo& operator = (const AVUtmSystemTrafo&);

};

#endif /* __AVUTMSYSTEMTRAFO_H__ */

// End of file
