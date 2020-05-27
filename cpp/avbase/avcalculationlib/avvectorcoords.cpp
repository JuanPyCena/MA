///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVCALCULATIONLIB - AviBit Calculation Library
//
/////////////////////////////////////////////////////////////////////////////

/*!
\file
\author  Dietmar G?sseringer, d.goesseringer@avibit.com
\author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
\brief   base for 2-D geometry: vectors & coordinates
*/

// local includes
#include "avvectorcoords.h"

///////////////////////////////////////////////////////////////////////////////

AVVectorCoords AVVectorCoords::Cartesian(double x, double y)
{
    return AVVectorCoords(x, y);
}

///////////////////////////////////////////////////////////////////////////////

AVVectorCoords AVVectorCoords::Cart(double x, double y)
{
    return AVVectorCoords(x, y);
}

///////////////////////////////////////////////////////////////////////////////

AVVectorCoords AVVectorCoords::Cart(double start_x, double start_y,
    double end_x, double end_y)
{
    return AVVectorCoords(end_x - start_x, end_y - start_y);
}

///////////////////////////////////////////////////////////////////////////////

AVVectorCoords AVVectorCoords::PolarDeg(double r, double phi_deg)
{
    double phi_rad = phi_deg * AV_PI / 180.0;
    return AVVectorCoords(r*cos(phi_rad), r*sin(phi_rad));
}

///////////////////////////////////////////////////////////////////////////////

AVVectorCoords AVVectorCoords::PolD(double r, double phi_deg)
{
    double phi_rad = phi_deg * AV_PI / 180.0;
    return AVVectorCoords(r*cos(phi_rad), r*sin(phi_rad));
}

///////////////////////////////////////////////////////////////////////////////

AVVectorCoords AVVectorCoords::PolarRad(double r, double phi_rad)
{
    return AVVectorCoords(r*cos(phi_rad), r*sin(phi_rad));
}

///////////////////////////////////////////////////////////////////////////////

AVVectorCoords AVVectorCoords::PolR(double r, double phi_rad)
{
    return AVVectorCoords(r*cos(phi_rad), r*sin(phi_rad));
}

///////////////////////////////////////////////////////////////////////////////

AVVectorCoords AVVectorCoords::Velocity(double heading, double speed)
{
    double phi_rad = (90.0 - heading) * AV_PI / 180.0;
    return AVVectorCoords(speed*cos(phi_rad), speed*sin(phi_rad));
}

// End of file
