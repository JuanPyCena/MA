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
    \author  Tobias Karns, t.karns@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   implements a calculator class for planar-computations
*/

#if !defined(AVPLANARCALCULATOR_H_INCLUDED)
#define AVPLANARCALCULATOR_H_INCLUDED

#include <QVector>
#include <cmath>
#include <iostream>


// Qt includes
#include <QPoint>
#include <QPolygon>
#include <QPointF>

#include "avcalculationlib_export.h"

using namespace std;

class AVCALCULATIONLIB_EXPORT AVPlanarCalculator
{
public:
    //! construct a OptsAviationCalculator object
    AVPlanarCalculator();

    //! destruct a OptsAviationCalculator object
    ~AVPlanarCalculator();

    //! calculate the distance in meters between two QPoints
    static double getDistanceOfPoints(const QPoint& p1,
                                      const QPoint& p2);

    //! calculate the distance in meters between two QVector2D, QVector3D, etc.
    template<class VECTORTYPE>
    static double getDistanceOfQVectors(const VECTORTYPE& p1,
                                        const VECTORTYPE& p2);

    //! overloaded member function
    //! calculate the distance in meters between two AVPoints
    static double getDistanceOfPoints(const QPointF& p1,
                                      const QPointF& p2);

    //! returns the length of the vector
    static double getVectorLength(const QPointF& vector);

    //! returns the heading of the vector in degrees (North=0, East=90,...)
    //! ATTENTION: Assumes x decreases EASTWARDS, y decreases SOUTHWARDS.
    static double getVectorHeading(const QPointF& vector);

    //! returns the unit vector for heading in degrees
    //! ATTENTION: Assumes x decreases EASTWARDS, y decreases SOUTHWARDS.
    static QPointF getUnitVectorForHeading(double heading);

    //! angle returns the angle between the two vectors in degree
    //! the angle in <= 180 degree
    //! returns false if one of the vectors is null
    static bool getAngleBetweenVectors(const QPointF& vector_a,
                                       const QPointF& vector_b,
                                       double& angle);

    //! returns the angle difference between the two angles
    //! (0<= result <=180)
    static double getAngleDifference(double angle1, double angle2);

    //! calculates the change from angle1 to angle 2 (-180 < result <= 180)
    static double getAngleDelta(double angle1, double angle2);

    //! Calculates a continuous new heading based on old heading
    //! It calculates a heading with any value (even <0 or >360), that is within 180 of old_heading
    //! Both old_heading and new_heading may have any values
    static double calculateContinousHeading(double new_heading, double old_heading);

    //! overloaded member function
    //! returns the length of the vector
    static double getVectorLength(const QPoint& vector);

    /*! checks, if the two line segments s1(p11,p12) and s2(p21,p22) have
        an intersection point.
        \param p11 first point of segment 1 (cartesian coordinates)
        \param p12 second point of segment 1 (cartesian coordinates)
        \param p21 first point of segment 2 (cartesian coordinates)
        \param p22 second point of segment 2 (cartesian coordinates)
        \param is if there is an intersection point, the coordinates of it  (cartesian coordinates)
        \param scalep11 the intersection point is at scalep11*dist(p11,p12) from p11
        \return true, if there is an intersection (see is), false if not
        \note An intersection is only detected, if the 2 segments are not
              parallel or colinear.
     */
    static bool intersect2Segs(const QPoint &p11, const QPoint &p12,
                               const QPoint &p21, const QPoint &p22,
                               QPoint &is, double &scalep11);


    /*! overloaded member function
        checks, if the two line segments s1(p11,p12) and s2(p21,p22) have
        an intersection point.
        \param p11 first point of segment 1 (cartesian coordinates)
        \param p12 second point of segment 1 (cartesian coordinates)
        \param p21 first point of segment 2 (cartesian coordinates)
        \param p22 second point of segment 2 (cartesian coordinates)
        \param is if there is an intersection point, the coordinates of it  (cartesian coordinates)
        \param scalep11 the intersection point is at scalep11*dist(p11,p12) from p11
        \return true, if there is an intersection (see is), false if not
        \note An intersection is only detected, if the 2 segments are not
              parallel or colinear.
     */
    static bool intersect2Segs(const QPointF &p11, const QPointF &p12,
                               const QPointF &p21, const QPointF &p22,
                               QPointF &is, double &scalep11);

      /*! checks, if the two line segments s1(p11,p12) and s2(p21,p22) have
        an intersection point.
        \param p11 first point of segment 1 (cartesian coordinates)
        \param p12 second point of segment 1 (cartesian coordinates)
        \param p21 first point of segment 2 (cartesian coordinates)
        \param p22 second point of segment 2 (cartesian coordinates)
        \note  The function returns also true, if the two
        segments are parallel but overlapping.
     */
    static bool twoSegsHaveIntersection(const QPoint &p11, const QPoint &p12,
                                        const QPoint &p21, const QPoint &p22);


      /*! checks, if the two line segments s1(p11,p12) and s2(p21,p22) have
        an intersection point, or have almost the same direction and a very
        low distance to each other
        \param p11 first point of segment 1 (cartesian coordinates)
        \param p12 second point of segment 1 (cartesian coordinates)
        \param p21 first point of segment 2 (cartesian coordinates)
        \param p22 second point of segment 2 (cartesian coordinates)
        \note  The function returns also true, if the two
        segments are parallel but overlapping.
     */
    static bool twoSegsHaveAlmostIntersection(const QPointF &p11, const QPointF &p12,
                                              const QPointF &p21, const QPointF &p22);

    //! returns the minimal distance of a point to a segment
    static double getMinDistanceOfPointToSegment(const QPoint &point,
                                                 const QPoint &p1,const QPoint &p2);

    //! returns the minimal distance of a point to a segment
    static double getMinDistanceOfPointToSegment(const QPointF &point,
                                                 const QPointF &p1,const QPointF &p2);

    //! returns the minimum distance of the two segments
    //! this function only works if the two segments have no intersection
    static double getMinDistanceOfSegments(const QPointF &p11, const QPointF &p12,
                                           const QPointF &p21, const QPointF &p22);

    /*! checks, if the line segment s1(s11,s12) and the line through l1(l11,l12) have
        an intersection point.
        \param s11 first point of segment 1 (cartesian coordinates)
        \param s12 second point of segment 1 (cartesian coordinates)
        \param l11 first point of line 1 (cartesian coordinates)
        \param l12 second point of line 1 (cartesian coordinates)
        \param is if there is an intersection point, the coordinates of it  (cartesian coordinates)
        \param scalep11 the intersection point is at scalep11*dist(p11,p12) from p11
        \param scalel11 the intersection point is at scalel11*dist(l11,l12) from l11
        \return true, if there is an intersection (see is), false if not
        \note An intersection is only detected, if the segment and line are not
              parallel or colinear.
     */
    static bool intersectSegLine(const QPoint &s11, const QPoint &s12,
                                 const QPoint &l11, const QPoint &l12,
                                 QPoint &is, double &scalep11, double &scalel11);

    /*! overloaded member function
        checks, if the line segment s1(s11,s12) and the line through l1(l11,l12) have
        an intersection point.
        \param s11 first point of segment 1 (cartesian coordinates)
        \param s12 second point of segment 1 (cartesian coordinates)
        \param l11 first point of line 1 (cartesian coordinates)
        \param l12 second point of line 1 (cartesian coordinates)
        \param is if there is an intersection point, the coordinates of it  (cartesian coordinates)
        \param scalep11 the intersection point is at scalep11*dist(p11,p12) from p11
        \param scalel11 the intersection point is at scalel11*dist(l11,l12) from l11
        \return true, if there is an intersection (see is), false if not
        \note An intersection is only detected, if the segment and line are not
              parallel or colinear.
     */
    static bool intersectSegLine(const QPointF &s11, const QPointF &s12,
                                 const QPointF &l11, const QPointF &l12,
                                 QPointF &is, double &scalep11, double &scalel11);



    /*! checks, if the line l1(l11,l12) and the line l2(l21,l22) have
    an intersection point.
    \param l11 first point of line 1 (cartesian coordinates)
    \param l12 second point of line 1 (cartesian coordinates)
    \param l21 first point of line 2 (cartesian coordinates)
    \param l22 second point of line 2 (cartesian coordinates)
    \param intersection_point is the intersection_point rounded to QPoint int
    \return true, if there is an intersection (see is), false if not
    \note An intersection is only detected, if the segment and line are not
            parallel or colinear.
    */
    static bool intersectLineWithLine(const QPointF &s11, const QPointF &s12,
                                      const QPointF &l11, const QPointF &l12,
                                      QPointF* intersection_point = 0);

    /*! intersects the line perpendicular to line segment s1(s11,s12) and through the point p
        with the line segment s1
        \param s11 first point of segment 1 (cartesian coordinates)
        \param s12 second point of segment 1 (cartesian coordinates)
        \param p point through which the perpendicular line goes (cartesian coordinates)
        \param is if there is an intersection point, the coordinates of it  (cartesian coordinates)
        \param scalep11 the intersection point is at scalep11*dist(s11,s12) from s11
        \return true, if there is an intersection (see is), false if not
     */
    static bool intersectSegPerpendicularThroughPoint(const QPoint &s11, const QPoint &s12,
                                                      const QPoint &p,
                                                      QPoint &is, double &scalep11);


    /*! overloaded member function
        intersects the line perpendicular to line segment s1(s11,s12) and through the point p
        with the line segment s1
        \param s11 first point of segment 1 (cartesian coordinates)
        \param s12 second point of segment 1 (cartesian coordinates)
        \param p point through which the perpendicular line goes (cartesian coordinates)
        \param is if there is an intersection point, the coordinates of it  (cartesian coordinates)
        \param scalep11 the intersection point is at scalep11*dist(s11,s12) from s11
        \return true, if there is an intersection (see is), false if not
     */
    static bool intersectSegPerpendicularThroughPoint(const QPointF &s11, const QPointF &s12,
                                                      const QPointF &p,
                                                      QPointF &is, double &scalep11);

    /*! intersects the line perpendicular to line l1(l11,l12) and through the point p
        with the line segment s1
        \param l11 first point of line 1 (cartesian coordinates)
        \param l12 second point of line 1 (cartesian coordinates)
        \param p point through which the perpendicular line goes (cartesian coordinates)
        \param s11 first point of segment 1 (cartesian coordinates)
        \param s12 second point of segment 1 (cartesian coordinates)
        \param is if there is an intersection point, the coordinates of it  (cartesian coordinates)
        \param scales11 the intersection point is at scales11*dist(s11,s12) from s11
        \return true, if there is an intersection (see is), false if not
     */
    static bool intersectSegPerpendicularThroughPoint(const QPoint &l11, const QPoint &l12,
                                                      const QPoint &p,
                                                      const QPoint &s11, const QPoint &s12,
                                                      QPoint &is, double &scales11);

    /*! intersects the point p perpendicular to line l1(l11,l12).
        The points l11 and l12 have to be different!
    \param l11 first point of line 1 (cartesian coordinates)
    \param l12 second point of line 1 (cartesian coordinates)
    \param p point through which the perpendicular line goes (cartesian coordinates)
    \param is if there is an intersection point, the coordinates of it  (cartesian coordinates)
    \param scales11 the intersection point is at scales11*dist(s11,s12) from s11
    \return true, if there is an intersection (see is), false if not
     */
    static bool intersectLinePerpendicularThroughPoint(const QPointF &l11,
                                                       const QPointF &l12,
                                                       const QPointF &p,
                                                       QPointF &is);


    /*! intersects a line given by 2 points with a circle
        an intersection point.
        \param linex1 x coordinate of point 1 defining the line (cartesian coordinates)
        \param liney1 y coordinate of point 1 defining the line (cartesian coordinates)
        \param linex2 x coordinate of point 2 defining the line (cartesian coordinates)
        \param liney2 y coordinate of point 2 defining the line (cartesian coordinates)
        \param centerx x coordinate of circle center point (cartesian coordinates)
        \param centery y coordinate of circle center point (cartesian coordinates)
        \param radius raduis of circle
        \param intersectx1 x coordinate of intersection point 1 (cartesian coordinates)
        \param intersecty1 y coordinate of intersection point 1 (cartesian coordinates)
        \param intersectx2 x coordinate of intersection point 2 (cartesian coordinates)
        \param intersecty2 y coordinate of intersection point 2 (cartesian coordinates)
        \return true, if there is an intersection, false if not
        \note if the 2 points defining the line are the same, there is anly an intersection
              when the 2 line points lie on the circle.
     */
    static bool intersectLineCircle(double linex1, double liney1,
                                    double linex2, double liney2,
                                    double centerx, double centery, double radius,
                                    double &intersectx1, double &intersecty1,
                                    double &intersectx2, double &intersecty2);

    /*! calculates the normal distance from a point to a line
        \param pointx x coordinate of point (cartesian coordinates)
        \param pointy y coordinate of point (cartesian coordinates)
        \param linex1 x coordinate of point 1 defining the line (cartesian coordinates)
        \param liney1 y coordinate of point 1 defining the line (cartesian coordinates)
        \param linex2 x coordinate of point 2 defining the line (cartesian coordinates)
        \param liney2 y coordinate of point 2 defining the line (cartesian coordinates)
        \note if the 2 points defining the line are the same, the returned value is the
              distnace between the point and the 2 equal line points
     */
    static double normalDistancePointToLine(double pointx, double pointy,
                                            double linex1, double liney1,
                                            double linex2, double liney2);

    //! test if the point p2 is right or left from the line from p0 to p1
    /*! \return >0 for p2 left of the line through p0 and p1
       =0 for p2 on the line
       <0 for p2 right of the line
     */
    static double isLeft(const QPoint &p0, const QPoint &p1, const QPoint &p2);

    //! test if the point p2 is right or left from the line from p0 to p1
    /*! \return >0 for p2 left of the line through p0 and p1
       =0 for p2 on the line
       <0 for p2 right of the line
     */
    static double isLeft(const QPointF &p0, const QPointF &p1, const QPointF &p2);

    //! test if the heading c13 in degrees is left from heading c12 in degrees
    /*! \return >0 for c13 left (c12 - 180 < c13 < c12) of c12
       =0 for c12 is co-linear (c13 = c12 or c13 = c12 +/- 180)
       <0 for c13 right (c12 < c13 < c12 + 180) of c12
        \note c12 and c13 angles must be between 0<=c<360
     */
    static int isLeft(double c12, double c13);

    //! Determines if a given point lies within the polygon
    /*! if the point lies on an edge, it is seen to be outside
     */
    static bool contains(const QPolygon &points, const QPoint &p);

    //! Determines if a given point lies within the polygon
    /*! if the point lies on an edge, it is seen to be outside
     */
    static bool contains(const QVector<QPoint> &polygon, const QPoint &p);

    //! Determines if a given point lies within the polygon
    /*! if the point lies on an edge, it is seen to be outside
     */
    static bool contains(const QVector<QPointF> &polygon, const QPointF &p);

    //! Returns true if polygon is either simple or weakly simple.
    /*! Algorithm has O(n^2) complexity and thus is not optimal (O(n*log(n)))!
     */
    static bool isSimple(const QPolygon &points);

private:
    static const double C_DOUBLE_EPS;
};


template<class VECTORTYPE>
double AVPlanarCalculator::getDistanceOfQVectors(const VECTORTYPE& p1,
                                                 const VECTORTYPE& p2)
{
    return (p2-p1).length();
}

#endif

// End of file
