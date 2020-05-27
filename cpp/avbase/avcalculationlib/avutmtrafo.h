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

#ifndef __AVUTMTRAFO_H__
#define __AVUTMTRAFO_H__

// AVLib includes
#include "avcalculationlib_export.h"
#include "avgausskruegertrafo.h"

///////////////////////////////////////////////////////////////////////////////
//! Implement Gauss-Kr�ger transformation x/y to lat/long (for given Ellipsoid)
/*!
    x/y are in meter, and are defined with geodetic directions (y=east,x=north).
    (Yeah, confuse the others that geodesist can make more money;-)

    \note This is only the gauss krueger trafo without datum transformation.
          For converting Austrian GK-Coordinates (related to BESSEL Ellipsoid) to
          WGS84 Coordinates use \saAVUtmSystemTrafo.
*/
class AVCALCULATIONLIB_EXPORT AVUtmTrafo : public AVGaussKruegerTrafo
{
public:
    //! constructor
    /*! \param ref_mer refernece meridian in deg,
        \param ref_ell reference ellispoid. It must be not empty (!ref_ell.isEmpty())
     */
    AVUtmTrafo(double ref_mer, const AVCoordEllipsoid& ref_ell);

    //! destructor
    ~AVUtmTrafo() override;

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
};

#endif /* __AVUTMTRAFO_H__ */

// End of file
