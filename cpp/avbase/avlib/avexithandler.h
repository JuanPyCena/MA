///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - AviBit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author    Stefan Kunz, s.kunz@avibit.com
    \author    QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief     Interface for exit handlers
*/


#ifndef __AVEXITHANDLER_H__
#define __AVEXITHANDLER_H__

// Qt includes
#include <QObject>
#include <QEvent>

#include "avlib_export.h"

//! Interface for exit handlers
class AVLIB_EXPORT AVExitHandler
{
public:
    //! Standard Constructor
    AVExitHandler() {}

    //! Destructor
    virtual ~AVExitHandler() {}

    //! Handles the required exit
    virtual void handleExit() = 0;

private:
    //! Hidden copy-constructor
    AVExitHandler(const AVExitHandler&);
    //! Hidden assignment operator
    const AVExitHandler& operator = (const AVExitHandler&);
};

#endif /* __AVEXITHANDLER_H__ */

// End of file
