///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVCALCULATIONLIB - AviBit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Alexander Randeu, a.randeu@avibit.com (Clemens Ender, c.ender@avibit.com)
    \author  QT4-PORT: Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   implements a calculator class for aviation-computations
*/

#if !defined(AVAVIATIONCALCULATOR_H_INCLUDED)
#define AVAVIATIONCALCULATOR_H_INCLUDED

// Qt includes
#include <QPoint>
#include <QPolygon>

// AVLib includes
#include <cmath>
#include <iostream>
#include <avplanarcalculator.h>

#include "avcalculationlib_export.h"

class QPointF;
using namespace std;

// forward delaractions

//! OptsAviationCalculator is a helper class for great circule calculations
/*! OptsAviationCalculator is a class which provides several methods for
    great circle computations, which are common in avation applications.
    For convenience there exist two methods for each computation. One that
    takes arguments in radians and one that takes arguments in decimal
    degrees
 */

class AVCALCULATIONLIB_EXPORT AVAviationCalculator {
public:
    //! construct a OptsAviationCalculator object
    AVAviationCalculator();

    //! destruct a OptsAviationCalculator object
    ~AVAviationCalculator();

    //! calculate the distance in meters between two lat/long-points
    //! given in decimal degrees
    /*! \param p1_lat latitude of point 1
        \param p1_lon longitude of point 1
        \param p2_lat latitude of point 2
        \param p2_lon logitude of point 2
        \param mean_h mean height <m> between p1 and p2,
               default is 0.0 (on earth)
        \return distance in meters
     */
    static double distance (double p1_lat, double p1_lon,
                            double p2_lat, double p2_lon,
                            double mean_h = 0.);

    //! calculate the distance in meters between two lat/long-points
    /*! All arguments must be given in degree.
        \param p1_lat latitude of point 1
        \param p1_lon longitude of point 1
        \param p2_lat latitude of point 2
        \param p2_lon logitude of point 2
        \param mean_h mean height <m> between p1 and p2,
               default is 0.0 (on earth)
        \return distance in meters
     */
    static double distanceRad (double p1_lat, double p1_lon,
                               double p2_lat, double p2_lon,
                               double mean_h = 0.);

    //! calculate the position of a point which is dist meters away from p1
    /*! All arguments must be given in degree.
        \param p1_lat latitude of point 1
        \param p1_lon longitude of point 1
        \param p2_lat latitude of point 2
        \param p2_lon logitude of point 2
        \param dist the distance in meters going from p1 to p2 and which must
               not exceed the total distance from p1 to p2.
        \param d_lat returns the latitude of the point between p1 and p2
               which is 'dist' meters away from p1.
        \param d_lon returns the longitude of the point between p1 and p2
               which is 'dist' meters away from p2.
        \param mean_h mean height <m> between p1 and p2,
               default is 0.0 (on earth)
        \return true if successful, false if dist is greater than the distance
            between p1 and p2.
    */
    static bool pointAtDist (double p1_lat, double p1_lon,
                             double p2_lat, double p2_lon,
                             double dist,
                             double& d_lat, double& d_lon,
                             double mean_h = 0.);

    //! calculate the position of a point which is dist meters away from p1
    /*! All arguments must be given in radians.
        \param p1_lat latitude of point 1
        \param p1_lon longitude of point 1
        \param p2_lat latitude of point 2
        \param p2_lon logitude of point 2
        \param dist the distance in meters going from p1 to p2 and which must
               not exceed the total distance from p1 to p2.
        \param d_lat returns the latitude of the point between p1 and p2
               which is 'dist' meters away from p1.
        \param d_lon returns the longitude of the point between p1 and p2
               which is 'dist' meters away from p2.
        \param mean_h mean height <m> between p1 and p2,
               default is 0.0 (on earth)
        \return true if successful, false if dist is greater than the distance
            between p1 and p2.
    */
    static bool pointAtDistRad (double p1_lat, double p1_lon,
                                double p2_lat, double p2_lon,
                                double dist,
                                double& d_lat, double& d_lon,
                                double mean_h = 0.);

    //! calculate the initial course when flying from point 1 to point 2
    /*! All arguments must be given in degree.
        \param p1_lat latitude of point 1
        \param p1_lon longitude of point 1
        \param p2_lat latitude of point 2
        \param p2_lon logitude of point 2
        \return initial course in degree
     */
    static double course (double p1_lat, double p1_lon,
                          double p2_lat, double p2_lon ) {
        double la1,lo1,la2,lo2;
        la1 = p1_lat * C_DEG2RAD;
        la2 = p2_lat * C_DEG2RAD;
        lo1 = p1_lon * C_DEG2RAD;
        lo2 = p2_lon * C_DEG2RAD;

        return courseRad(la1,lo1,la2,lo2)*C_RAD2DEG;
    };

    //! calculate the initial course when flying from point 1 to point 2
    /*! All arguments must be given in radians.
        \param p1_lat latitude of point 1
        \param p1_lon longitude of point 1
        \param p2_lat latitude of point 2
        \param p2_lon logitude of point 2
        \return initial course in radians
     */
    static double courseRad (double p1_lat,
                             double p1_lon,
                             double p2_lat,
                             double p2_lon ) {
        return aviationMod (atan2(sin(p2_lon - p1_lon) * cos(p2_lat),
                                  cos(p1_lat) * sin(p2_lat) -
                                  sin(p1_lat) * cos(p2_lat) *
                                  cos(p1_lon-p2_lon) ), 2.0 * C_PI);
    };

    //! calculate course deviation from point 1 to point 2 and 3
    /*! This method returns the course deviation in degress between the
        courses from point 1 to point 2 and from point 1 to point 3.
        \param p1_lat latitude of point 1
        \param p1_lon longitude of point 1
        \param p2_lat latitude of point 2
        \param p2_lon logitude of point 2
        \param p2_lat latitude of point 3
        \param p2_lon logitude of point 3
        \return initial course in degree
    */
    static double courseDeviation (double p1_lat, double p1_lon,
                                   double p2_lat, double p2_lon,
                                   double p3_lat, double p3_lon) {
        double c12 = course (p1_lat,p1_lon,p2_lat,p2_lon);
        double c13 = course (p1_lat,p1_lon,p3_lat,p3_lon);
        double c132 = fabs (c12 - c13);
        if ( c132 > 180.0 )
            c132 = ( c12 > c13 ? 360.0 - c12 + c13 : c12 + 360.0 - c13 );
        return c132;
    };

    //! calculate course deviation from point 1 to point 2 and 3
    /*! This method returns the course deviation in radians between the
        courses from point 1 to point 2 and from point 1 to point 3.
        All arguments must be given in radians.
        \param p1_lat latitude of point 1
        \param p1_lon longitude of point 1
        \param p2_lat latitude of point 2
        \param p2_lon logitude of point 2
        \param p2_lat latitude of point 3
        \param p2_lon logitude of point 3
        \return initial course in radians
    */
    static double courseDeviationRad (double p1_lat,
                                      double p1_lon,
                                      double p2_lat,
                                      double p2_lon,
                                      double p3_lat,
                                      double p3_lon) {
        double c12 = courseRad (p1_lat,p1_lon,p2_lat,p2_lon);
        double c13 = courseRad (p1_lat,p1_lon,p3_lat,p3_lon);
        double c132 = fabs (c12 - c13);
        if ( c132 > C_PI )
            c132 = ( c12 > c13 ? 2 * C_PI - c12 + c13 : c12 + 2 * C_PI - c13 );
        return c132;
    };

    //! Added by Alex (no documenation available yet)
    static double courseDeviation (double p1_lat,
                                   double p1_lon,
                                   double c12,
                                   double p3_lat,
                                   double p3_lon) {
        double c132 = courseDeviationRad(p1_lat*C_DEG2RAD,
                                         p1_lon*C_DEG2RAD,
                                         c12*C_DEG2RAD,
                                         p3_lat*C_DEG2RAD,
                                         p3_lon*C_DEG2RAD);
        return c132*C_RAD2DEG;
    };

    //! Added by Alex (no documenation available yet)
    static double courseDeviationRad (double p1_lat,
                                      double p1_lon,
                                      double c12,
                                      double p3_lat,
                                      double p3_lon) {
        double c13 = courseRad (p1_lat,p1_lon,p3_lat,p3_lon);
        double c132 = fabs (c12 - c13);
        if ( c132 > C_PI )
            c132 = ( c12 > c13 ? 2 * C_PI - c12 + c13 : c12 + 2 * C_PI - c13 );
        return c132;
    };

    //! calculate if the heading from p1 intersects the segment p3-p4. If yes the
    //! angle between segment p3-p4 and the heading
    /*! All arguments have to be given in degree. Calculations are performed on the sphere.
        \param p1_lat latitude of point 1
        \param p1_lon longitude of point 1
        \param c12 heading in degress at point p1 (0=north,90=east), 0<=c12<360
        \param p3_lat latitude of point 3 of the segment (start)
        \param p3_lon longitude of point 3 of the segment (start)
        \param p4_lat latitude of point 4 of the segment (end)
        \param p4_lon longitude of point 4 of the segment (end)
        \param deviation if there is an intersection, the smaller angle between the
                         heading c12 and the segment p3-p4 (0<=deviation<=180)
        \return true, if there is an intersection, false if not
        \note the function returns true if p1=p3 or p1=p4
    */
    static bool intersectionHeading(double p1_lat,
                                    double p1_lon,
                                    double c12,
                                    double p3_lat,
                                    double p3_lon,
                                    double p4_lat,
                                    double p4_lon,
                                    double &deviation) {
        if (isEqual(p1_lat, p1_lon, p3_lat, p3_lon)) return true;
        if (isEqual(p1_lat, p1_lon, p4_lat, p4_lon)) return true;
        // p3 and p4 can be the same

        double c13 = course(p1_lat, p1_lon, p3_lat, p3_lon);
        double c14 = course(p1_lat, p1_lon, p4_lat, p4_lon);
        int counter = AVPlanarCalculator::isLeft(c12, c13);
        counter += AVPlanarCalculator::isLeft(c12, c14);
        if (counter != 0) return false;

        // now check if intersection point is in front of p1
        counter = AVPlanarCalculator::isLeft(c13, c12);
        counter += AVPlanarCalculator::isLeft(c13, c14);
        if (counter == 0) return false;

        // now check, if the intersection is in direction of c12 or behind p1
        // ToDo FixxME

        // now get intersection angle
        double c34 = course(p3_lat, p3_lon, p4_lat, p4_lon);
        deviation = c34 - c12;
        if (deviation < 0.0) deviation += 360.0;
        if (deviation > 180.0) deviation = 360.0 - deviation;
        return true;
    }

    static bool intersection2SegmentsSpherical(double p1_lat, double p1_lon,
                                               double p2_lat, double p2_lon,
                                               double p3_lat, double p3_lon,
                                               double p4_lat, double p4_lon,
                                               double &p_int_lat, double &p_int_lon);

    //! calculate if the segment p1-p2 intersects the segment p3-p4
    /*! All arguments have to be given in degree. Calculations are performed on the sphere.
        \param p1_lat latitude of point 1
        \param p1_lon longitude of point 1
        \param p2_lat latitude of point 1
        \param p2_lon longitude of point 1
        \param p3_lat latitude of point 3 of the segment (start)
        \param p3_lon longitude of point 3 of the segment (start)
        \param p4_lat latitude of point 4 of the segment (end)
        \param p4_lon longitude of point 4 of the segment (end)
        \return true, if there is an intersection, false if not
        \note the function returns true if p1=p3 or p1=p4
    */
    static bool intersection2SegmentsSpherical(double p1_lat,
                                               double p1_lon,
                                               double p2_lat,
                                               double p2_lon,
                                               double p3_lat,
                                               double p3_lon,
                                               double p4_lat,
                                               double p4_lon) {
        if (isEqual(p1_lat, p1_lon, p3_lat, p3_lon)) return true;
        if (isEqual(p1_lat, p1_lon, p4_lat, p4_lon)) return true;
        if (isEqual(p2_lat, p2_lon, p3_lat, p3_lon)) return true;
        if (isEqual(p2_lat, p2_lon, p4_lat, p4_lon)) return true;
        bool eq_12 = isEqual(p1_lat, p1_lon, p2_lat, p2_lon);
        bool eq_34 = isEqual(p3_lat, p3_lon, p4_lat, p4_lon);
        if (eq_12 && eq_34) return false; // because not the same points, checked above

        int counter = 0;
        if (!eq_12)
        {
            double c12 = course(p1_lat, p1_lon, p2_lat, p2_lon);
            double c13 = course(p1_lat, p1_lon, p3_lat, p3_lon);
            double c14 = course(p1_lat, p1_lon, p4_lat, p4_lon);
            counter += AVPlanarCalculator::isLeft(c12, c13);
            counter += AVPlanarCalculator::isLeft(c12, c14);
            if (counter != 0) return false;
        }

        if (!eq_34)
        {
            double c34 = course(p3_lat, p3_lon, p4_lat, p4_lon);
            double c31 = course(p3_lat, p3_lon, p1_lat, p1_lon);
            double c32 = course(p3_lat, p3_lon, p2_lat, p2_lon);
            counter += AVPlanarCalculator::isLeft(c34, c31);
            counter += AVPlanarCalculator::isLeft(c34, c32);
            if (counter != 0) return false;
        }

        return true;
    }

    //! checks if the 2 given points have the same position
    /*! All arguments have to be given in degree. Calculations are performed on the sphere.
        \param p1_lat latitude of point 1
        \param p1_lon longitude of point 1
        \param p2_lat latitude of point 1
        \param p2_lon longitude of point 1
        \param p3_lat latitude of point 3 of the segment (start)
        \param p3_lon longitude of point 3 of the segment (start)
        \param p4_lat latitude of point 4 of the segment (end)
        \param p4_lon longitude of point 4 of the segment (end)
        \return true, if there is an intersection, false if not
        \note the function returns true if p1=p3 or p1=p4
    */
    static bool isEqual(double p1_lat, double p1_lon, double p2_lat, double p2_lon) {
        if (fabs(p1_lat - p2_lat) < AVAviationCalculator::C_DOUBLE_EPS &&
            fabs(p1_lon - p2_lon) < AVAviationCalculator::C_DOUBLE_EPS) return true;
        return false;
    }

    //! calculate the normal distance and nadir of a point P3 to a line
    //! between P1 and P2
    /*! All arguments have to be given in degree. The lat/long of the nadir
        is returned in degree.
        \param p1_lat latitude of point 1 of the line
        \param p1_lon longitude of point 1 of the line
        \param p2_lat latitude of point 2 of the line
        \param p2_lon longitude of point 2 of the line
        \param p3_lat latitude of point 3
        \param p3_lon longitude of point 3
        \param ndist return the normal distance from point P3 to the line P1-P2
               in meters
        \param p4_lat latitude of the nadir
        \param p4_lon longitude of the nadir
        \param mean_h mean height <m> between p1 and p2,
               default is 0.0 (on earth)
        \return true if computation was successful, false if nadir is outside
                the line between P1 and P2
    */
    static bool normalDist (double p1_lat, double p1_lon,
                     double p2_lat, double p2_lon,
                     double p3_lat, double p3_lon,
                     double& ndist, double& p4_lat, double& p4_lon,
                     double mean_h = 0.);

    //! calculate the normal distance and nadir of a point P3 to a line
    //! between P1 and P2
    /*! All arguments have to be given in radians. The lat/long of the nadir
        is returned in radians.
        \param p1_lat latitude of point 1 of the line (in radians)
        \param p1_lon longitude of point 1 of the line (in radians)
        \param p2_lat latitude of point 2 of the line (in radians)
        \param p2_lon longitude of point 2 of the line (in radians)
        \param p3_lat latitude of point 3 (in radians)
        \param p3_lon longitude of point 3 (in radians)
        \param ndist return the normal distance from point P3 to the line P1-P2
                     in meters
        \param p4_lat latitude of the nadir (in radians)
        \param p4_lon longitude of the nadir (in radians)
        \param mean_h mean height <m> between p1 and p2,
               default is 0.0 (on earth)
        \return true if computation was successful, false is nadir is outside
                the line between P1 and P2
    */
    static bool normalDistRad (double p1_lat, double p1_lon,
                               double p2_lat, double p2_lon,
                               double p3_lat, double p3_lon,
                               double& ndist, double& p4_lat, double& p4_lon,
                               double mean_h = 0.);

    //! Calculate the heading (north = 0 deg., east = 90 deg.) of the axis
    //! of symmetry at p2 between p1 and p3. The heading always points to the
    //! side with the smaller angle (p2 to p1,p3).
    /*!
        \param p1_lat latitude of point 1 (in deg.)
        \param p1_lon longitude of point 1 (in deg)
        \param p2_lat latitude of point 2 (in deg)
        \param p2_lon longitude of point 2 (in deg)
        \param p3_lat latitude of point 3 (in deg)
        \param p3_lon longitude of point 3 (in deg)
        \return heading of axis of symmetry at p2 (in deg.)
    */
    static double symAxis(double p1_lat, double p1_lon,
                          double p2_lat, double p2_lon,
                          double p3_lat, double p3_lon)
    {
        double head;
        head = symAxisRad(p1_lat*C_DEG2RAD,p1_lon*C_DEG2RAD,
                          p2_lat*C_DEG2RAD,p2_lon*C_DEG2RAD,
                          p3_lat*C_DEG2RAD,p3_lon*C_DEG2RAD);
        return head*C_RAD2DEG;
    }

    //! Calculate the heading (north = 0 deg., east = 90 deg.) of the axis
    //! of symmetry at p2 between p1 and p3. The heading always points to the
    //! side, thus every course c from p2 to a point
    //! heading < c < heading + 180 deg lies on the side to wp1 and
    //! heading - 180 deg < c < heading lies on the side to wp3.
    /*!
        \param p1_lat latitude of point 1 (in radians)
        \param p1_lon longitude of point 1 (in radians)
        \param p2_lat latitude of point 2 (in radians)
        \param p2_lon longitude of point 2 (in radians)
        \param p3_lat latitude of point 3 (in radians)
        \param p3_lon longitude of point 3 (in radians)
        \return heading of axis of symmetry at p2 (in radians)
    */
    static double symAxisRad(double p1_lat, double p1_lon,
                             double p2_lat, double p2_lon,
                             double p3_lat, double p3_lon)
    {
        double head, c21, c23;
        c21 = courseRad(p2_lat, p2_lon, p1_lat, p1_lon);
        c23 = courseRad(p2_lat, p2_lon, p3_lat, p3_lon);

        head = (c21 + c23)/2.0;

        if(fabs(c21 - c23) > C_PI)
            head = aviationMod(head + C_PI, 2.0 * C_PI);

        return head;
    }

    /*! calculates the center of turn at FL=0
        \param lat1 latitude <deg> of start point
        \param lon1 longitude <deg> of start point
        \param course1 course <deg> at start point
                       (0=north,90=east, 0<=course<360)
        \param lat2 latitude <deg> of target point
        \param lon2 longitue <deg> of target point
        \param turnradius turn radius <m> at FL=0
        \param latc latitude <deg> of turn center
        \param lonc longitude of turn center
        \param inc_f incrementation factor
                     (-1 for left turns, +1 for right turns)
        \note azimuth_c_s from center to startpoint =
              course1 - inc_f*90deg.. additional points of turn are found
              by azimuth_c_s + delta*inc_f
    */
    static void calcTurnCenter(double lat1, double lon1,
                               double cou,
                               double lat2, double lon2,
                               double turnradius,
                               double &latc, double &lonc, double &inc_f);

    //! adjusts an angle to 0deg <= angle < 360deg
    /*! \param head angle <deg>
        \return angle with 0deg <= angle < 360deg
     */
    static double adjustHeading(double head)
    {
        return aviationMod(head, 360.0);
    }

    //! adjusts an angle to 0 <= angle < 2 PI
    /*! \param head angle <rad>
        \return angle with 0 <= angle < 2 PI
     */
    static double adjustHeadingRad(double head)
    {
        return aviationMod(head, 2 * C_PI);
    }

   //! Lat/lon given course and distance
    /*! \param lat1 latitude <deg> of start point
        \param lon1 longitude <deg> of start point
        \param dd distance <m> to target point
        \param tcd course <deg> to target point (0=north, 90=east)
        \param lat2 latitude of target <deg>
        \param lon2 longitude of target <deg>
        \param mean_h distance d is given for height mean_h <m>
               default mean_h=0.0
     */
    static void pointAtCourseDist(double lat1d, double lon1d,
                                  double dd, double tcd,
                                  double &lat2, double &lon2,
                                  double mean_h = 0.0);

    //! Lat/lon given course and distance
     /*! \param lat1 latitude <rad> of start point
         \param lon1 longitude <rad> of start point
         \param dd distance <m> to target point
         \param tcd course <rad> to target point (0=north, 90=east)
         \param lat2 latitude of target <rad>
         \param lon2 longitude of target <rad>
         \param mean_h distance d is given for height mean_h <m>
                default mean_h=0.0
      */
    static void pointAtCourseDistRad(const double lat1, const double lon1,
                                     const double dd, const double tc,
                                     double &lat2, double &lon2,
                                     const double mean_h = 0.0);

    static const double C_PI;
    static const double C_RE;
    static const double C_DEG2RAD;
    static const double C_RAD2DEG;
    static const double C_INVALID_ALT;

    //! Returns the flightlevel <100 ft> from an QNH corrected altitude <ft>
    //! for the given QNH <hpascal or mbar>.
    //! "-10000.0" will be returned in case of an error.
    static double getFlightlevelFromQNHCorrectedAltitude(const double& altitude,
                                                         const double& qnh);

    //! converts lat/lon coordinates <deg> to x/y/z coordinates assuming an earth radius of 1.0
    static void convertLatLonToXYZ(double lat, double lon, double &x, double &y, double &z)
    {
        convertLatLonToXYZRad(lat * C_DEG2RAD, lon * C_DEG2RAD, x, y, z);
    }

    //! converts lat/lon coordinates <rad> to x/y/z coordinates assuming an earth radius of 1.0
    static void convertLatLonToXYZRad(double lat, double lon, double &x, double &y, double &z)
    {
        x = cos(lat)*cos(lon);
        y = cos(lat)*sin(lon);
        z = sin(lat);
    }

    //! calculate if the distance between two moving targets is below a given threshold
    //! at any time.
    /*! \param t1x x coordinate of target 1 <m>
        \param t1y y coordinate of target 1 <m>
        \param vt1x velocity in x direction of target 1 <m/s>
        \param vt1y velocity in y direction of target 1 <m/s>
        \param t2x x coordinate of target 2 <m>
        \param t2y y coordinate of target 2 <m>
        \param vt2x velocity in x direction of target 2 <m/s>
        \param vt2y velocity in y direction of target 2 <m/s>
        \param dist_threshold threshol to check <m>
        \param has_solution set to true, if the distance between the two targets gets below
                                the given threshold. if const_dist is false, t1 and t2
                                contain the start and end timestamp where the targets
                                are nearer than the threshold.
        \param const_dist is set to true when both targets have the same speed and direction
                          and the distance is below dist_threshold
        \param t1 start time in seconds (relative to current positions) after which the
                  targets are nearer than dist_threshold. Note: may be negative. Only set
                  if has_solution=true and const_dist=false.
        \param t2 end time in seconds (relative to current positions) after which the
                  targets are nearer than dist_threshold. Note: may be negative. Only set
                  if has_solution=true and const_dist=false.
        \return true if the calculation was successful. A return value of true and
                has_solution=false means, that the targets are never nearer to each other
                than the given threshold. A return value of true and has_solution=true and
                const_dist=true means, one target is following the other target within
                dist_threshold (same direction). A return value of true and
                has_solution=true and const_dist=false means that the targets are nearer
                to each other than the given threshold between t1 and t2.
                A return value of false means, there was an error.
     */
    static bool minDistMovingTargets(double t1x, double t1y, double vt1x, double vt1y,
                                     double t2x, double t2y, double vt2x, double vt2y,
                                     double dist_threshold,
                                     bool &has_solution, bool &const_dist,
                                     double &t1, double &t2);


    //! Returns the QNH corrected altitude <ft> for the given QNH <hpascal or mbar>
    //! and flightlevel <100 ft>. "-10000.0" (=C_INVALID_ALT) will be returned in case of an error.
    static double getQNHCorrectedAltitudeFromFlightlevel(const double& flightlevel,
                                                         const double& qnh);

private:
    //! special modulo function required for OptsAviationCalulator calculations
    static double aviationMod(double y, double x) {
        double mod = fmod(y, x);
        if (mod < 0.0) mod += x;
        return mod;
    }

    static const double C_DOUBLE_EPS;
};

#endif

// End of file
