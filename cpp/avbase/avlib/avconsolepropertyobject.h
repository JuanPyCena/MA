//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2015
//
// Module:    AVLIB - AviBit Library
//
//-----------------------------------------------------------------------------

/*! \file
    \brief   AVConsolePropertyObject header
 */

#ifndef AVCONSOLEPROPERTYOBJECT_INCLUDED
#define AVCONSOLEPROPERTYOBJECT_INCLUDED

// Qt includes
#include <QString>

#include "avlib_export.h"

//-----------------------------------------------------------------------------
//! Interface for declaring the functionality of being able to print the current
//! internal status to a QString. A child class deriving from AVConsolePropertyObject
//! needs to implement the AVConsolePropertyObject::toString method to provide 
//! a string representation of its content.
class AVLIB_EXPORT AVConsolePropertyObject
{
public:
    AVConsolePropertyObject();
    virtual ~AVConsolePropertyObject();

    virtual QString toString() const = 0;

private:

};

#endif // AVONSOLEPROPERTYOBJECT_INCLUDED

// End of file
