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
    \brief   Declaration of classes for calculating transformation lookup values
*/

#if !defined(AVTRANSFORMATIONCALCULATOR_H_INCLUDED)
#define AVTRANSFORMATIONCALCULATOR_H_INCLUDED

// Qt includes
#include <QTransform>

#include "avcalculationlib_export.h"

// forward declarations
class AVCoordinateTransformBase;

///////////////////////////////////////////////////////////////////////////////

class AVCALCULATIONLIB_EXPORT AVTransformationCalculator
{
public:
    virtual ~AVTransformationCalculator() {}

    virtual bool transform(double inX, double inY, double& outX, double& outY) = 0;

    virtual bool untransform(double inX, double inY, double& outX, double& outY) = 0;

    //! Returns the name of a AVTransformationCalculator instance
    virtual QString name() { return QString::null; }
};

///////////////////////////////////////////////////////////////////////////////

class AVCALCULATIONLIB_EXPORT AVProjectionTransformationCalculator : public AVTransformationCalculator
{
public:
    AVProjectionTransformationCalculator(AVCoordinateTransformBase* sourceProjection,
                                         AVCoordinateTransformBase* targetProjection);

    ~AVProjectionTransformationCalculator() override;

    bool transform(double inX, double inY, double& outX, double& outY) override;

    bool untransform(double inX, double inY, double& outX, double& outY) override;

    //! Returns the name of a AVProjectionTransformationCalculator instance
    //! The name is composed of "source projection name" + "-" + "target projection name"
    //! \sa AVCoordinateTransformBase::name(), AVCoordinateTransformBase::setName()
    QString name() override;

protected:
    AVCoordinateTransformBase* m_sourceProjection;
    AVCoordinateTransformBase* m_targetProjection;

private:
    //! Hidden copy constructor
    AVProjectionTransformationCalculator(const AVProjectionTransformationCalculator&);

    //! Hidden assignment operator
    AVProjectionTransformationCalculator& operator=(const AVProjectionTransformationCalculator&);
};

///////////////////////////////////////////////////////////////////////////////

class AVCALCULATIONLIB_EXPORT AVPlanarTransformationCalculator : public AVTransformationCalculator
{
public:
    AVPlanarTransformationCalculator();

    ~AVPlanarTransformationCalculator() override;

    bool transform(double inX, double inY, double& outX, double& outY) override;

    bool untransform(double inX, double inY, double& outX, double& outY) override;

    virtual void setTransform(const QTransform& transform);

    inline QTransform transformMatrix()   const { return m_transform;   }
    inline QTransform untransformMatrix() const { return m_untransform; }

protected:
    QTransform m_transform;
    QTransform m_untransform;
};

#endif

// End of file
