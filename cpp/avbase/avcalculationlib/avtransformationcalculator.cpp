///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVCALCULATIONLIB - AviBit Calculation Library
//
///////////////////////////////////////////////////////////////////////////////

/*!
    \file
    \author  Kevin Krammer, k.krammer@avibit.com
    \author  QT4-PORT: Andreas Schuller, a.schuller@avibit.com
    \brief   Implementation of classes for calculating transformation lookup values
*/


// AVLib includes
#include "avcoordinatetransformbase.h"
#include "avlog.h"
#include "avmisc.h"
#include "avtransformationcalculator.h"


///////////////////////////////////////////////////////////////////////////////

AVProjectionTransformationCalculator:: AVProjectionTransformationCalculator(
        AVCoordinateTransformBase* sourceProjection, AVCoordinateTransformBase* targetProjection)
    : m_sourceProjection(sourceProjection),
      m_targetProjection(targetProjection)
{
    AVASSERT(m_sourceProjection);
    AVASSERT(m_targetProjection);
}

///////////////////////////////////////////////////////////////////////////////

AVProjectionTransformationCalculator::~AVProjectionTransformationCalculator()
{
}

///////////////////////////////////////////////////////////////////////////////

bool AVProjectionTransformationCalculator::transform(double inX, double inY,
                                                     double& outX, double& outY)
{
    double latitude  = 0.0;
    double longitude = 0.0;

    if (!m_sourceProjection->xy2latlong(inX, inY, latitude, longitude))
    {
        LOGGER_AVCOMMON_AVLIB.Write(LOG_ERROR,
                "Source projection failed to transform x/y (%f, %f) to lat/long",
                inX, inY);
        return false;
    }

    if (!m_targetProjection->latlong2xy(latitude, longitude, outX, outY))
    {
        LOGGER_AVCOMMON_AVLIB.Write(LOG_ERROR,
                "Target projection failed to transform lat/long (%f, %f) to x/y",
                inX, inY);
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVProjectionTransformationCalculator::untransform(double inX, double inY,
                                                       double& outX, double& outY)
{
    double latitude  = 0.0;
    double longitude = 0.0;

    if (!m_targetProjection->xy2latlong(inX, inY, latitude, longitude))
    {
        LOGGER_AVCOMMON_AVLIB.Write(LOG_ERROR,
                "Target projection failed to untransform x/y (%f, %f) to lat/long",
                inX, inY);
        return false;
    }

    if (!m_sourceProjection->latlong2xy(latitude, longitude, outX, outY))
    {
        LOGGER_AVCOMMON_AVLIB.Write(LOG_ERROR,
                "Source projection failed to untransform lat/long (%f, %f) to x/y",
                inX, inY);
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

QString AVProjectionTransformationCalculator::name()
{
    return m_sourceProjection->uniqueName() + "-" +  m_targetProjection->uniqueName();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

AVPlanarTransformationCalculator::AVPlanarTransformationCalculator()
{
}

///////////////////////////////////////////////////////////////////////////////

AVPlanarTransformationCalculator::~AVPlanarTransformationCalculator()
{
}

///////////////////////////////////////////////////////////////////////////////

bool AVPlanarTransformationCalculator::transform(double inX, double inY,
                                                 double& outX, double& outY)
{
    m_transform.map(inX, inY, &outX, &outY);

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVPlanarTransformationCalculator::untransform(double inX, double inY,
                                                   double& outX, double& outY)
{
    m_untransform.map(inX, inY, &outX, &outY);

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void AVPlanarTransformationCalculator::setTransform(const QTransform& transform)
{
    if (transform == m_transform) return;

    bool invertOK = true;

    m_transform   = transform;
    m_untransform = transform.inverted(&invertOK);

    if (!invertOK)
    {
        AVLogError << &LOGGER_AVCOMMON_AVLIB << "AVPlanarTransformationCalculator::setMatrix: "
                   "inverting transformation matrix is not possible, untransform() will "
                   "not work";
    }
}

// End of File
