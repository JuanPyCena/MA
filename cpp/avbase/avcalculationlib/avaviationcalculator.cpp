///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVCALCULATIONLIB - AviBit Calculation Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Alexander Randeu, a.randeu@avibit.com (Clemens Ender, c.ender@avibit.com)
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Defines a helper class for great circle computation

    This class provides great circle computations in radians and degree.
*/


// Qt includes
#include <QPoint>
#include <QPointF>

// AVLib includes
#include "avlog.h"
#include "avplanarcalculator.h"

// local includes
#include "avaviationcalculator.h"


///////////////////////////////////////////////////////////////////////////////

const double AVAviationCalculator::C_PI = 3.1415926535897932385E0; //!< PI
const double AVAviationCalculator::C_RE = 6370949.0; //!< mean equatorial
                                            //!< radius
                                            //!< of the earth in meters

const double AVAviationCalculator::C_DEG2RAD = C_PI / 180.0;
const double AVAviationCalculator::C_RAD2DEG = 180.0 / C_PI;

const double AVAviationCalculator::C_DOUBLE_EPS = 1.0e-20;

const double AVAviationCalculator::C_INVALID_ALT = -10000.0;

///////////////////////////////////////////////////////////////////////////////

AVAviationCalculator::AVAviationCalculator()
{
}

///////////////////////////////////////////////////////////////////////////////

AVAviationCalculator::~AVAviationCalculator()
{
}

///////////////////////////////////////////////////////////////////////////////

double AVAviationCalculator::distance (double p1_lat,
                                       double p1_lon,
                                       double p2_lat,
                                       double p2_lon,
                                       double mean_h)
{
    double lat1, lon1, lat2, lon2, mh;
    lat1 = p1_lat*C_DEG2RAD;
    lon1 = p1_lon*C_DEG2RAD;
    lat2 = p2_lat*C_DEG2RAD;
    lon2 = p2_lon*C_DEG2RAD;
    mh = mean_h;
    return distanceRad(lat1, lon1, lat2, lon2, mh);
};

///////////////////////////////////////////////////////////////////////////////

double AVAviationCalculator::distanceRad (double p1_lat,
                                          double p1_lon,
                                          double p2_lat,
                                          double p2_lon,
                                          double mean_h)
{
    double cc1 = cos(p1_lat)*cos(p2_lat);
    double cc2 = (p2_lon - p1_lon)/2.0;
    double scc2 = sin(cc2);
    double ccc2 = cos(cc2);
    double sd1 = sin((p2_lat - p1_lat)/2.0);
    double sd2 = sin((p1_lat + p2_lat)/2.0);
    double c1 = sqrt(sd1*sd1 + cc1*scc2*scc2);
    double c2 = sqrt(sd2*sd2 + cc1*ccc2*ccc2);
    return 2.0*atan2(c1, c2) * (C_RE + mean_h);
}

///////////////////////////////////////////////////////////////////////////////

bool AVAviationCalculator::pointAtDist (double p1_lat,
                                        double p1_lon,
                                        double p2_lat,
                                        double p2_lon,
                                        double dist,
                                        double& d_lat,
                                        double& d_lon,
                                        double mean_h)
{
    double d = distance (p1_lat, p1_lon, p2_lat, p2_lon, mean_h);
    if (fabs(dist) > d || d <= 0.0)
    {
        if (fabs(dist)>d) AVLogger->Write(LOG_DEBUG, "AVAviationCalculator:pointAtDist: dist>d");
        if (d<=0.0) AVLogger->Write(LOG_DEBUG, "AVAviationCalculator:pointAtDist: dist<=0");
        return false;
    }
    double f = dist / d;
    d        = d / (C_RE + mean_h);     // need distance in radians
    double A = sin((1 - f) * d) / sin(d);
    double B = sin(f * d) / sin(d);
    double x = A * cos(p1_lat*C_DEG2RAD) * cos(p1_lon*C_DEG2RAD) +
               B * cos(p2_lat*C_DEG2RAD) * cos(p2_lon*C_DEG2RAD);
    double y = A * cos(p1_lat*C_DEG2RAD) * sin(p1_lon*C_DEG2RAD) +
                   B * cos(p2_lat*C_DEG2RAD) * sin(p2_lon*C_DEG2RAD);
    double z = A * sin(p1_lat*C_DEG2RAD) + B * sin(p2_lat*C_DEG2RAD);
    d_lon  = atan2(y, x) * C_RAD2DEG;
    d_lat  = atan2(z, sqrt(x * x + y * y)) * C_RAD2DEG;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVAviationCalculator::pointAtDistRad (double p1_lat,
                                           double p1_lon,
                                           double p2_lat,
                                           double p2_lon,
                                           double dist,
                                           double& d_lat, double& d_lon,
                                           double mean_h)
{
    double d = distanceRad (p1_lat, p1_lon, p2_lat, p2_lon, mean_h);
    if (fabs(dist) > d || d <= 0.0) return false;
    double f = dist / d;
    d        = d / (C_RE + mean_h);     // need distance in radians
    double A = sin((1 - f) * d) / sin(d);
    double B = sin(f * d) / sin(d);
    double x = A * cos(p1_lat) * cos(p1_lon) +
               B * cos(p2_lat) * cos(p2_lon);
    double y = A * cos(p1_lat) * sin(p1_lon) +
               B * cos(p2_lat) * sin(p2_lon);
    double z = A * sin(p1_lat) + B * sin(p2_lat);
    d_lon  = atan2(y, x);
    d_lat  = atan2(z, sqrt(x * x + y * y));
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVAviationCalculator::intersection2SegmentsSpherical(double p1_lat, double p1_lon,
                                                          double p2_lat, double p2_lon,
                                                          double p3_lat, double p3_lon,
                                                          double p4_lat, double p4_lon,
                                                          double &p_int_lat, double &p_int_lon)
{
    // first the simple checks
    if (isEqual(p1_lat, p1_lon, p3_lat, p3_lon)) {
        p_int_lat = p1_lat;
        p_int_lon = p1_lon;
        return true;
    }
    if (isEqual(p1_lat, p1_lon, p4_lat, p4_lon)) {
        p_int_lat = p1_lat;
        p_int_lon = p1_lon;
        return true;
    }
    if (isEqual(p2_lat, p2_lon, p3_lat, p3_lon)) {
        p_int_lat = p2_lat;
        p_int_lon = p2_lon;
        return true;
    }
    if (isEqual(p2_lat, p2_lon, p4_lat, p4_lon))  {
        p_int_lat = p2_lat;
        p_int_lon = p2_lon;
        return true;
    }
    bool eq_12 = isEqual(p1_lat, p1_lon, p2_lat, p2_lon);
    bool eq_34 = isEqual(p3_lat, p3_lon, p4_lat, p4_lon);
    if (eq_12 && eq_34) return false; // because not the same points, checked above

    // now the work
    double x0, y0, z0;
    double x1, y1, z1;
    double x2, y2, z2;
    double x3, y3, z3;
    convertLatLonToXYZ(p1_lat, p1_lon, x0, y0, z0);
    convertLatLonToXYZ(p2_lat, p2_lon, x1, y1, z1);
    convertLatLonToXYZ(p3_lat, p3_lon, x2, y2, z2);
    convertLatLonToXYZ(p4_lat, p4_lon, x3, y3, z3);

    // calculate the normal vectors for planes x0/x1/origin -> [a,b,c] and
    // x2/x3/origin -> [d,e,f]
    double a =   y0*z1 - y1*z0;
    double b = -(x0*z1 - x1*z0);
    double c =   x0*y1 - x1*y0;
    double d =   y2*z3 - y3*z2;
    double e = -(x2*z3 - x3*z2);
    double f =   x2*y3 - x3*y2;

    double h1 = (e*a - d*b);
    AVASSERT(fabs(h1) > C_DOUBLE_EPS);
    double h = (d*c - f*a)/h1;

    AVASSERT(fabs(a) > C_DOUBLE_EPS);
    double g = (-b*h - c)/a;
    // note: R=1.0
    double k = sqrt(1.0/(g*g + h*h + 1.0));

    // intersections: (g*k,h*k,k) and (-g*k,-h*k,-k)
    p_int_lat = asin(k)*C_RAD2DEG;
    p_int_lon = atan2(h*k, g*k)*C_RAD2DEG;

    // now check, if this intersection point is valid (between p1/p2 and p3/p4)
    double c12i = courseDeviation (p1_lat, p1_lon, p2_lat, p2_lon, p_int_lat, p_int_lon);
    if (c12i <= 90.0) {
        double c21i = courseDeviation (p2_lat, p2_lon, p1_lat, p1_lon, p_int_lat, p_int_lon);
        if (c21i <= 90.0) {
            double c34i = courseDeviation (p3_lat, p3_lon, p4_lat, p4_lon, p_int_lat, p_int_lon);
            if (c34i <= 90.0) {
                double c43i = courseDeviation (p4_lat, p4_lon, p3_lat, p3_lon,
                                               p_int_lat, p_int_lon);
                if (c43i <= 90.0) {
                    return true;
                }
            }
        }
    }

    p_int_lat = asin(-k)*C_RAD2DEG;
    p_int_lon = atan2(-h*k, -g*k)*C_RAD2DEG;

    c12i = courseDeviation (p1_lat, p1_lon, p2_lat, p2_lon, p_int_lat, p_int_lon);
    if (c12i <= 90.0) {
        double c21i = courseDeviation (p2_lat, p2_lon, p1_lat, p1_lon, p_int_lat, p_int_lon);
        if (c21i <= 90.0) {
            double c34i = courseDeviation (p3_lat, p3_lon, p4_lat, p4_lon, p_int_lat, p_int_lon);
            if (c34i <= 90.0) {
                double c43i = courseDeviation (p4_lat, p4_lon, p3_lat, p3_lon,
                                               p_int_lat, p_int_lon);
                if (c43i <= 90.0) {
                    return true;
                }
            }
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool AVAviationCalculator::normalDist (double p1_lat,
                                       double p1_lon,
                                       double p2_lat,
                                       double p2_lon,
                                       double p3_lat,
                                       double p3_lon,
                                       double& ndist, double& p4_lat,
                                       double& p4_lon,
                                       double mean_h)
{
    if(distance(p1_lat, p1_lon, p2_lat, p2_lon, mean_h) < C_DOUBLE_EPS) {
        ndist = distance(p1_lat, p1_lon, p3_lat, p3_lon, mean_h);
        p4_lat = p1_lat;
        p4_lon = p1_lon;

        return true;
    }

    double myR = C_RE + mean_h;

    double c123 = courseDeviation (p1_lat,p1_lon,
                                   p2_lat,p2_lon,
                                   p3_lat,p3_lon);
    double c213 = courseDeviation (p2_lat,p2_lon,
                                   p1_lat,p1_lon,
                                   p3_lat,p3_lon);

    if ( (c123 > 90.0) || ( c213 > 90.0 ) ) {
        return false;
    }

    double s13 = distance (p1_lat,p1_lon,p3_lat,p3_lon, mean_h);

    ndist = myR * asin ( sin (s13 / myR) * sin (c123 * C_DEG2RAD) );
    double s14 = myR * atan ( tan (s13 / myR) * cos (c123 * C_DEG2RAD) );

    pointAtDist (p1_lat, p1_lon, p2_lat, p2_lon, s14, p4_lat, p4_lon,
                 mean_h);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVAviationCalculator::normalDistRad (double p1_lat,
                                          double p1_lon,
                                          double p2_lat,
                                          double p2_lon,
                                          double p3_lat,
                                          double p3_lon,
                                          double& ndist, double& p4_lat,
                                          double& p4_lon,
                                          double mean_h)
{
    if(distanceRad(p1_lat, p1_lon, p2_lat, p2_lon, mean_h) < C_DOUBLE_EPS) {
        ndist = distanceRad(p1_lat, p1_lon, p3_lat, p3_lon, mean_h);
        p4_lat = p1_lat;
        p4_lon = p1_lon;

        return true;
    }

    double myR = C_RE + mean_h;

    double c123 = courseDeviationRad (p1_lat,p1_lon,
                                      p2_lat,p2_lon,
                                      p3_lat,p3_lon);
    double c213 = courseDeviationRad (p2_lat,p2_lon,
                                      p1_lat,p1_lon,
                                      p3_lat,p3_lon);

    if ( (c123 > C_PI/2.0) || ( c213 > C_PI/2.0 ) ) return false;

    double s13 = distanceRad (p1_lat,p1_lon,p3_lat,p3_lon,mean_h);

    ndist = myR * asin ( sin (s13 / myR) * sin (c123) );
    double s14 = myR * atan ( tan (s13 / myR) * cos (c123) );

    pointAtDistRad (p1_lat, p1_lon, p2_lat, p2_lon, s14, p4_lat, p4_lon,
                    mean_h);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void AVAviationCalculator::calcTurnCenter(double lat1,
                                          double lon1,
                                          double cou,
                                          double lat2,
                                          double lon2,
                                          double turnradius,
                                          double &latc,
                                          double &lonc,
                                          double &inc_f)
{
    double c12 = course(lat1,lon1,lat2,lon2);
    double dd = adjustHeading(c12 - cou);
    if(dd > 180.0) inc_f = -1.0;
    else inc_f = 1.0;

    double cd = cou+inc_f*90;

    pointAtCourseDist(lat1,lon1,turnradius,cd,latc,lonc);
}

///////////////////////////////////////////////////////////////////////////////

void AVAviationCalculator::pointAtCourseDist(double lat1d,
                                             double lon1d,
                                             double dd,
                                             double tcd,
                                             double &lat2,
                                             double &lon2,
                                             double mean_h)
{
    double lat1, lon1, tc;

    lat1 = lat1d * C_DEG2RAD;
    lon1 = lon1d * C_DEG2RAD;
    tc   = tcd   * C_DEG2RAD;

    pointAtCourseDistRad(lat1, lon1, dd, tc, lat2, lon2, mean_h);

    lat2 *= C_RAD2DEG;
    lon2 *= C_RAD2DEG;
}

/////////////////////////////////////////////////////////////////////////////

void AVAviationCalculator::pointAtCourseDistRad(const double lat1, const double lon1,
                                                const double dd, const double tc,
                                                double &lat2, double &lon2,
                                                const double mean_h)
{
    double d(0.);
    double myR = C_RE + mean_h;

    d = dd / myR;

    lat2 = asin(sin(lat1)*cos(d)+cos(lat1)*sin(d)*cos(tc));

    if (fabs(cos(lat2)) < C_DOUBLE_EPS)
        lon2 = lon1;      // endpoint a pole
    else
        lon2 = aviationMod(lon1 + asin(sin(tc)*sin(d)/cos(lat2)) + C_PI
                           ,2*C_PI) - C_PI;

    //This algorithm is limited to distances such that dlon <pi/2,
    //i.e those that extend around less than one quarter of the
    //circumference of the earth in longitude. A completely general,
    //but more complicated algorithm is necessary if greater distances
    //are allowed:
    //lat2 = asin(sin(lat1)*cos(d)+cos(lat1)*sin(d)*cos(tc));
    //double dlon;
    //dlon = atan2(sin(tc)*sin(d)*cos(lat1),cos(d)-sin(lat1)*sin(lat2));
    //lon2 = aviationMod( lon1 - dlon + C_PI,2*C_PI )- C_PI;
}

/////////////////////////////////////////////////////////////////////////////

double AVAviationCalculator::getQNHCorrectedAltitudeFromFlightlevel(const double& flightlevel,
                                                                    const double& qnh)

{
    double corrected_alt = C_INVALID_ALT;

    if (qnh <= 0.0)
    {
        AVLogger->Write(LOG_WARNING,
                        "AVAviationCalculator:getQNHCorrectedAltitudeFromFlightlevel: "
                        "Given QNH of (%.02f) out of range - aborting", qnh);
        return corrected_alt;
    }

    // Hr = (1 - (Pr/1013.25)^0.1903)*10^6/6.8756
    double hr = ((1.0 - pow((qnh/1013.25), 0.1903)) * 1000000.0) / 6.8756;
    corrected_alt = (flightlevel * 100.0) - hr;

    return corrected_alt;
}

/////////////////////////////////////////////////////////////////////////////

double AVAviationCalculator::getFlightlevelFromQNHCorrectedAltitude(const double& altitude,
                                                                    const double& qnh)
{
    double flightlevel = C_INVALID_ALT;

    if (qnh <= 0.0)
    {
        AVLogger->Write(LOG_WARNING,
                        "AVAviationCalculator:getFlightlevelFromQNHCorrectedAltitude: "
                        "Given QNH of (%.02f) out of range - aborting", qnh);
        return flightlevel;
    }

    // Hr = (1 - (Pr/1013.25)^0.1903)*10^6/6.8756
    double hr = ((1.0 - pow((qnh/1013.25), 0.1903)) * 1000000.0) / 6.8756;
    flightlevel = (altitude + hr) / 100.0;

    return flightlevel;
}

/////////////////////////////////////////////////////////////////////////////

bool AVAviationCalculator::minDistMovingTargets(double t1x, double t1y, double vt1x, double vt1y,
                                                double t2x, double t2y, double vt2x, double vt2y,
                                                double dist_threshold,
                                                bool &has_solution, bool &const_dist,
                                                double &t1, double &t2)
{
    has_solution = false;
    const_dist   = false;
    t1 = 0.0;
    t2 = 0.0;

    double dvtx = vt1x - vt2x;
    double dvty = vt1y - vt2y;
    double A = dvtx*dvtx + dvty*dvty;

    // both AC have a speed vector of 0 -> constant distance between targets
    if (A < C_DOUBLE_EPS) {
        double dx = t1x - t2x;
        double dy = t1y - t2y;
        if (sqrt(dx*dx + dy*dy) <= dist_threshold)
        {
            has_solution = true;
            const_dist   = true;
        }
        return true;
    }

    double dtx = t1x - t2x;
    double dty = t1y - t2y;

    double B = 2.0*(dtx*dvtx + dty*dvty);
    double C = dtx*dtx + dty*dty - dist_threshold*dist_threshold;

    double cc1 = B*B - 4.0*A*C;

    // no real solution
    if (cc1 < 0) return true;

    cc1 = sqrt(cc1);
    double cc2 = 2.0*A;

    t1 = (-B - cc1)/cc2;
    t2 = (-B + cc1)/cc2;

    has_solution = true;
    return true;
}

// End of file
