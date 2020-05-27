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
    \author  QT4-PORT: Anton Skrabal, a.skrabal@avibit.com
    \brief   External callback for client handlers
*/

#ifndef __AVCLIENTHANDLERCALLBACKIFACE_H__
#define __AVCLIENTHANDLERCALLBACKIFACE_H__

#include "avlib_export.h"

class QDataStream;

//! External callback for client handlers.
//! The callback is provided with a socket device, which can be used to read
//!  commands, etc.
class AVLIB_EXPORT AVClientHandlerCallbackIface
{
public:
    //! Standard Constructor
    AVClientHandlerCallbackIface() {};

    //! Destructor
    virtual ~AVClientHandlerCallbackIface() {};

    //! May be called by another module in order to execute external tasks.
    //! "datastream" may be used to read commands, etc. from, "state_return_value",
    //! "int_return_value" and "double_return_value" may be used to return a
    //! state and an arbitrary value.
    virtual void callback(QDataStream& datastream,
                          quint32& state_return_value,
                          quint32& int_return_value,
                          double& double_return_value) = 0;

protected:

private:
    //! Hidden copy-constructor
    AVClientHandlerCallbackIface(const AVClientHandlerCallbackIface&);
    //! Hidden assignment operator
    const AVClientHandlerCallbackIface& operator = (const AVClientHandlerCallbackIface&);
};

#endif /* __AVCLIENTHANDLERCALLBACKIFACE_H__ */

// End of file
