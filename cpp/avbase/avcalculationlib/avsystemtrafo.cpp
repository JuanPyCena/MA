///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2011
//
// Module:    AVCALCULATIONLIB - AviBit Calculation Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Clemens Ender, c.ender@avibit.com
    \author  Dr. Thomas Leitner, t.leitner@avibit.com
    \author  Alexander Randeu, a.randeu@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Implement system transformation from lat/long (WGS84) to a
             system x/y coordinate systems in meters.
*/


// local includes
#include "avsystemtrafo.h"
#include "avsystemtrafocfg.h"
#include "avmisc.h"

///////////////////////////////////////////////////////////////////////////////
//                        STATIC VARIABLE DEFINITIONS
///////////////////////////////////////////////////////////////////////////////

// const static members
const double AVSystemTrafo::A      = 6378137.000;         //!< Aequator-radius
const double AVSystemTrafo::B      = 6356752.314;         //!< Pole-Radius
const double AVSystemTrafo::E2     = 1.0 - (B*B) / (A*A); //!< square of the numeric excentricity
const double AVSystemTrafo::RES    = 1.0;                 //! RESOLUTION in Meters per Pixel
static const double DOUBLE_EPS     = 1e-20;               //!< epsilon for double comparsions

// non const static members
double AVSystemTrafo::DEFAULT_LATP  = 47.5 * AV_PI / 180.0;  //!< origin, LAT
double AVSystemTrafo::DEFAULT_LONGP = 14.0 * AV_PI / 180.0;  //!< origin, LONG
double AVSystemTrafo::DEFAULT_EN    = A / sqrt(1.0 - E2 * sin(DEFAULT_LATP) * sin(DEFAULT_LATP));
bool   AVSystemTrafo::DEFAULT_ORIGIN_FILE_LOADED = false;

///////////////////////////////////////////////////////////////////////////////
//                         class AVSystemTrafo
///////////////////////////////////////////////////////////////////////////////

// TODO Name should include config name/hash
AVSystemTrafo::AVSystemTrafo() : AVCoordinateTransformBase("SystemTrafo")
{
    bool first_init = false;
    if (!DEFAULT_ORIGIN_FILE_LOADED)
    {
        AVConfig2Container cfg_container;
        AVSystemTrafoConfig cfg(cfg_container);

        DEFAULT_LATP  = deg2rad(cfg.m_origin_lat);
        DEFAULT_LONGP = deg2rad(cfg.m_origin_lon);
        DEFAULT_EN    = A / sqrt(1.0 - E2 * sin(DEFAULT_LATP) * sin(DEFAULT_LATP));

        DEFAULT_ORIGIN_FILE_LOADED = true;
        first_init = true;
    }

    m_latp  = DEFAULT_LATP;
    m_longp = DEFAULT_LONGP;
    m_en    = DEFAULT_EN;

    if (first_init)
    {
        LOGGER_ROOT.Write(LOG_INFO,
            "AVSystemTrafo::AVSystemTrafo: "
            "Initialized with DEFAULT system origin %s for the 1st time. Origin values stored "
            "statically. Inhibiting further default system origin configuration file loads...",
            qPrintable(AVSystemTrafo::toString()));
    }
}

///////////////////////////////////////////////////////////////////////////////

AVSystemTrafo::AVSystemTrafo(const QString& cfgfilename) : AVCoordinateTransformBase("SystemTrafo")
{
    bool log = !(AVSystemTrafoConfig::ORIGIN_CONFIG_FILENAMES.contains(cfgfilename));

    AVConfig2Container cfg_container;
    AVSystemTrafoConfig cfg(cfg_container, cfgfilename);
    m_latp  = deg2rad(cfg.m_origin_lat);
    m_longp = deg2rad(cfg.m_origin_lon);
    m_en    = A / sqrt(1.0 - E2 * sin(m_latp) * sin(m_latp));

    if (log)
        LOGGER_ROOT.Write(LOG_INFO,
            "AVSystemTrafo::AVSystemTrafo: "
            "Initialized with system origin %s for the 1st time. "
            "Inhibiting eventual further logs with this origin...",
            qPrintable(AVSystemTrafo::toString()));
}

///////////////////////////////////////////////////////////////////////////////

AVSystemTrafo::AVSystemTrafo(double rlat, double rlong) : AVCoordinateTransformBase("SystemTrafo")
{
    m_latp  = deg2rad(rlat);      //! origin, LAT
    m_longp = deg2rad(rlong);     //! origin, LONG
    m_en    = A / sqrt(1.0 - E2 * sin(m_latp) * sin(m_latp));

    LOGGER_ROOT.Write(LOG_INFO,
        "AVSystemTrafo::AVSystemTrafo: "
        "Initialized with system origin %s",
        qPrintable(toString()));
}

///////////////////////////////////////////////////////////////////////////////

bool AVSystemTrafo::getResFactor(double &resolution) const
{
    resolution = RES;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSystemTrafo::xy2latlong(double x, double y, double &la, double &lo) const
{
    bool result = xy2latlong_rad(x, y, la, lo);
    if (result) {
        la = rad2deg(la);
        lo = rad2deg(lo);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSystemTrafo::xy2latlong_rad(double x, double y, double &la, double &lo) const
{
    return xy2latlongRef_rad(x, y, la, lo, m_latp, m_longp, m_en);
}

///////////////////////////////////////////////////////////////////////////////

bool AVSystemTrafo::xy2latlongRef_rad(double x, double y, double &la, double &lo,
                                      double rla, double rlo, double en) const
{
    y = -y;

    double rlat;
    double rho = sqrt(x*x + y*y);
    if (fabs(rho) < DOUBLE_EPS) {
        rlat = rla;
        // longitude is ready
        lo = rlo;
    } else {
        double sinlatp = sin(rla);
        double coslatp = cos(rla);
        double m1      = coslatp/sqrt(1.0 - E2*sinlatp*sinlatp);
        double ceval   = 2.0*atan(rho*coslatp/(2.0*A*m1));
        double cosce   = cos(ceval);
        double since   = sin(ceval);

        rlat           = asin(cosce*sinlatp + (y*since*coslatp)/rho);
        // longitude is ready
        lo             = rlo + atan2(x*since, rho*coslatp*cosce -
                                     y*sinlatp*since);
    }

    // now latitude psi from rlat
    if (fabs(rlat - AV_PI/2.0) < DOUBLE_EPS) {
        la = AV_PI/2.0;
    } else if (fabs(rlat + AV_PI/2.0) < DOUBLE_EPS) {
        la = -AV_PI/2.0;
    } else {
        double C  = E2 * en * sin(rla);
        double cos2rlat = cos(rlat)*cos(rlat);
        double sin2rlat = sin(rlat)*sin(rlat);
        double C2A2 = (C/A)*(C/A);
        double a1val = cos2rlat*(1.0 - C2A2*(E2/(1.0 - E2))) - sin2rlat;
        double bbval = 2.0*sin(rlat)*cos(rlat);
        double cc = cos2rlat*C2A2 - sin2rlat;
        double aaval = cc - a1val;

        // qDebug("rlat=%.6f (%.6f) rla=%.6f (%.6f)", rlat, rad2deg(rlat), rla, rad2deg(rla));
        double tt = bbval*bbval - 4.0*aaval*cc;
        // TODO: check if this is a correct assumption
        if(tt < 0.0) tt = 0.0;

        double tan_psi = 0.0;
        if((rlat < 0.0 && rla >= 0.0) || (rlat >= 0.0 && rla >= 0.0)) tan_psi = (-bbval + sqrt(tt))/(2.0*aaval);
        else                                                          tan_psi = (-bbval - sqrt(tt))/(2.0*aaval);

        la = atan2(tan_psi, 1.0 - E2);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSystemTrafo::latlong2xy(double la, double lo, double &x, double &y) const
{
    return latlong2xy_rad(deg2rad(la), deg2rad(lo), x, y);
}

///////////////////////////////////////////////////////////////////////////////

bool AVSystemTrafo::latlong2xy_rad(double la, double lo, double &x, double &y) const
{
    return latlong2xyRef_rad(la, lo, x, y, m_latp, m_longp, m_en);
}

///////////////////////////////////////////////////////////////////////////////

bool AVSystemTrafo::latlong2xyRef(double la, double lo, double &x, double &y,
                                  double rla, double rlo) const
{
    double rrla = deg2rad(rla);
    double rrlo = deg2rad(rlo);
    double enval = A / sqrt(1.0 - E2 * sin(rrla) * sin(rrla));
    return latlong2xyRef_rad(deg2rad(la), deg2rad(lo), x, y, rrla, rrlo, enval);
}

///////////////////////////////////////////////////////////////////////////////

bool AVSystemTrafo::latlong2xyRef_rad(double la, double lo,
                                      double &x, double &y,
                                      double rla, double rlo,
                                      double en) const
{
    double psi  = atan((1.0 - E2) * tan(la));
    double c    = E2 * en * sin(rla);
    double bbval   = A / sqrt ( 1 + ((E2 * sin(psi) * sin(psi)) / (1.0 - E2)));
    double rlat = atan((c + bbval * sin(psi)) / (bbval * cos(psi)));
    double a11  = cos(rlat) * cos(rla) * cos(rlo - lo) +
                  sin(rlat) * sin(rla);
    double a21  = -cos(rlat) * sin(rlo - lo);
    double a31  = sin(rlat) * cos(rla) -
                  cos(rlat) * sin(rla) * cos(rlo - lo);
    x = 2.0 * en * a21 / (1.0 + a11);
    y = 2.0 * en * a31 / (1.0 + a11);
    y = -y;                              // we need an inverse Y coordinate
    return true;
}

///////////////////////////////////////////////////////////////////////////////

QString AVSystemTrafo::uniqueName() const
{
    return m_name + "_" + QString::number(m_latp) + "_" + QString::number(m_longp);
}

///////////////////////////////////////////////////////////////////////////////

QString AVSystemTrafo::toString() const
{
    return m_name + ":LAT/LON=" +
           QString::number(rad2deg(m_latp)) + "/" + QString::number(rad2deg(m_longp));
}

// End of file
