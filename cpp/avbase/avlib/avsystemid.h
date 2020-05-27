///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Thomas Pock, t.pock@avibit.com
    \author  QT4-PORT: Alexander Randeu, a.randeu@avibit.com
    \brief   Class to find out the system id of a computer
*/

//! This class is used to find out the System ID of a computer
/*!
    After creating the object, you are able to find out the System ID
    by calling the getSID() function. 
*/

#ifndef AVSYSTEMID_H
#define AVSYSTEMID_H

#include "avlib_export.h"

#include <qstring.h>

class AVLIB_EXPORT AVSystemId
{
// Con(De)structors
public:
    //! Standard constructor
    AVSystemId();
    //! Standard destructor
    ~AVSystemId();

// Methods
public:
    //! Returns the the System ID of the computer
    QString getSID();
};

#endif



// End of file
