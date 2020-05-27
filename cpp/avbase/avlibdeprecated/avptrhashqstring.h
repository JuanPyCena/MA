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
 \brief   AVPtrHashQString class header.
 */

#if !defined AVPTRHASHQSTRING_H_INCLUDED
#define AVPTRHASHQSTRING_H_INCLUDED

// QT includes
#include <QString>

// local includes
#include "avptrhash.h"
#include <avlog.h>

/////////////////////////////////////////////////////////////////////////////
//! Reimplemetation of QT3 class QDict to ensure compatibility of old code
/*!
 *
 */

template<class TYPE> class AVPtrHashQString: public AVPtrHash<QString, TYPE>
{
public:
    explicit AVPtrHashQString(int size = 0, bool case_sensitive=true) :
        AVPtrHash<QString, TYPE>(size),
        m_case_sensitive(case_sensitive)
    {
        if(!case_sensitive)
        {
            AVLogFatal << "AVPtrHashQString::AVPtrHashQString: "
                    "Not case sensitive: not implemented"; //QT4PORT TODO
        }
    }
    virtual ~AVPtrHashQString()
    {
    }

private:
    bool m_case_sensitive;
};

#endif

// End of file
