///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// QT-Version: QT5
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Thomas Leitner, t.leitner@avibit.com
    \author  QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief   SMR coordinate transformation class
*/

#if !defined(AVSMRTRAFO_H_INCLUDED)
#define AVSMRTRAFO_H_INCLUDED

// system includes
#include <cmath>

// AVLib includes
#include "avcoordinatetransformbase.h"
#include "avmisc.h"
#include "avcalculationlib_export.h"

///////////////////////////////////////////////////////////////////////////////

//! !! DEPRECATED, don't use in new code !!
//! Introduced during KDAB porting of the fdp2.
//!
//! AVSMRTrafo implements coordinate transformation from
//! lat/long-coordinates to x/y-coordinates for the SMR display.

class AVCALCULATIONLIB_EXPORT AVSMRTrafo : public AVCoordinateTransformBase {
public:
    //! creates a AVSMRTrafo object
    AVSMRTrafo();

    //! destructs the AVSMRTrafo object
    ~AVSMRTrafo() override {}

    //! transform lat/long-coordinates to x/y-coordinates for Astos
    bool latlong2xy(double la, double lo, double &x, double &y) const override;

    //! transform x/y-coordinates <NOT m> to lat/long-coordinates <deg,>
    bool xy2latlong(double x, double y, double &lat, double &lon) const override;

    //! return x/y to m resolution scaling factor (meters per pixel)
    bool getResFactor(double &res) const override;

    //! conversion parameters
    static const double XOFF; //static variables must be declared on separate lines to
    static const double YOFF; //avoid a bug with MS compilers in dllexported classes.
    static const double RES;  //see https://support.microsoft.com/en-us/kb/127900

    static const double SCALE;
    static const double NORTHING;
    static const double NORTHING_RAD;
    static const double NORTHING_MATH;

    //! radar location
    static const double RLAT;
    static const double RLONG;

private:
    double m_a;     //!< ellipsoid major axis <m>
    double m_b;     //!< ellipsoid semi-major axis <m>
    double m_e2;    //!< excentricity squared
    double m_inva;  //!< 1./m_a
    double m_lat0;  //!< reference latitude <rad>
    double m_lon0;  //!< reference longitude <rad>
    double m_beta0; //!< reduced latitude <rad>
    double m_sb0;   //!< sin(m_beta0)
    double m_cb0;   //!< cos(m_beta0)
    double m_s;     //!< needed for calcs

    //! normalize angle from min to 2 * pi + min
    inline double normAng(double a, double min) const {
        double c = fmod(a - min, 2.0 * AV_PI);
        if (c < 0.0) c += 2.0 * AV_PI;
        return c + min;
    }

    //! WGS84 to GNOMONIC projection, returns x/y with 0,0 at the radar center
    bool WGS84ToGnomonic(double lat, double lon, double &x, double &y) const;

    //! GNOMONIC to WGS84 projection, returns the radar center for (x,y)=(0,0)
    bool GnomonicToWGS84(double x, double y, double &lat, double &lon) const;
};

#endif

// End of file
