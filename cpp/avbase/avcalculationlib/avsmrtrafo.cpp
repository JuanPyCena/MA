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
    \brief   Implements class for SMR coordinate transformations
*/


#include "avmisc.h"
#include "avsmrtrafo.h"
#include "avlog.h"


///////////////////////////////////////////////////////////////////////////////
//                        STATIC VARIABLE DEFINITIONS
///////////////////////////////////////////////////////////////////////////////

//! SMR LOWW Position is 48 6 2,1649N, 16 34 40,4612E (WGS84)
//! actual SMR geographical position (WGS84), Lat
const double AVSMRTrafo::RLAT  = 48.10060136111111 * AV_PI / 180.0;
//! actual SMR geographical position (WGS84), Long
const double AVSMRTrafo::RLONG = 16.57790588888888 * AV_PI / 180.0;

//! SMR Schwechat, XOFFSET
const double AVSMRTrafo::XOFF  = 700.0;       // smr reference point x
//! SMR Schwechat, YOFFSET
const double AVSMRTrafo::YOFF  = 400.0;       // smr reference point y
//! SMR Schwechat, RESOLUTION in Meters per Pixel
const double AVSMRTrafo::RES   = 4.5;
//! SMR Schwechat, SCALING factor
const double AVSMRTrafo::SCALE = 1.5;

//! SMR Schwechat, NORTHING. This is the angle of LOWW RWY11 which is to be
//! shown horizontally in the SMR image. The angle is taken from the AIP
//! and is the heading of the runway (measured from the vertical = north
//! downwards).
const double AVSMRTrafo::NORTHING = 115.918;

//! SMR Schwechat, NORTHING in radians
const double AVSMRTrafo::NORTHING_RAD = NORTHING * AV_PI / 180.0;

//! SMR Schwechat, NORTHING_MATH, mathematical rotation angle in radians.
/*! The mathematical rotation angle NORTHING_RAD is calculated from the
    heading as specified below because the northing angle is specified as
    the heading of the runway (measured clockwise from the north direction).
    So we need to convert that to the mathematical angle (measured from the
    horizontal axis, counterclockwise), hence the 2 * AV_PI minus X value.
*/
const double AVSMRTrafo::NORTHING_MATH = 2.0*AV_PI - AV_PI/2.0 - NORTHING_RAD;

//! epsilon for double compare
static const double DOUBLE_EPS = 1.0e-14;

///////////////////////////////////////////////////////////////////////////////

AVSMRTrafo::AVSMRTrafo() : AVCoordinateTransformBase("SMRTrafo")
{
    // WGS84 ellipsoid
    m_a = 6378137.0;
    m_b = 6356752.31425;

    // reference point = radar location
    m_lat0 = RLAT;
    m_lon0 = normAng(RLONG, -AV_PI);

    // beta0
    if(fabs(cos(m_lat0)) - DOUBLE_EPS > 0.0)
        m_beta0 = atan(m_a / m_b * tan(m_lat0));
    else
        m_beta0 = m_lat0;

    m_inva = 1.0 / m_a;
    m_sb0  = sin(m_beta0);
    m_cb0  = cos(m_beta0);
    m_e2   = 1.0 - pow(m_b / m_a, 2.0);
    m_s    = sqrt(pow((m_b / m_a) * m_cb0, 2.0) + pow(m_sb0, 2.0));
}

///////////////////////////////////////////////////////////////////////////////

bool AVSMRTrafo::WGS84ToGnomonic(double lat, double lon, double &x, double &y) const
{
    double lat1, lon1, sl, cl, k, ainvk, beta, sb ,cb;

    // input point
    lat1 = lat;
    lon1 = normAng(lon, -AV_PI);

    // beta
    if(fabs(cos(lat1)) - DOUBLE_EPS > 0.0)
        beta = atan((m_a / m_b) * tan(lat1));
    else
        beta = lat1;

    sb = sin(beta);
    cb = cos(beta);
    sl = sin(lon1 - m_lon0);
    cl = cos(lon1 - m_lon0);
    k  = (m_sb0 * sb + m_cb0 * cb * cl);
    if (k <= 0.)
    {
        AVLogger->Write(LOG_WARNING, "AVSMRTrafo:WGS84ToGnomonic: "
                        "k<=0.0! Transformation not possible (lat=%.5f,lon=%.5f)",
                        lat, lon);
        return false;          // something's wrong
    }

    ainvk = m_a/k;
    x = ainvk * cb * sl;
    y = m_s * ainvk * (m_cb0 * sb - m_sb0 * cb *cl);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSMRTrafo::getResFactor(double &res) const
{
    res = RES;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSMRTrafo::latlong2xy(double la, double lo, double &x, double &y) const
{
    // perform gnomonic projection to x/y in meters based on the radar center
    double xr, yr;
    if (!WGS84ToGnomonic(deg2rad(la), deg2rad(lo), xr, yr)) {
        xr = yr = 0.0;
        return false;
    }

    // we need the y coordinate mirroreded
    yr = -yr;

    // rotate coordinates for northing (radar image is rotated)
    double r = sqrt(xr * xr + yr * yr);
    double a = atan2(yr, xr);
    a += NORTHING_MATH;
    xr = r * cos(a);
    yr = r * sin(a);

    // scale from meters to radar pixels
    xr /= RES;
    yr /= RES;

    // apply the offset
    x = (XOFF / SCALE) + xr;
    y = (YOFF / SCALE) + yr;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSMRTrafo::GnomonicToWGS84(double xv, double yv, double &lat, double &lon) const
{
    double x = xv/m_a;
    double y = yv/(m_s*m_a);

    // now the real calculation
    double dist = sqrt(x*x + y*y);

    if (fabs(dist) < DOUBLE_EPS) {
        lat = m_lat0;
        lon = m_lon0;
        return true;
    }

    double sinu = sin(atan(dist));
    double cosu = sqrt(1.0 - sinu*sinu);

    double c = cosu*m_sb0 + (y*sinu*m_cb0)/dist;
    double beta;
    if (fabs(c) >= 1.0) {
        if (c > 0) beta = AV_PI/2.0;
        else beta = -AV_PI/2.0;
    } else {
        beta = asin(c);
    }

    // lon
    double c1 = (cosu - m_sb0*sin(beta))*dist;
    lon = m_lon0 + atan2(x*sinu*m_cb0, c1);
    lon = normAng(lon, -AV_PI);

    // lat
    if(fabs(cos(beta)) > DOUBLE_EPS) {
        lat = atan((m_b/m_a)*tan(beta));
    } else {
        lat = beta;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSMRTrafo::xy2latlong(double xv, double yv, double& lat, double& lon) const
{
    // remove the offset
    double x = xv - (XOFF / SCALE);
    double y = yv - (YOFF / SCALE);

    // scale from radar pixels to meters
    x *= RES;
    y *= RES;

    // un-rotate coordinates for northing (radar image is rotated)
    double r = sqrt(x * x + y * y);
    double a = atan2(y, x);
    a -= NORTHING_MATH;
    x = r * cos(a);
    y = r * sin(a);

    // we need the y coordinate mirroreded
    y = -y;

    // perform inverse gnomonic projection from x/y in meters based on the
    // radar center to lat/lon in radians
    if (!GnomonicToWGS84(x, y, lat, lon)) {
        lat = lon = 0.0;
        return false;
    }

    lat = rad2deg(lat);
    lon = rad2deg(lon);

    return true;
}
// End of file
