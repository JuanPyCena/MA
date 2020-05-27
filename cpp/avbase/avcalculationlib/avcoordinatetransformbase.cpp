///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVCALCULATIONLIB - AviBit Calculation Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Thomas Leitner, t.leitner@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Base class for coordinate transformations.
*/


#include "avsciconst.h"
#include "avcoordinatetransformbase.h"


///////////////////////////////////////////////////////////////////////////////
//                        STATIC VARIABLE DEFINITIONS
///////////////////////////////////////////////////////////////////////////////

const double AVCoordinateTransformBase::NMPM = AVSciConst::nmTokm * 1000.0;  //!< 1 nm = 1852 m
const double AVCoordinateTransformBase::RE   = AVSciConst::Re;  //!< mean earth radius = 6378000.0 m
const double AVCoordinateTransformBase::FTOM = AVSciConst::ftTom;  //!< 1 ft = 0.3048 m

///////////////////////////////////////////////////////////////////////////////

AVCoordinateTransformBase::AVCoordinateTransformBase(const QString& name)
    : m_name(name)
{
}

///////////////////////////////////////////////////////////////////////////////

AVCoordinateTransformBase::~AVCoordinateTransformBase()
{
}

// End of file
