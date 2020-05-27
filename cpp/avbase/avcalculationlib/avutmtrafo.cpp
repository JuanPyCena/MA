///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  DI Alexander Randeu, a.randeu@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief
*/




// AVLib includes
#include "avlog.h"
#include "avsciconst.h"

// local includes
#include "avutmtrafo.h"

///////////////////////////////////////////////////////////////////////////////

AVUtmTrafo::AVUtmTrafo(double ref_mer, const AVCoordEllipsoid& ref_ell)
    : AVGaussKruegerTrafo(ref_mer, ref_ell)
{
}

///////////////////////////////////////////////////////////////////////////////

AVUtmTrafo::~AVUtmTrafo()
{
}

///////////////////////////////////////////////////////////////////////////////

bool AVUtmTrafo::xy2latlong(double x, double y, double &la, double &lo) const
{
    double x1 = x/0.9996;
    double y1 = y/0.9996;
    AVGaussKruegerTrafo::xy2latlong(x1, y1, la, lo);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVUtmTrafo::latlong2xy(double la, double lo, double &x, double &y) const
{
    AVGaussKruegerTrafo::latlong2xy(la, lo, x, y);
    x *= 0.9996;
    y *= 0.9996;
    return true;
}

// End of file
