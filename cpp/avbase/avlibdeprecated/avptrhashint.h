///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIBDEBRECATED - Avibit Library Deprecated classes
//                               for compatibility with ported code from QT3
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
 \author  Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
 \brief   AVPtrHashInt class header.
 */

#if !defined AVPTRHASHINT_H_INCLUDED
#define AVPTRHASHINT_H_INCLUDED


// local includes
#include "avptrhash.h"

/////////////////////////////////////////////////////////////////////////////
//! Reimplemetation of QT3 class QIntDict to ensure compatibility of old code
/*!
 *
 */

template<class TYPE> class AVPtrHashInt : public AVPtrHash<int, TYPE>
{
public:
    explicit AVPtrHashInt(int size = 0) :
        AVPtrHash<int, TYPE>(size)
    {
    }

    virtual ~AVPtrHashInt()
    {
    }

};


#endif

// End of file
