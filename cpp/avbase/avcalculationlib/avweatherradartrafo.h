///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Andreas Schuller, a.schuller@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Declaration of a weather radar coordinate system transformation class
*/

#if !defined(AVWEATHERRADARTRAFO_H_INCLUDED)
#define AVWEATHERRADARTRAFO_H_INCLUDED

// AVLib includes
#include "avcalculationlib_export.h"
#include "avcoordinatetransformbase.h"

// Local includes
#include <cmath>

///////////////////////////////////////////////////////////////////////////////
//! Implements a transformation from lat/long (WGS84) to the weather radar
//! coordinate system and vice versa.
class AVCALCULATIONLIB_EXPORT AVWeatherRadarTrafo : public AVCoordinateTransformBase {
public:
    //! creates a AVWeatherRadarTrafo object
    /*! Uses given parameters for transformation
    */
    explicit AVWeatherRadarTrafo(double v0 = 8.194, double v1 = 50.437,  double v2 = 0.0,
                                 double v3 = 0.0,   double v4 = 1018.18, double v5 = 1018.18);

    //! destroys the AVWeatherRadarTrafo object
    ~AVWeatherRadarTrafo() override{};

    //! convert x/y coordinates to lat/long (in degrees)
    //! based on WGS84
    bool xy2latlong(double x, double y, double &la, double &lo) const override;

    //! convert lat/long in degrees to x/y
    bool latlong2xy(double la, double lo, double &x, double &y) const override;

    //! Not implemented yet
    bool getResFactor(double &res) const override;

    //! returns the unique trafo name
    /*! the trafo name is built as follows:
        trafo name + V0_V1_V2_V3_V4_V5,
        whereas the trafo name can be set using the setName() method.
    */
    QString uniqueName() const override;

protected:

    // Parameters used in calculation
    const double V0;
    const double V1;
    const double V2;
    const double V3;
    const double V4;
    const double V5;

    //! convert lat/long in radians to x/y
    bool latlong2xy_rad(double la, double lo, double &x, double &y) const;

    //! convert x/y system coordinates in meters to lat/long in radians
    bool xy2latlong_rad(double x, double y, double &la, double &lo) const;

    //! Calculates the arc co-tangesns from the given value
    static double acotan (double x)
    {
        double pi = 4.0 * atan(1.0);
        if (x < 0.0) return -(pi / 2.0 - atan(-x));
        else         return  (pi / 2.0 - atan( x));
    }
};

///////////////////////////////////////////////////////////////////////////////

#endif

// End of file
