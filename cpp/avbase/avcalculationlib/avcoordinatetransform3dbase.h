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

#ifndef __AVCOORDINATETRANSFORM3DBASE_H__
#define __AVCOORDINATETRANSFORM3DBASE_H__

#include <qstring.h>

// system includes
#include <cmath>
#include <iostream>
using namespace std;

#include "avcalculationlib_export.h"

///////////////////////////////////////////////////////////////////////////////
//! base class for assorted coordinate transformations in 3D, meaning it takes
//! 3 coordinates as input and generates 3 coordinates as output.
/*! ToDo: Think about a common base class with AVCoordinateTransformBase!
*/

class AVCALCULATIONLIB_EXPORT AVCoordinateTransform3dBase
{
public:
    //! constructor
    explicit AVCoordinateTransform3dBase(const QString& name = QString::null);

    //! destructor
    virtual ~AVCoordinateTransform3dBase();

    /*! sets the radar position in geodetic coordinates belonging
        to the ellipsoid set with setEllipsoid()
        \param geod_lat geodetic latitude of radar <deg.>
        \param geod_lon geodetic longitude of radar <deg.>
        \param geod_h geodetic height of radar <m>
        \return true if successful, false if not (e.g. see note)
        \note geod_lat must be between -90deg. <= geod_lat <= + 90deg.
     */
    virtual bool setRadarPos(const double &geod_lat, const double &geod_lon,
                             const double &geod_h) = 0;

    /*! returns the radar position in geodetic coordinates
        \param geod_lat geodetic latitude of radar <deg.>
        \param geod_lon geodetic longitude of radar <deg.>
        \param geod_h geodetic height of radar <m>
     */
    virtual void getRadarPos(double &geod_lat, double &geod_lon, double &geod_h) const = 0;

    //! returns the geod. radar height
    virtual double getRadarGeodHeightM() const = 0;

    //! convert u/v/w to lat/long/h in degrees/meters
    /*! this function just returns true, so derived classes have to implement
        this function to perform a calculation
     */
    virtual bool uvw2latlonh(const double &u, const double &v, const double &w,
                             double &la, double &lo, double &h) const = 0;

    //! lat/long in degrees/meters to u/v/w
    /*! this function just returns true, so derived classes have to implement
        this function to perform a calculation
     */
    virtual bool latlonh2uvw(const double &la, const double &lo, const double &h,
                             double &u, double &v, double &w) const = 0;

    //! Sets the trafo name
    virtual void setName(const QString& name){ m_name = name; }

    //! Returns the unique trafo name
    virtual QString uniqueName() const { return m_name; }

    //! Returns the trafo name
    virtual QString name() const { return m_name; }

protected:

    QString m_name; //!< trafo name

};

#endif /* __AVCOORDINATETRANSFORM3DBASE_H__ */

// End of file

// End of file
