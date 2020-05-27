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
    \brief   Implementation of a meteosat coordinate system transformation class
*/


// local includes
#include "avmeteosattrafo.h"
#include "avmisc.h"


///////////////////////////////////////////////////////////////////////////////
//                        STATIC VARIABLE DEFINITIONS
///////////////////////////////////////////////////////////////////////////////

const double AVMeteoSatTrafo::V0  = -0.261799;
const double AVMeteoSatTrafo::V1  =  0.523599;
const double AVMeteoSatTrafo::V2  =  670.0;
const double AVMeteoSatTrafo::V3  = -20.0;
const double AVMeteoSatTrafo::V4  =  1.16937;
const double AVMeteoSatTrafo::V5  =  0.837758;
const double AVMeteoSatTrafo::V6  = -130.0;
const double AVMeteoSatTrafo::V7  =  980.0;

const double AVMeteoSatTrafo::LEVEL = 0.0001 * AV_PI / 180.0;
const double AVMeteoSatTrafo::ITLIM = 1000;

const double AVMeteoSatTrafo::R = 6379000.0;

double AVMeteoSatTrafo::H = (42163000.0 - AVMeteoSatTrafo::R);

///////////////////////////////////////////////////////////////////////////////
//                         class AVMeteoSatTrafo
///////////////////////////////////////////////////////////////////////////////

AVMeteoSatTrafo::AVMeteoSatTrafo() : AVCoordinateTransformBase("MeteoSatTrafo")
{
}

///////////////////////////////////////////////////////////////////////////////

bool AVMeteoSatTrafo::getResFactor(double &res) const
{
    Q_UNUSED( res );
    AVASSERT( 0 );
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool AVMeteoSatTrafo::xy2latlong(double x, double y, double &la, double &lo) const
{
    bool result = xy2latlong_rad(x, y, la, lo);
    if (result) {
        la = rad2deg(la);
        lo = rad2deg(lo);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool AVMeteoSatTrafo::xy2latlong_rad(double x, double y, double &la, double &lo) const
{
    lo = atan((0.0-((6370000.0*((2.0)/(1.0+sin(V1)))*cos(V1)*sin(V0))-(V3-x)/
         ((V3-V7)/((6370000.0*((2.0)/(1.0+sin(V1)))*cos(V1)*sin(V0))-(6370000.0
         *((2.0)/(1.0+sin(V5)))*cos(V5)*sin(V4))))))/(((0.0-6370000.0)*((2.0)/(1.0
         +sin(V1)))*cos(V1)*cos(V0))-(V2-y)/((V2-V6)/(((0.0-6370000.0)*((2.0)/(1.0
         +sin(V1)))*cos(V1)*cos(V0))-((0.0-6370000.0)*((2.0)/(1.0+sin(V5)))*cos(V5)*
         cos(V4))))));

    la = 2.0*atan(6370000.0*(2.0/((6370000.0*((2.0)/(1.0+sin(V1)))*cos(V1)*sin(V0))
         -(V3-x)/((V3-V7)/((6370000.0*((2.0)/(1.0+sin(V1)))*cos(V1)*sin(V0))-
         (6370000.0*((2.0)/(1.0+sin (V5)))*cos(V5)*sin(V4))))))*sin((atan((0.0-
         ((6370000.0*((2.0)/(1.0+sin(V1)))*cos(V1)*sin(V0))-(V3-x)/((V3-V7)/
         ((6370000.0*((2.0)/(1.0+sin(V1)))*cos(V1)*sin(V0))-(6370000.0*((2.0)/
         (1.0+sin (V5)))*cos(V5)*sin(V4))))))/(((0.0-6370000.0)*((2.0)/(1.0+
         sin(V1)))*cos(V1)*cos(V0))-(V2-y)/((V2-V6)/(((0.0-6370000.0)*((2.0)/
         (1.0+sin (V1)))*cos(V1)*cos(V0))-((0.0-6370000.0)*((2.0)/(1.0+sin(V5)))*
         cos(V5)*cos(V4)))))))))-90.0*3.14159265359/180.0;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVMeteoSatTrafo::latlong2xy(double la, double lo, double &x, double &y) const
{
    return latlong2xy_rad(deg2rad(la), deg2rad(lo), x, y);
}

///////////////////////////////////////////////////////////////////////////////

bool AVMeteoSatTrafo::latlong2xy_rad(double la, double lo, double &x, double &y) const
{
    y = V2-(((0.0-6370000.0)*(2.0/(1.0+sin(V1)))*cos(V1)*cos(V0))-((0.0-6370000.0)
        *(2.0/(1.0+sin(la)))*cos(la)*cos(lo)))*((V2-V6)/(((0.0-6370000.0)*((2.0)/
        (1.0+sin(V1)))*cos(V1)*cos(V0))-((0.0-6370000.0)*((2.0)/(1.0+sin(V5)))*
        cos(V5)*cos(V4))));

    x = V3-((6370000.0*((2.0)/(1.0+sin(V1)))*cos(V1)*sin(V0))-(6370000.0*(2.0/
        (1.0+sin (la)))*cos(la)*sin(lo)))*((V3-V7)/((6370000.0*((2.0)/(1.0+
        sin(V1)))*cos(V1)*sin(V0))-(6370000.0*((2.0)/(1.0+sin(V5)))*cos(V5)*
        sin(V4))));

    return true;
}

///////////////////////////////////////////////////////////////////////////////

// End of file
