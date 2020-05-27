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
 \brief   AVPtrHashVoid class header.
 */

#if !defined AVPTRHASHVOID_H_INCLUDED
#define AVPTRHASHVOID_H_INCLUDED

// local includes
#include "avptrhash.h"

/////////////////////////////////////////////////////////////////////////////
//! Reimplemetation of QT3 class QPtrDict to ensure compatibility of old code
/*!
 *
 */

template<class TYPE> class AVPtrHashVoid : public AVPtrHash<void*, TYPE>
{
public:
    explicit AVPtrHashVoid(int size=0):
        AVPtrHash<void*, TYPE>(size)
    {

    }

    virtual ~AVPtrHashVoid()
    {
    }


};

#endif

// End of file
