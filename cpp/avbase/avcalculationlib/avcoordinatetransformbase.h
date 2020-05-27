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
    \author  Clemens Ender, c.ender@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Deflares base class for coordinate transformations
*/

#if !defined(AVCOORDINATETRANSFORMBASE_H_INCLUDED)
#define AVCOORDINATETRANSFORMBASE_H_INCLUDED

#include <QString>

// system includes
#include <cmath>
#include <iostream>
using namespace std;

#include "avcalculationlib_export.h"

///////////////////////////////////////////////////////////////////////////////
//! base class for assorted coordinate transformations

class AVCALCULATIONLIB_EXPORT AVCoordinateTransformBase
{
public:
    //! constructor
    explicit AVCoordinateTransformBase(const QString& name = QString::null);
    //! destructor
    virtual ~AVCoordinateTransformBase();
    //! convert x/y to lat/long in degrees
    /*! this function just returns true, so derived classes have to implement
        this function to perform a calculation
     */
    virtual bool xy2latlong(double x, double y, double &la, double &lo) const = 0;
    //! lat/long in degrees to x/y
    /*! this function just returns true, so derived classes have to implement
        this function to perform a calculation
     */
    virtual bool latlong2xy(double la, double lo, double &x, double &y) const = 0;
    //! return x/y to m resolution scaling factor (meters per pixel)
    /*! this function just returns true, so derived classes have to implement
        this function to perform a calculation
     */
    virtual bool getResFactor(double &res) const = 0;

    //! NMPM the factor to calculate from nautical miles to meter.
    //! one nautical mile is 1852 meters.
    static const double NMPM;

    //! RE is the radius of the earth in m.
    static const double RE;

    //! FEET to Meter constant
    static const double FTOM;

    //! Sets the trafo name
    virtual void setName(const QString& name){ m_name = name; }

    //! Returns the unique trafo name
    virtual QString uniqueName() const { return m_name; }

    //! Returns the trafo name
    virtual QString name() const { return m_name; }

protected:

    QString m_name; //!< trafo name

protected:
    inline double avMod(double y, double x) const {
        double mod = fmod(y, x);
        if (mod < 0.0) mod += x;
        return mod;
    }
};

#endif

// End of file
