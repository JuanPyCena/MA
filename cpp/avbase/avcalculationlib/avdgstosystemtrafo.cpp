///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVCALCULATIONLIB - AviBit Calculation Library
//
/////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Madalina Hodorog, m.hodorog@avibit.com
    \brief   Implements class for Cartesian to WGS84 coordinate transformations
*/


#include "avmisc.h" //atan2 included here
#include "avdgstosystemtrafo.h"
#include "avlog.h"
#include "avsystemtrafo.h"
#include "avsciconst.h" //convertrad2deg included here

///////////////////////////////////////////////////////////////////////////////

AVDGSToSystemTrafo::AVDGSToSystemTrafo(double laser_lat,
                                       double laser_long,
                                       double laser_h,
                                       double reference_lat,
                                       double reference_long,
                                       double reference_h,
                                       double reference_x,
                                       double reference_y,
                                       double reference_z)
  : m_laser_lat(laser_lat),
    m_laser_long(laser_long),
    m_laser_h(laser_h),
    m_reference_lat(reference_lat),
    m_reference_long(reference_long),
    m_reference_h(reference_h),
    m_reference_x(reference_z),     // the sensor system is rotated w.r.t. the system trafo
    m_reference_y(-reference_x),    // the sensor system is rotated w.r.t. the system trafo
    m_reference_z(reference_y)      // the sensor system is rotated w.r.t. the system trafo
{
    m_laser_trafo = new AVSSRRadarCartTrafo("DGSSystemTrafo");
    AVASSERT(m_laser_trafo != 0);

    AVASSERT(m_laser_trafo->setRadarPos(m_laser_lat, m_laser_long, m_laser_h));

    m_system_trafo= new AVSystemTrafo;
    AVASSERT(m_system_trafo != 0);
}

///////////////////////////////////////////////////////////////////////////////

AVDGSToSystemTrafo::~AVDGSToSystemTrafo()
{
    delete m_laser_trafo;
    delete m_system_trafo;
}
///////////////////////////////////////////////////////////////////////////////

double AVDGSToSystemTrafo::getRotationAngleFromOrdinate() const
{
    double rotation_angle_ordinate = 0.0;

    rotation_angle_ordinate = atan2(m_reference_x, m_reference_y);
    if (rotation_angle_ordinate < 0)
        rotation_angle_ordinate += 2*AVAviationCalculator::C_PI;
    return rotation_angle_ordinate;
}

///////////////////////////////////////////////////////////////////////////////

double AVDGSToSystemTrafo::getRotationAngleFromNorth() const
{
    double rotation_angle_north = 0.0;

    rotation_angle_north = AVAviationCalculator::courseRad(AVSciConst::convdegTorad(m_laser_lat),
                                                           AVSciConst::convdegTorad(m_laser_long),
                                                           AVSciConst::convdegTorad(m_reference_lat),
                                                           AVSciConst::convdegTorad(m_reference_long));

    return rotation_angle_north;
}
///////////////////////////////////////////////////////////////////////////////

void AVDGSToSystemTrafo::getRotatedCoordinates(double rotation_angle_radians,
                                               double x,
                                               double y,
                                               double &x_rotated,
                                               double &y_rotated) const
{
    //compute the rotated coordinates counterclockwise
    x_rotated = x * cos(rotation_angle_radians) - y * sin(rotation_angle_radians);
    y_rotated = x * sin(rotation_angle_radians) + y * cos(rotation_angle_radians);
}
///////////////////////////////////////////////////////////////////////////////

void AVDGSToSystemTrafo::xyz2latlonh(double target_x,
                                     double target_y,
                                     double target_z,
                                     double &target_lat,
                                     double &target_long,
                                     double &target_h)
{
    double rotation_angle_ordinate = 0.0;
    double rotation_angle_north = 0.0;
    double target_x_rotated = 0.0;
    double target_y_rotated = 0.0;

    double swap_x = target_x;
    double swap_y = target_y;
    double swap_z = target_z;
    target_x = swap_z;         // the sensor system is rotated w.r.t. the system trafo
    target_y = -swap_x;        // the sensor system is rotated w.r.t. the system trafo
    target_z = swap_y;         // the sensor system is rotated w.r.t. the system trafo

    rotation_angle_ordinate = getRotationAngleFromOrdinate();
    rotation_angle_north = getRotationAngleFromNorth();

    double rotation_angle = rotation_angle_ordinate - rotation_angle_north;
    AVLogDebug<<"AVDGSToSystemTrafo::xyz2latlonh: Rotation angle in degrees from ordinate = "
              << AVSciConst::convradTodeg(rotation_angle_ordinate);
    AVLogDebug<<"AVDGSToSystemTrafo::xyz2latlonh: Rotation angle in degrees from north = "
              << AVSciConst::convradTodeg(rotation_angle_north);
    AVLogDebug<<"AVDGSToSystemTrafo::xyz2latlonh: Rotation angle in degrees = "
              << AVSciConst::convradTodeg(rotation_angle);
    getRotatedCoordinates(rotation_angle, target_x, target_y, target_x_rotated, target_y_rotated);
    m_laser_trafo->radarCart2Geod(target_x_rotated,
                                  target_y_rotated,
                                  target_z,
                                  target_lat,
                                  target_long,
                                  target_h);
}

///////////////////////////////////////////////////////////////////////////////

void AVDGSToSystemTrafo::xyz2xySystemTrafo(double target_x,
                                           double target_y,
                                           double target_z,
                                           double &target_x_av,
                                           double &target_y_av)
{
    double target_lat=0.0;
    double target_long=0.0;
    double target_h=0.0;

    xyz2latlonh(target_x, target_y,target_z, target_lat,target_long,target_h);
    m_system_trafo->latlong2xy(target_lat, target_long, target_x_av, target_y_av);
}

///////////////////////////////////////////////////////////////////////////////

void AVDGSToSystemTrafo::latlong2xySystemTrafo(double target_lat,
                                               double target_long,
                                               double& target_x_av,
                                               double& target_y_av)
{
    m_system_trafo->latlong2xy(target_lat, target_long, target_x_av, target_y_av);
}

// End of file
