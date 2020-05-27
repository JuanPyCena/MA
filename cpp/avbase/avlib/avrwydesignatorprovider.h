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

#ifndef __AVRWYDESIGNATORPROVIDER_H__
#define __AVRWYDESIGNATORPROVIDER_H__

#include "avlib_export.h"

/////////////////////////////////////////////////////////////////////////////

//! rwy id <-> rwy designator converter interface definition
class AVLIB_EXPORT AVRwyDesignatorProvider
{
public:
    //! Destroys the instance
    virtual ~AVRwyDesignatorProvider() {}

    //! Returns the RWY ID for the given RWY designator
    virtual int getRwyIdByDesignator(const QString& rwy_designator) const = 0;

    //! Returns the RWY designator for the given RWY ID
    virtual QString getRwyDesignatorById(int id) const = 0;
};

/////////////////////////////////////////////////////////////////////////////

#endif /* __AVRWYDESIGNATORPROVIDER_H__ */

// End of file

