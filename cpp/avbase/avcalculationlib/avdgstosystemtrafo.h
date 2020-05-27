///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit Data Processing GmbH, 2001-2011
//
// Module:     AVCALCULATIONLIB - AviBit Calculation Library
//
/////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Madalina Hodorog, m.hodorog@avibit.com
    \brief   Conversion class for a target point from Cartesian coordinates referenced by an
    arbitrary sensor system to WGS84 coordinates and to Cartesian coordinates referenced by
    the system with origin in the latitude, and longitude coordinates specified by the
    system trafo class.
*/

#if !defined(AVDGSTOSYSTEMTRAFO_H_INCLUDED)
#define AVDGSTOSYSTEMTRAFO_H_INCLUDED

// system includes
#include <cmath>

// AVLib includes
#include "avcalculationlib_export.h"

#include "avcoordinatetransformbase.h"
#include "avmisc.h"

#include "avssrtrafo.h"             // for using the AVSSRRadarCartTrafo class
#include "avaviationcalculator.h"   // for using the AVAviationCalculator class

//forward declarations
class AVSystemTrafo;

///////////////////////////////////////////////////////////////////////////////
//!  Implementation of the AVDGSToSystemTrafo class
/*!
*    The class AVDGSToSystemTrafo implements coordinate transformation from
*    a x/y/z-Cartesian coordinates with respect to an arbitrary sensor system
*    (i.e. a laser sensor system used for the docking guidance system) to lat/long/height-WGS84
*    coordinates, and then to the x/y-Cartesian coordinates system with origin in the
*    latitude, and longitude coordinates specified by the system trafo class, i.e.
*    - given a sensor and a reference point
*    (with known coordinates in the x/y/z Cartesian and the lat/long/height WGS84 systems
*    w.r.t the sensor) and a target point (with known coordinates in the x/y/z Cartesian
*    coordinates w.r.t the sensor),
*    - the class computes the lat/long/height-WGS84 coordinates of the target point and
*    the x/y-Cartesian coordinates of the target point with respect to the system trafo.
*
*    Documents of interest: SDD_DAQ_DGS.
*
*    @note: The class was originally implemented as the kernel of the DAQ_DGS module, thus for more
*    information, please check the SDD-DAQ_DGS design document.
*/
class AVCALCULATIONLIB_EXPORT AVDGSToSystemTrafo
{

    //! friend declaration for function level test case
    friend class TstAVDGSToSystemTrafo;

public:

    //! Creates an AVDGSToSystemTrafo object
    explicit AVDGSToSystemTrafo(double laser_lat,
                                double laser_long,
                                double laser_h,
                                double reference_lat,
                                double reference_long,
                                double reference_h,
                                double reference_x,
                                double reference_y,
                                double reference_z);

    //! Destructs the AVDGSToSystemTrafo object
    virtual ~AVDGSToSystemTrafo();

    //! Transforms the x/y/z-Cartesian coordinates of a target point w.r.t. the sensor system to
    //! lat/long/height-WGS84 coordinates
    //! returns the lat/long/height-WGS84 coordinates of the target point
    /*
        \param target_x      the x-Cartesian coordinate of the target point w.r.t. the sensor system
        \param target_y      the y-Cartesian coordinate of the target point w.r.t. the sensor system
        \param target_z      the z-Cartesian coordinate of the target point w.r.t. the sensor system
        \param target_lat    returns the latitude of the target point
        \param target_long   returns the longitude of the target point
        \param target_h      returns the height of the target point
    */
    virtual void xyz2latlonh(double target_x,
                             double target_y,
                             double target_z,
                             double &target_lat,
                             double &target_long,
                             double &target_h);

    //! Transforms the x/y/z-Cartesian coordinates of a target point w.r.t. the sensor system to
    //! x/y-Cartesian coordinates w.r.t. the system trafo
    //! returns the x/y-Cartesian coordinates of the target point w.r.t. the system trafo
    /*
        \param target_x      the x-Cartesian coordinate of the target point w.r.t. the sensor system
        \param target_y      the y-Cartesian coordinate of the target point w.r.t. the sensor system
        \param target_z      the z-Cartesian coordinate of the target point w.r.t. the sensor system
        \param target_x_av   returns the x-Cartesian coordinate of the target point w.r.t. the system trafo
        \param target_y_av   returns the y-Cartesian coordinate of the target point w.r.t. the system trafo
    */
    virtual void xyz2xySystemTrafo(double target_x,
                                   double target_y,
                                   double target_z,
                                   double &target_x_av,
                                   double &target_y_av);

    //! Transforms the lat/long-coordinates of a target point w.r.t. the sensor system to
    //! x/y-Cartesian coordinates w.r.t. the system trafo
    //! returns the lat/lon/height-WGS84 coordinates of the target point w.r.t. the system trafo
    /*  \param target_lat       the latitude of the target point given in WGS84 coordinates
        \param target_long      the longitude of the target point given in WGS84 coordinates
        \param target_x_av      returns the x-Cartesian coordinate of the target point w.r.t. the system trafo
        \param target_y_av      returns the y-Cartesian coordinate of the target point w.r.t. the system trafo
    */
    virtual void latlong2xySystemTrafo(double target_lat,
                                       double target_long,
                                       double& target_x_av,
                                       double& target_y_av);

protected:

    AVSSRTrafo *m_laser_trafo;
    AVSystemTrafo *m_system_trafo;

private:
    AVDISABLECOPY(AVDGSToSystemTrafo);

    //! Computes the rotation angle referenced to the OY-Cartesian ordinate in clockwise direction
    //! w.r.t the sensor system
    /*
       \return the rotation angle in radians between the OY-Cartesian ordinate in clockwise
       direction and the vector referenced by the reference point
    */
    double getRotationAngleFromOrdinate() const;

    //! Computes the rotation angle referenced to the North direction in clockwise direction
    //! w.r.t the sensor system
    /*
       \return the rotation angle in radians between the North direction and the vector
       referenced by the reference point
    */
    double getRotationAngleFromNorth() const;

    //! Computes the rotated coordinates of the x/y-coordinates referenced to the rotation_angle
    //! in counterclockwise direction w.r.t. the sensor system
    /* \param rotation_angle_radians    the rotation angle around the axis in radians
       \param x                         the x-coordinate of the given point
       \param y                         the y-coordinate of the given point
       \param x_rotated                 returns the x-coordinate of the given point rotated around
       the rotation_angle
       \param y_rotated                 returns the y-coordinate of the given point rotated around
       the rotation_angle
    */
    void getRotatedCoordinates(double rotation_angle_radians,
                               double x,
                               double y,
                               double &x_rotated,
                               double &y_rotated) const;

private:

    double m_laser_lat;         //! Latitude of the laser point w.r.t. the sensor system
    double m_laser_long;        //! Longitude of the laser point w.r.t the sensor system
    double m_laser_h;           //! Height of the laser point w.r.t. the sensor system
    double m_reference_lat;     //! Latitude of the reference point w.r.t. the sensor system
    double m_reference_long;    //! Longitude of the reference point w.r.t. the sensor system
    double m_reference_h;       //! Height of the reference point w.r.t. the sensor system
    double m_reference_x;       //! X-Cartesian coordinate of the reference point w.r.t. the sensor system
    double m_reference_y;       //! Y-Cartesian coordinate of the reference point w.r.t. the sensor system
    double m_reference_z;       //! Z-Cartesian coordinate of the reference point w.r.t. the sensor system

};

#endif

// End of file
