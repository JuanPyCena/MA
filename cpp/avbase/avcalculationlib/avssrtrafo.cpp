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
    \brief   Declares class for ssr coordinate transformations based on
             EUROCONTROL TransLib SASSC-SRD-980154
*/


// Qt includes

// local includes
#include "avlog.h"
#include "avmisc.h" //for AV_PI
#include "avssrtrafo.h"



///////////////////////////////////////////////////////////////////////////////
//                        STATIC VARIABLE DEFINITIONS
///////////////////////////////////////////////////////////////////////////////

const double AVSSRTrafo::PI = AV_PI;
const double AVSSRTrafo::RAD2DEG = 180.0 / PI;
const double AVSSRTrafo::DEG2RAD = PI / 180.0;

//Epsilon for double comparsion
static const double DOUBLE_EPS = 1e-20;

///////////////////////////////////////////////////////////////////////////////

bool AVSSRTrafo::radarObs2Geod(const double &rho,
                               const double &theta,
                               const double &H,
                               double &geod_lat,
                               double &geod_lon,
                               double &geod_h) const
{
    double xr, yr, zr;

    if(radarObs_rad2RadarCart(rho, theta*DEG2RAD, H, xr, yr, zr))
    {
        if (radarCart2Geod(xr, yr, zr, geod_lat, geod_lon, geod_h))
        {
            return true;
        }
    }

    geod_lat = 0.;
    geod_lon = 0.;
    geod_h = 0.;

    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSSRTrafo::geod2RadarObs(const double &geod_lat,
                               const double &geod_lon,
                               const double &geod_h,
                               double &rho,
                               double &theta,
                               double &H) const
{
    double xr, yr, zr;

    if(geod2RadarCart(geod_lat, geod_lon, geod_h, xr, yr, zr))
    {
        if(radarCart2RadarObs_rad(xr, yr, zr, rho, theta, H)) {
            theta *= RAD2DEG;
            return true;
        }
    }
    rho = 0.;
    theta = 0.;
    H = 0.;

    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSSRTrafo::radarCart2Geod(const double &xr,
                                const double &yr,
                                const double &zr,
                                double &geod_lat,
                                double &geod_lon,
                                double &geod_h) const
{
    double xgs, ygs, zgs;

    if(radarCart2GeocenSysCart(xr, yr, zr, xgs, ygs, zgs))
    {
        if(geocenSysCart2Geod_rad(xgs, ygs, zgs, geod_lat,
                                  geod_lon, geod_h))
        {
            geod_lat *= RAD2DEG;
            geod_lon *= RAD2DEG;
            return true;
        }
    }

    geod_lat = 0.;
    geod_lon = 0.;
    geod_h = 0.;

    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSSRTrafo::radarCartHeight2Geod(const double &xr,
                                      const double &yr,
                                      const double &H,
                                      double &geod_lat,
                                      double &geod_lon,
                                      double &geod_h) const
{
    //implementation belongs to SASSC-TransLib-SR-28
    double R0 = ell.geta()*(1.0 - 0.5*ell.gete2()*cos(2.0*m_glatr));

    double B = -2.0*(R0 + m_ghr);
    double cc1 = R0 + H;
    double C = cc1*cc1 - xr*xr - yr*yr;

    if(C < 0.0){
        AVLogger->Write(LOG_ERROR, "AVSSRTrafo::radarCartHeight2Geod: "
                        "Error: measurements are not valid: (xr=%f, yr=%f"
                        ", H=%f)", xr, yr, H);
        geod_lat = 0.0;
        geod_lon = 0.0;
        geod_h   = 0.0;
        return false;
    }

    double zr = B + 2.0*sqrt(C);

    return radarCart2Geod(xr, yr, zr, geod_lat, geod_lon, geod_h);
}

///////////////////////////////////////////////////////////////////////////////

bool AVSSRTrafo::geod2RadarCartHeight(const double &geod_lat,
                                      const double &geod_lon,
                                      const double &geod_h,
                                      double &xr,
                                      double &yr,
                                      double &H) const
{
    Q_UNUSED(geod_lat);
    Q_UNUSED(geod_lon);
    Q_UNUSED(geod_h);
    Q_UNUSED(xr);
    Q_UNUSED(yr);
    Q_UNUSED(H);

    AVLogger->Write(LOG_FATAL, "AVSSRTrafo::geod2RadarCartHeight: "
                    "Function not implemented yet!");
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSSRTrafo::geod2RadarCart(const double &geod_lat,
                                const double &geod_lon,
                                const double &geod_h,
                                double &xr,
                                double &yr,
                                double &zr) const
{
    double xgs, ygs, zgs;

    double glat = geod_lat*DEG2RAD;
    double glon = geod_lon*DEG2RAD;

    if(geod_rad2GeocenSysCart(glat, glon, geod_h, xgs, ygs, zgs))
    {
        if(geocenSysCart2RadarCart(xgs, ygs, zgs, xr, yr, zr))
        {
            return true;
        }
    }

    xr = 0.;
    yr = 0.;
    zr = 0.;

    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSSRTrafo::radarObs_rad2RadarCart(const double &rho,
                                        const double &theta,
                                        const double &H,
                                        double &xr,
                                        double &yr,
                                        double &zr) const
{
    //implementation belongs to SASSC-TransLib-SR-28
    double rho2, cc, cc1, R0, R0H, R0H0, R0H2, R0H02, cosalpha;


    R0 = ell.geta()*(1.0 - 0.5*ell.gete2()*cos(2.0*m_glatr));
    AVASSERT(R0 > 0.0);

    rho2 = rho*rho;

//NOTE: The following code from the translib was replaced by code from ralex to fix some problems.
//(see /aman_docs/design/comments_to_sassc-translib-sr-28.pdf)
#if 0
    if(rho < DOUBLE_EPS) {
        if(fabs(H - m_ghr) < DOUBLE_EPS) zr = 0.0;
        else {
            AVLogger->Write(LOG_ERROR, "AVSSRTrafo::radarObs2RadarCart_rad: "
                            "Error: (rho=0., but H != ghr, H=%f , ghr=%f)",
                            H, m_ghr);
            xr = 0.0;
            yr = 0.0;
            zr = 0.0;
            return false;
        }
    }
    else {
       //this one is from TransLib
        zr = H - m_ghr - (rho2 - H*H) / (2.0*R0);
    }
#else
    //this one is the corrected code from ralex

    R0H = R0 + H;
    R0H0 = R0 + m_ghr;
    R0H2 = R0H*R0H;
    R0H02 = R0H0*R0H0;

    cosalpha = (R0H2 + R0H02 - rho2)/(2.0*R0H*R0H0);

    //test for valid measurements
    if(fabs(cosalpha) > 1.0){
        AVLogger->Write(LOG_ERROR, "AVSSRTrafo::radarObs2RadarCart_rad: "
                        "Error: measurements are not valid: (rho=%f, H=%f"
                        ", ghr=%f)", rho, H, m_ghr);
        xr = 0.0;
        yr = 0.0;
        zr = 0.0;
        return false;
    }
    else {
        if(rho < DOUBLE_EPS)
            zr = 0.0;
        else
            //zr = (R0H2 - rho2)/(2.0*R0H0) - 0.5*R0H0;
            zr = R0H*cosalpha - R0H0;
    }
    //end from ralex
#endif

    cc = rho2 - zr*zr;
    if(cc < 0.0) {
        if(-cc < DOUBLE_EPS) cc = 0.0; //rounding errors aso.
        else {
            AVLogger->Write(LOG_ERROR, "AVSSRTrafo::radarObs2RadarCart_rad: "
                            "Error: (rho^2 - zr^2 < 0.0, rho=%f , zr=%f)",
                            rho, zr);
            xr = 0.0;
            yr = 0.0;
            zr = 0.0;
            return false;
        }
    }

    cc1 = sqrt(cc);

    xr = cc1*sin(theta);
    yr = cc1*cos(theta);

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSSRTrafo::radarCart2RadarObs_rad(const double &xr,
                                        const double &yr,
                                        const double &zr,
                                        double &rho,
                                        double &theta,
                                        double &H) const
{
    //implementation is NOT given in SASSC-TransLib

    theta = atan2(xr, yr);
    rho   = sqrt(xr*xr + yr*yr + zr*zr);

    double R0   = ell.geta()*(1.0 - 0.5*ell.gete2()*cos(2.0*m_glatr));
    double R0H0 = R0 + m_ghr;

    H = sqrt((2.0*zr + R0H0)*R0H0 + rho*rho) - R0;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSSRTrafo::radarCart2GeocenSysCart(const double &xr,
                                         const double &yr,
                                         const double &zr,
                                         double &xgs,
                                         double &ygs,
                                         double &zgs) const
{
    //implementation belongs to SASSC-TransLib-SR-27
    double v, sla, cla, slo, clo, xgs1, ygs1, zgs1, e2sla, cc;

    sla = sin(m_glatr);
    cla = cos(m_glatr);
    slo = sin(m_glonr);
    clo = cos(m_glonr);

    e2sla = ell.gete2()*sla;
    cc = 1.0 - e2sla*sla;

    AVASSERT(cc > DOUBLE_EPS);

    v = ell.geta()/sqrt(cc);

    xgs1 = xr;
    ygs1 = yr - e2sla*cla*v;
    zgs1 = zr - (e2sla*sla*v - (v + m_ghr));

    xgs = -slo*xgs1 - sla*clo*ygs1 + cla*clo*zgs1;
    ygs =  clo*xgs1 - sla*slo*ygs1 + cla*slo*zgs1;
    zgs =                 cla*ygs1 +     sla*zgs1;

#if 0
    //another way
    double v, sla, cla, slo, clo, xgs1, ygs1, zgs1, e2sla, cc, e2;
    double xr1,yr1,zr1;

    sla = sin(m_glatr);
    cla = cos(m_glatr);
    slo = sin(m_glonr);
    clo = cos(m_glonr);

    e2 = ell.gete2();
    e2sla = e2*sla;
    cc = 1.0 - e2sla*sla;

    AVASSERT(cc > DOUBLE_EPS);

    v = ell.geta()/sqrt(cc);

    xr1 = xr;
    yr1 = yr;
    zr1 = zr;

    xgs1 = -slo*xr1 - sla*clo*yr1 + cla*clo*zr1;
    ygs1 =  clo*xr1 - sla*slo*yr1 + cla*slo*zr1;
    zgs1 =                cla*yr1 +     sla*zr1;

    xgs = xgs1 + (v + m_ghr)*cla*clo;
    ygs = ygs1 + (v + m_ghr)*cla*slo;
    zgs = zgs1 + ((1.0 - e2)*v + m_ghr)*sla;
#endif

    return true;
}

///////////////////////////////////////////////////////////////////////////////


bool AVSSRTrafo::geocenSysCart2RadarCart(const double &xgs,
                                         const double &ygs,
                                         const double &zgs,
                                         double &xr,
                                         double &yr,
                                         double &zr) const
{
    //implementation belongs to SASSC-TransLib-SR-26
    double sla = sin(m_glatr);
    double cla = cos(m_glatr);
    double slo = sin(m_glonr);
    double clo = cos(m_glonr);

    double e2sla = ell.gete2()*sla;
    double cc = 1.0 - e2sla*sla;
    AVASSERT(cc > DOUBLE_EPS);

    double v = ell.geta()/sqrt(cc);

    xr = -slo*xgs     + clo*ygs               + 0;
    yr = -sla*clo*xgs - sla*slo*ygs + cla*zgs + e2sla*cla*v;
    zr =  cla*clo*xgs + cla*slo*ygs + sla*zgs + (e2sla*sla*v - (v + m_ghr));

    return true;
}

///////////////////////////////////////////////////////////////////////////////


bool AVSSRTrafo::geocenSysCart2Geod_rad(const double &xgs,
                                        const double &ygs,
                                        const double &zgs,
                                        double &geod_lat,
                                        double &geod_lon,
                                        double &geod_h) const
{
#if 0
    // implementation belongs to SASSC-TransLib-SR-25

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
    // there also exist a closed formula for this, but it does not
    // conform to SASSC-TransLib doc.
    // checked in AVStereoTrafo Unit Test

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

bool AVSSRTrafo::geod_rad2GeocenSysCart(const double &geod_lat,
                                        const double &geod_lon,
                                        const double &geod_h,
                                        double &xgs,
                                        double &ygs,
                                        double &zgs) const
{
    //implementation belongs to SASSC-TransLib-SR-24
    double sla = sin(geod_lat);
    double cla = cos(geod_lat);
    double slo = sin(geod_lon);
    double clo = cos(geod_lon);

    double cc = 1.0 - ell.gete2()*sla*sla;
    AVASSERT(cc > 0.0);
    cc = ell.geta()/sqrt(cc);

    xgs = (cc + geod_h)*cla*clo;
    ygs = (cc + geod_h)*cla*slo;
    zgs = cc*(1.0 - ell.gete2())*sla + geod_h*sla;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSSRTrafo::radarObsZ2Geod(const double &rho,
                                const double &theta,
                                const double &z,
                                double &geod_lat,
                                double &geod_lon,
                                double &geod_h) const
{
    double theta_rad = theta*DEG2RAD;
    double rho2z2    = rho*rho - z*z;

    if(rho2z2 < 0.0){
        AVLogger->Write(LOG_ERROR, "AVSSRTrafo::radarObsZ2Geod: "
                        "Error: measurements are not valid: (rho=%f, theta=%f"
                        ", z=%f)", rho, theta, z);
        geod_lat = 0.0;
        geod_lon = 0.0;
        geod_h   = 0.0;
        return false;
    }

    double xr = sqrt(rho2z2)*sin(theta_rad);
    double yr = sqrt(rho2z2)*cos(theta_rad);

    if (radarCart2Geod(xr, yr, z, geod_lat, geod_lon, geod_h))
    {
        return true;
    }

    geod_lat = 0.;
    geod_lon = 0.;
    geod_h   = 0.;

    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSSRTrafo::geod2RadarObsZ(const double &geod_lat,
                                const double &geod_lon,
                                const double &geod_h,
                                double &rho,
                                double &theta,
                                double &z) const
{
    double xr, yr;

    if(geod2RadarCart(geod_lat, geod_lon, geod_h, xr, yr, z))
    {
        rho   = sqrt(xr*xr + yr*yr + z*z);
        theta = atan2(xr, yr)*RAD2DEG;
        return true;
    }
    rho   = 0.;
    theta = 0.;
    z     = 0.;

    return false;
}

///////////////////////////////////////////////////////////////////////////////

QString AVSSRTrafo::uniqueName() const
{
    return AVCoordinateTransform3dBase::uniqueName() + "_" + QString::number(m_glatr) + "_" +
            QString::number(m_glonr) + "_" + QString::number(m_ghr);
}

// End of file
