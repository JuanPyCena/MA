///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - AviBit Library
//
/////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  DI Alexander Wemmer, a.wemmer@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief
*/

#ifndef __AVPROTOTYPE_H__
#define __AVPROTOTYPE_H__

#include "avlib_export.h"

//! Interface for classes providing prototype capabilities.
//! The clone() method returns a clone of the prototype class.
//! \sa AVProtoTypeFactory
class AVLIB_EXPORT AVProtoType
{
public:
    //! Standard Constructor
    AVProtoType();

    //! Destructor
    virtual ~AVProtoType();

    //! Returns a clone of this prototype. Must be implemented in all derived classes.
    virtual AVProtoType* clone() const = 0;

    //FIXXME clone with parent
};

#endif /* __AVPROTOTYPE_H__ */

// End of file
