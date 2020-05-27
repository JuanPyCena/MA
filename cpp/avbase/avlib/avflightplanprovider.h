///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  DI Alexander Wemmer, a.wemmer@avibit.com
    \author  QT4-PORT: Gerhard Scheikl, g.scheikl@avibit.com
    \brief
*/

#ifndef __AVFLIGHTPLANPROVIDER_H__
#define __AVFLIGHTPLANPROVIDER_H__

#include "avlib_export.h"

/////////////////////////////////////////////////////////////////////////////

class AVMsgFlightplan;
class QString;

//! ifplid -> flightplan converter interface definition
//!   No export here because all methods are abstract or inline, 
//!   otherwise it may lead to MSVC compile errors in depending code (see SWE-4779).
//!   Finding a suitable location for this class is described in SWE-4784.
class AVFlightplanProvider
{
public:
    //! Destroys the instance
    virtual ~AVFlightplanProvider() {}

    //! must be called before using getFPbyIFPLID to lock the FP provider
    virtual void lockFPAccess() = 0;

    //! returns a dict holding the current FPs
    virtual const AVMsgFlightplan* getFPbyIFPLID(const QString& ifplid) const = 0;

    //! must be called after using getFPbyIFPLID to unlock the FP provider
    virtual void unlockFPAccess() = 0;
};

/////////////////////////////////////////////////////////////////////////////

#endif /* __AVFLIGHTPLANPROVIDER_H__ */

// End of file

