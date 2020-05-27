///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2010
//
// Module:     AVCALCULATIONLIB - AviBit Calculation Library
//
/////////////////////////////////////////////////////////////////////////////

/*! \file
    \author    Alexander Randeu, a.randeu@avibit.com
    \author    QT4-PORT: Alexander Randeu, a.randeu@avibit.com
    \brief     Defines scientific constants and transformation formulas
*/

#if !defined(AVSCICONST_H_INCLUDED)
#define AVSCICONST_H_INCLUDED

// Qt includes
#include <qstring.h>

// local includes
#include "avcalculationlib_export.h"

// forward delaractions

///////////////////////////////////////////////////////////////////////////////

//! Defines scientific constants and transformation formulas
/*! Defines scientific constants and transformation formulas. Conversion
    factors given with an # at the end of the number (in the comment) are
    yield to be exact ones.
 */
class AVCALCULATIONLIB_EXPORT AVSciConst
{
public:
    static const double pi;         //!< pi = 3.141592654...
    static const double T0ISA;      //!< ISA temperature at sea level = 288.15 [K]
    static const double Rho0ISA;    //!< ISA air density at sea level = 1.225 [kg/m^3]
    static const double P0ISA;      //!< ISA pressure at sea level = 101325 [Pa]
    static const double Ttrop;      //!< temp. above tropopause = 216.65 [K]
    static const double Rair;       //!< gas const. for air = 287.04 [m^2/Ks^2]
    static const double g;          //!< gravitational acceleration = 9.81 [m/s^2]
    static const double kT;         //!< ISA temp. gradient with alt. below
                                    //!< tropopause = -0.0065 [K/m]
    static const double atrop;      //!< speed of sound above tropopause = 295.07 [m/s]
    static const double a0ISA;      //!< ISA speed of sound at sea level = 340.29 [m/s]
    static const double gamma;      //!< isentropic expansion coef. for air = 1.4 []
    static const double HtropISA;   //!< alt. of tropopause for ISA cond. = 11000 [m]

    static const double ftTom;      //!< 1 feet = 0.3048# m
    static const double mToft;      //!< 1 m    = 3.281 feet
    static const double ktTomps;    //!< 1 knot = 0.5144444 m/s
    static const double mpsTokt;    //!< 1 m/s  = 1.9438446604 knots
    static const double nmTokm;     //!< 1 nm   = 1.852 km
    static const double kmTonm;     //!< 1 km   = 0.53995680346 nm

    static const double Re;         //!< mean earth radius 6378000 [m]
    static const double c;          //!< speed of light = 299792458 [m/s]

    //! convert <deg.> to <rad>
    static double convdegTorad(double deg) {return deg/180.*pi; };
    //! convert <rad> to <deg,>
    static double convradTodeg(double rad) {return rad/pi*180.; };

    //! convert degrees (counterclockwise, 0deg = east) to heading (clockwise, 0deg = north)
    static double convdegToheading(double deg) {double h=90.0-deg; if (h<0.0) h+=360.0; return h;};
    //! convert heading (clockwise, 0deg = north) to degrees (counterclockwise, 0deg = east)
    static double convheadingTodeg(double h) {double d=90.0-h; if (d<0.0) d+=360.0; return d;};

    //! convert <kt> to <m/s>
    static double convktTomps(double kt) { return ktTomps*kt; };
    //! convert <m/s> to <kt>
    static double convmpsTokt(double mps) { return mpsTokt*mps; };
    //! convert <m> to <ft>
    static double convmToft(double m) {return mToft*m; };
    //! convert <ft> to <m>
    static double convftTom(double ft) {return ftTom*ft; };
    //! convert <FL = ft/100> to <m>
    static double convFLTom(double fl) {return ftTom*fl*100.0; };
    //! convert <m> to <FL = ft/100>
    static double convmToFL(double m) {return mToft*m/100.0; };
    //! convert <nm> to <km>
    static double convnmTokm(double nm) { return nmTokm*nm; }
    //! convert <nm> to <m>
    static double convnmTom(double nm) { return nmTokm*nm*1000.0; }
    //! convert <m> to <nm>
    static double convmTonm(double m) { return kmTonm*m/1000.0; }
    //! convert <km> to <nm>
    static double convkmTonm(double km) { return kmTonm*km; }
    //! convert <deg,> to <deg mm ss,>
    /*! \param deg degree in <deg,>
        \param signbit The sign of the angle stored separately from the DMS value. \c true if the
               angle is negative.
        \param d result degree (0.0 <= d < 360.0)
        \param m result minutes (0 <= m < 60)
        \param s result seconds (0.0 <= s < 60.0)
        \note remember the problem of converting the seconds s to a string
              with precision of 2 digits after the "," for s near to 60.0.
              e.g. s=59.9999 with a precision of 2 gives s=60.00. You can use
              rounddms to automatically increase the minutes m by one
              automatically.
     */
    static void convdegTodms(double deg, bool& signbit, int &d, int &m, double &s);
    //! convert <deg mm ss,> to <deg,>
    /*! \param deg result degree in <deg,>
        \param signbit the sign of the resulting angle, \c true for a negative angle. The sign needs to be specified
               separately from the DMS value to account for negative angles between -0.0 and -1.0
               (\c d is zero in this case).
        \param d degree  (0 <= deg < 360.0)
        \param m minutes (0 <= m < 60)
        \param s seconds (0.0 <= s < 60.0)
     */
    static void convdmsTodeg(double &deg, bool signbit, int d, int m, double s);
    //! round s to the given precision and corrects d and m
    /*! e.g. s=59.9999 with a precicion of 2 gives s=60.00. You can use
        rounddms to automatically increase the minutes m by one and if
        necessary also d automatically.
        \param signbit The sign of the overall angle. \c true if the
               angle is negative. The sign may change on a leap from - 359 59 59.* to + 0 0 0.
        \param d degree <deg> (0 <= d < 360)
        \param m minutes (0 <= m < 60)
        \param s seconds (0.0 <= s < 60.0)
        \param pr precission to use (digits after ',' for s)
     */
    static void rounddms(bool& signbit, int &d, int &m, double &s, int pr);

    //! converts <deg,> to a dms string with given precision for s
    /*! converts deg to a string of format "deg mm ss,xx"
        \param deg degree <deg,>
        \param pr precision for s = digits after ','
     */
    static QString convdegTodmsString(double deg, int pr);

    //! converts a string dms of format "deg mm ss,xx" to <deg,>
    /*! \return <deg,> if successful, 0.0 if not
     */
    static double convdmsStringTodeg(const QString &dms);

    //! converts a string dms in AIXM format to <deg,>
    /*! For exact specification see AIXM 4.5 Schema definition for geoLatBase and geoLonBase

        Examples for valid string values are:
        471300N       => 47deg 13min 00sec North
        471345.1234N  => with decimal seconds part
        4713N         => no seconds given
        4713.123456N  => with decimal minutes part
        13S           => 13deg South
        13.12345678S  => with decimal degrees part
        0210509.1234E => 21deg 05min 09.1234sec East (longitude always has 3 digits for degrees)
        135W          => 135deg West
        900000N       => 90 degrees North
        1800000E      => 180 degrees East

        Examples for invalid string values are:
        471300        => none of N/S/W/E given
        900000.0001N  => latitude > 90deg
        1800000.0001E => longitude > 180deg
        476000N       => minutes > 59
        475960N       => seconds > 59
        475959.12345N => more than 4 second decimals
        47.123456789N => more than 8 degree decimals

        \return <deg,> if successful, 0.0 if not
     */
    static double convdmsAIXMStringTodeg(const QString &dms);

    //! normalizes an angle to (0.0 <= angle < 360.0) degree
    static double normaliseDeg(double angle);

public:
    //! destroys the object
    ~AVSciConst();
private:
    //! hides the constructor
    AVSciConst();
};

#endif
// End of file
