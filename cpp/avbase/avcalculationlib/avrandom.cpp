///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2013
//
// Module:    AVCALCULATIONLIB - AviBit Calculation Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   AVRandom implementation
 */

#include <limits>
#include <stdlib.h>

// Qt includes
#include <QtGlobal>
#include <qmath.h>

// avlib includes

// local includes
#include "avrandom.h"


// work around windows namespace pollution
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif


double AVRandom::m_cached_normal = std::numeric_limits<double>::max();

///////////////////////////////////////////////////////////////////////////////

double AVRandom::randUnit()
{
    return qrand()/(static_cast<double>(RAND_MAX)+1.);
}

///////////////////////////////////////////////////////////////////////////////

double AVRandom::randNormal()
{
    double value = 0;

    if(m_cached_normal != std::numeric_limits<double>::max())
    {
        value = m_cached_normal;
        m_cached_normal = std::numeric_limits<double>::max();
    }
    else
    {
        double R = qSqrt(-2.l*qLn(1.-randUnit())); // (1-..) exludes 0
        double theta = 2*M_PI*randUnit();

        value = R*qCos(theta);
        m_cached_normal = R*qSin(theta);
    }
    return value;
}

// End of file
