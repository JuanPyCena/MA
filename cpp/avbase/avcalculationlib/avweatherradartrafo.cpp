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
    \brief   Implementation of a weather radar coordinate syten transformation class
*/


// local includes
#include "avweatherradartrafo.h"
#include "avmisc.h"


///////////////////////////////////////////////////////////////////////////////
//                         class AVWeatherRadarTrafo
///////////////////////////////////////////////////////////////////////////////

AVWeatherRadarTrafo::AVWeatherRadarTrafo(double v0, double v1, double v2,
                                         double v3, double v4, double v5)
    : AVCoordinateTransformBase("WeatherRadarTrafo"),
      V0(v0), V1(v1), V2(v2), V3(v3), V4(v4), V5(v5)
{
}

///////////////////////////////////////////////////////////////////////////////

bool AVWeatherRadarTrafo::getResFactor(double &res) const
{
    Q_UNUSED( res );
    AVASSERT( 0 );
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVWeatherRadarTrafo::xy2latlong(double x, double y, double &la, double &lo) const
{
    bool result = xy2latlong_rad(x, y, la, lo);
    if (result) {
        la = rad2deg(la);
        lo = rad2deg(lo);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool AVWeatherRadarTrafo::xy2latlong_rad(double x, double y, double &la, double &lo) const
{
    lo = acotan(5899179.2/((5676531.8*pow((tan((1.570796326795-(V1*0.017453292520))
         /2)/0.373884679485),0.737361597616)*sin(0.012869387656*(V0-13.333333333))
         -V3*V4)+x*V4)-((6380000.0*(0.924636243305-0.889738536848*pow((tan((
         1.570796326795-(V1*0.017453292520))/2)/0.373884679485),0.737361597616)*
         cos(0.012869387656*(V0-13.333333333)))+V2*V5)-y*V5)/((5676531.8*pow((
         tan((1.570796326795-(V1*0.017453292520))/2)/0.373884679485),
         0.737361597616)*sin(0.012869387656*(V0-13.333333333))-V3*V4)+x*V4))/
         0.737361597616+0.232710566928;

    la = (1.570796326795-2*atan(0.404026225835*pow((1/sin(acotan(5899179.2/
         ((5676531.8*pow((tan((1.570796326795-(V1*0.017453292520))/2)/
         0.373884679485),0.737361597616)*sin(0.012869387656*(V0-13.333333333))-
         V3*V4)+x*V4)-((6380000.0*(0.924636243305-0.889738536848*pow((tan((
         1.570796326795-(V1*0.017453292520))/2)/0.373884679485),0.737361597616)*
         cos(0.012869387656*(V0-13.333333333)))+V2*V5)-y*V5)/((5676531.8*
         pow((tan((1.570796326795-(V1*0.017453292520))/2)/0.373884679485),
         0.737361597616)*sin(0.012869387656*(V0-13.333333333))-V3*V4)+x*V4))))*
         ((5676531.8*pow((tan((1.570796326795-(V1*0.017453292520))/2)/
         0.373884679485),0.737361597616)*sin(0.012869387656*(V0-13.333333333))-
         V3*V4)+x*V4)/6380000.0*0.737361597616/sin(0.767944870878),1/
         0.737361597616)));

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVWeatherRadarTrafo::latlong2xy(double la, double lo, double &x, double &y) const
{
    return latlong2xy_rad(deg2rad(la), deg2rad(lo), x, y);
}

///////////////////////////////////////////////////////////////////////////////

bool AVWeatherRadarTrafo::latlong2xy_rad(double la, double lo, double &x, double &y) const
{
    y = ((6380000.0*(0.924636243305-0.889738536848*pow((tan((1.570796326795-
        (V1*0.017453292520))/2.0)/0.373884679485),0.737361597616)*cos(
        0.012869387656*(V0-13.333333333)))+V2*V5)-6380000.0*(0.924636243305-
        0.889738536848*pow((tan((1.570796326795-la)/2.0)/0.373884679485),
        0.737361597616)*cos(0.737361597616*(lo-0.232710566352))))/V5;

    x = (5676531.8*pow((tan((1.570796326795-la)/2.0) / 0.373884679485),
        0.737361597616)*sin(0.737361597616*(lo-0.232710566352))-(5676531.8*pow((
        tan((1.570796326795-(V1*0.017453292520))/2.0)/0.373884679485),
        0.737361597616)*sin(0.012869387656*(V0-13.333333333))-V3*V4))/V4;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

QString AVWeatherRadarTrafo::uniqueName() const
{
    QString params;
    AVsprintf(params, "%f_%f_%f_%f_%f_%f", V0, V1, V2, V3, V4, V5);
    return m_name + params;
}

// End of file
