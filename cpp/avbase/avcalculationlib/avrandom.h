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
    \brief   AVRandom header
 */

#ifndef AVRANDOM_INCLUDED
#define AVRANDOM_INCLUDED

// Qt includes
#include <QObject>

// avlib includes
#include "avcalculationlib_export.h"
#include "avmacros.h"

// local includes

// forward declarations

///////////////////////////////////////////////////////////////////////////////
//! Helper methods for random numbers
/*!
*/
class AVCALCULATIONLIB_EXPORT AVRandom
{
public:

    //! returns a equally distributed random number in half-open interval [0,1)
    static double randUnit();

    //! returns a normally distributed random number using Box-Muller algorithm
    static double randNormal();


private:

    static double m_cached_normal;
};

#endif // AVRANDOM_INCLUDED

// End of file
