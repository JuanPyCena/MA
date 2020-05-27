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
    \brief   Declaration of a meteosat coordinate system transformation class
*/

#if !defined(AVMETEOSATTRAFO_H_INCLUDED)
#define AVMETEOSATTRAFO_H_INCLUDED

// AVLib includes
#include "avcalculationlib_export.h"
#include "avmisc.h"
#include "avcoordinatetransformbase.h"

// Local includes
#include <cmath>

///////////////////////////////////////////////////////////////////////////////
//! Implement system transformation from lat/long (WGS84) to a meteosat coordinate
//! system
class AVCALCULATIONLIB_EXPORT AVMeteoSatTrafo : public AVCoordinateTransformBase {
public:
    //! creates a AVMeteoSatTrafo object
    AVMeteoSatTrafo();

    //! destroys the AVMeteoSatTrafo object
    ~AVMeteoSatTrafo() override{};

    //! convert x/y coordinates to lat/long (in degrees)
    //! based on WGS84
    bool xy2latlong(double x, double y, double &la, double &lo) const override;

    //! convert lat/long in degrees to x/y
    bool latlong2xy(double la, double lo, double &x, double &y) const override;

    //! Not implemented yet
    bool getResFactor(double &res) const override;

protected:

    //Parameters used in calculation
    static const double V0;
    static const double V1;
    static const double V2;
    static const double V3;
    static const double V4;
    static const double V5;
    static const double V6;
    static const double V7;

    static const double LEVEL;     //!< accuracy level for iteration
    static const double ITLIM;     //!< max. number of iterations

    static const double R;         //!< earth radius in meters
    static double H;

    //! convert lat/long in radians to x/y
    bool latlong2xy_rad(double la, double lo, double &x, double &y) const;

    //! convert x/y system coordinates in meters to lat/long in radians
    bool xy2latlong_rad(double x, double y, double &la, double &lo) const;

    //!
    static double acotan (double x)
    {
        double pi = 4.0 * atan(1.0);
        if (x < 0.0) return -(pi / 2.0 - atan(-x));
        else         return  (pi / 2.0 - atan( x));
    }

    //!
    static double laphi2xm (double la, double phi)
    {
        double x_1, y_1, r, phi1, a, xm;

        x_1 = R * cos (phi) * sin (la);
        y_1 = R * sin (phi);

        r = sqrt (x_1 * x_1 + y_1 * y_1);
        phi1 = asin (r / R);
        a = R - R * cos (phi1);

        xm = x_1 * (H + R) / (H + a);
        return xm;
    }

/* ------------------------------------------------------------------------ */

    //!
    static double laphi2ym (double la, double phi)
    {
        double x_1, y_1, r, phi1, a, ym;

        x_1 = R * cos (phi) * sin (la);
        y_1 = R * sin (phi);

        r = sqrt (x_1 * x_1 + y_1 * y_1);
        phi1 = asin (r / R);
        a = R - R * cos (phi1);

        ym = y_1 * (H + R) / (H + a);
        return ym;
    }

    //!
    static double xmym2la (double xm, double ym)
    {
        double x_1, y_1, dxp, dyp, la, phi;
        int n;

        xmym_to_laphi_aprox (xm, ym, &la, &phi);
        n = 0;

        do {
            n++;
            x_1 = laphi2xm (la, phi);
            y_1 = laphi2ym (la, phi);
            if (AVFLOATEQ(xm, 0.0, 1e-20)) dxp = (x_1 - xm) / 10.0;  /* X-error */
            else                           dxp = (x_1 - xm) / xm;    /* X-error */
            if (AVFLOATEQ(ym, 0.0, 1e-20)) dyp = (y_1 - ym) / 10.0;  /* Y-error */
            else                           dyp = (y_1 - ym) / ym;    /* Y-error */
            if ((dxp < LEVEL && dyp < LEVEL) || n > ITLIM) {
            if (n > ITLIM) {
                LOGGER_ROOT.Write(LOG_ERROR,
                                  "max. # of iterations exceeded in xmym2la");
            }
            return la;
            }
            phi = phi * (1.0 - dyp / 10.0);
            la = la * (1.0 - dxp / 10.0);
        } while (true);
        return 0;
    }

/* ------------------------------------------------------------------------ */

    //!
    static double xmym2phi (double xm, double ym)
    {
        double x_1, y_1, dxp, dyp, la, phi;
        int n;

        xmym_to_laphi_aprox (xm, ym, &la, &phi);
        n = 0;

        do {
            n++;
            x_1 = laphi2xm (la, phi);
            y_1 = laphi2ym (la, phi);
            if (AVFLOATEQ(xm, 0.0, 1e-20)) dxp = (x_1 - xm) / 10.0; /* X-error */
            else                           dxp = (x_1 - xm) / xm;   /* X-error */
            if (AVFLOATEQ(ym, 0.0, 1e-20)) dyp = (y_1 - ym) / 10.0; /* Y-error */
            else                           dyp = (y_1 - ym) / ym;   /* Y-error */
            if ((dxp < LEVEL && dyp < LEVEL) || n > ITLIM) {
                if (n > ITLIM) {
                    LOGGER_ROOT.Write(LOG_ERROR,
                                "max. # of iterations exceeded in xmym2phi");
                }
                return phi;
            }
            phi = phi * (1.0 - dyp / 10.0);
            la = la * (1.0 - dxp / 10.0);
        } while (true);
        return 0;
    }

    /* ------------------------------------------------------------------------ */
    /* This function aproximates the result of XMYM_TO_LAPHI for iteration */
    /* purposes. */
    static void xmym_to_laphi_aprox (double xm, double ym, double *la, double *phi)
    {
        *phi = asin (ym / R);
        *la = asin (xm / (R * cos (*phi)));
    }
};

#endif

// End of file
