///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVCALCULATIONLIB - AviBit Calculation Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  DI Alexander Randeu, a.randeu@avibit.com
    \author  QT4-PORT: Gerhard Scheikl, g.scheikl@avibit.com
    \brief   Deflares base class for coordinate transformations
*/



// local includes
#include "avcoordinatetransform3dbase.h"

///////////////////////////////////////////////////////////////////////////////

AVCoordinateTransform3dBase::AVCoordinateTransform3dBase(const QString& name)
    : m_name(name)
{
}

///////////////////////////////////////////////////////////////////////////////

AVCoordinateTransform3dBase::~AVCoordinateTransform3dBase()
{
}

// End of file
