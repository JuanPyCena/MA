///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVCALCULATIONLIB - AviBit Calculation Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Alexander Randeu, a.randeu@avibit.com
    \author  QT4-PORT: Gerhard Scheikl, g.scheikl@avibit.com
    \brief   Declares class for stereographic transformations based on
             EUROCONTROL TransLib SASSC-SRD-980154 and
             ARTAS IRS 46127301-506/V7.1/29.10.2004 Appendix A2
*/



// Qt includes

// local includes
#include "avlog.h"
#include "avmisc.h" //for AV_PI
#include "avstereotrafo.h"

///////////////////////////////////////////////////////////////////////////////
//                        STATIC VARIABLE DEFINITIONS
///////////////////////////////////////////////////////////////////////////////

const double AVStereoTrafo::PI = AV_PI;
const double AVStereoTrafo::RAD2DEG = 180.0 / PI;
const double AVStereoTrafo::DEG2RAD = PI / 180.0;

//Epsilon for double comparsion
static const double DOUBLE_EPS = 1e-20;

///////////////////////////////////////////////////////////////////////////////

bool AVStereoTrafo::stereoObs2Geod(const double &u,
                                   const double &v,
                                   const double &H,
                                   double &geod_lat,
                                   double &geod_lon,
                                   double &geod_h) const
{
    AVASSERT(m_radar_set_done);

    double xc, yc, zc;

    if(stereoObs2SysCart(u, v, H, xc, yc, zc))
    {
        double xg, yg, zg;
        if(sysCart2GeocenSysCart(xc, yc, zc, xg, yg, zg))
        {
            if (geocenSysCart2Geod_rad(xg, yg, zg, geod_lat, geod_lon, geod_h))
            {
                geod_lat *= RAD2DEG;
                geod_lon *= RAD2DEG;
                return true;
            }
        }
    }

    geod_lat = 0.;
    geod_lon = 0.;
    geod_h = 0.;

    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool AVStereoTrafo::geod2StereoObs(const double &geod_lat,
                                   const double &geod_lon,
                                   const double &geod_h,
                                   double &u,
                                   double &v,
                                   double &H) const
{
    AVASSERT(m_radar_set_done);

    double xg, yg, zg;
    if (geod_rad2GeocenSysCart(geod_lat*DEG2RAD, geod_lon*DEG2RAD, geod_h, xg, yg, zg))
    {
        double xc, yc, zc;
        if(geocenSysCart2SysCart(xg, yg, zg, xc, yc, zc))
        {
            if(sysCart2StereoObs(xc, yc, zc, u, v, H))
            {
                return true;
            }
        }
    }

    u = 0.;
    v = 0.;
    H = 0.;

    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool AVStereoTrafo::stereoObs2SysCart(const double &u,
                                      const double &v,
                                      const double &H,
                                      double &xc,
                                      double &yc,
                                      double &zc) const
{
    AVASSERT(m_radar_set_done);

    double d_uv2 = u*u + v*v;

    double cc = 2.0*m_RT + m_huv;
    double cc2 = cc*cc;
    zc = (H + m_RT)*((cc2 - d_uv2)/(cc2 + d_uv2)) - (m_RT + m_hxy);

    double k = cc/(2.0*m_RT + m_hxy + zc + H);
    if (fabs(k) < DOUBLE_EPS)
    {
        xc = 0.0;
        yc = 0.0;
        zc = 0.0;
        return false;
    }

    xc = u/k;
    yc = v/k;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVStereoTrafo::sysCart2StereoObs(const double &xc,
                                      const double &yc,
                                      const double &zc,
                                      double &u,
                                      double &v,
                                      double &H) const
{
    AVASSERT(m_radar_set_done);

    double d_xy2 = xc*xc + yc*yc;

    double cc = zc + m_hxy + m_RT;
    H = sqrt(d_xy2 + cc*cc) - m_RT;

    double k = (2.0*m_RT + m_huv)/(2.0*m_RT + m_hxy + zc + H);

    u = k*xc;
    v = k*yc;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVStereoTrafo::sysCart2GeocenSysCart(const double &xc,
                                          const double &yc,
                                          const double &zc,
                                          double &xg,
                                          double &yg,
                                          double &zg) const
{
    AVASSERT(m_radar_set_done);

    xg = m_R2_mat[0][0]*xc + m_R2_mat[1][0]*yc + m_R2_mat[2][0]*zc +  m_T2_vec[0];
    yg = m_R2_mat[0][1]*xc + m_R2_mat[1][1]*yc + m_R2_mat[2][1]*zc +  m_T2_vec[1];
    zg = m_R2_mat[0][2]*xc + m_R2_mat[1][2]*yc + m_R2_mat[2][2]*zc +  m_T2_vec[2];

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVStereoTrafo::geocenSysCart2SysCart(const double &xg,
                                          const double &yg,
                                          const double &zg,
                                          double &xc,
                                          double &yc,
                                          double &zc) const
{
    AVASSERT(m_radar_set_done);

    double x = xg - m_T2_vec[0];
    double y = yg - m_T2_vec[1];
    double z = zg - m_T2_vec[2];

    xc = m_R2_mat[0][0]*x + m_R2_mat[0][1]*y + m_R2_mat[0][2]*z;
    yc = m_R2_mat[1][0]*x + m_R2_mat[1][1]*y + m_R2_mat[1][2]*z;
    zc = m_R2_mat[2][0]*x + m_R2_mat[2][1]*y + m_R2_mat[2][2]*z;

    return true;
}

///////////////////////////////////////////////////////////////////////////////


bool AVStereoTrafo::geocenSysCart2Geod_rad(const double &xgs,
                                        const double &ygs,
                                        const double &zgs,
                                        double &geod_lat,
                                        double &geod_lon,
                                        double &geod_h) const
{
#if 0
    //implementation belongs to SASSC-TransLib-SR-25
    AVASSERT(m_radar_set_done);

    double alpha, beta, sqrtx2y2, tlat0, tlat1, tlat2, cc, cc0, cc1, cc2;

    sqrtx2y2 = sqrt(xgs*xgs + ygs*ygs);

    if(fabs(sqrtx2y2) < DOUBLE_EPS) {
        //pole geod_lat = +/-pi/2

        geod_lon = 0.;

        if(zgs > 0.0) geod_lat = PI/2.0;
        else geod_lat = -PI/2.0;

        geod_h = zgs - ell.geta();
    }
    else {
        //longitude
        geod_lon = atan2(ygs, xgs);

        //latitude
        alpha = zgs/sqrtx2y2;
        beta = ell.gete2()*ell.geta()/sqrtx2y2;

        cc = (1.0 - ell.gete2());
        AVASSERT(fabs(cc) > DOUBLE_EPS);
        tlat0 = alpha/cc;

        cc0 = 1.0 + cc*tlat0*tlat0;
        AVASSERT(cc0 > DOUBLE_EPS);
        tlat1 = alpha + beta*(tlat0/sqrt(cc0));

        cc0 = 1.0 + cc*tlat1*tlat1;
        AVASSERT(cc0 > DOUBLE_EPS);
        tlat2 = alpha + beta*(tlat1/sqrt(cc0));

        cc1 = (tlat2 - 2*tlat1 + tlat0);
        if(fabs(cc1) < DOUBLE_EPS)
            geod_lat = atan(tlat2);
        else
            geod_lat = atan(tlat0 - pow(tlat1 - tlat0, 2.0)/cc1);

        //height
        cc2 = sqrt(1.0 - ell.gete2()*pow(sin(geod_lat),2.0));
        AVASSERT(fabs(cc2) > DOUBLE_EPS);

        if(fabs(geod_lat) < PI/4.0)
            geod_h = sqrtx2y2/cos(geod_lat) - ell.geta()/cc2;
        else
            geod_h = zgs/sin(geod_lat) - (cc*ell.geta())/cc2;
    }

#else
    //there also exist a closed formula for this, but it does not
    //conform to SASSC-TransLib doc.

    double sqrtx2y2, cc, es2, cc1, cc2, a, b, clat, N;

    sqrtx2y2 = sqrt(xgs*xgs + ygs*ygs);

    if(fabs(sqrtx2y2) < DOUBLE_EPS) {
        //pole geod_lat = +/-pi/2

        geod_lon = 0.;

        if(zgs > 0.0) geod_lat = PI/2.0;
        else geod_lat = -PI/2.0;

        geod_h = zgs - ell.getb();
    }
    else {
        a = ell.geta();
        b = ell.getb();

        geod_lon = atan2(ygs, xgs);

        cc = atan2((zgs*a), (sqrtx2y2*b));
        es2 = pow(a/b,2.0) - 1.0;

        cc1 = zgs + es2*b*pow(sin(cc), 3.0);
        cc2 = sqrtx2y2 - ell.gete2()*a*pow(cos(cc), 3.0);
        geod_lat = atan2(cc1, cc2);

        clat = cos(geod_lat);
        N = a*a/(b*sqrt(1 + es2*clat*clat));
        geod_h = sqrtx2y2/clat - N;
    }
#endif

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVStereoTrafo::geod_rad2GeocenSysCart(const double &geod_lat,
                                           const double &geod_lon,
                                           const double &geod_h,
                                           double &xgs,
                                           double &ygs,
                                           double &zgs) const
{
    //implementation belongs to SASSC-TransLib-SR-24
    AVASSERT(m_radar_set_done);

    double sla = sin(geod_lat);
    double cla = cos(geod_lat);
    double slo = sin(geod_lon);
    double clo = cos(geod_lon);

    double cc = 1.0 - ell.gete2()*sla*sla;
    AVASSERT(cc > 0.0);
    cc = ell.geta()/sqrt(cc);

    xgs = (geod_h + cc)*cla*clo;
    ygs = (geod_h + cc)*cla*slo;
    zgs = (geod_h + cc*(1.0 - ell.gete2()))*sla;

    return true;
}

// End of file
