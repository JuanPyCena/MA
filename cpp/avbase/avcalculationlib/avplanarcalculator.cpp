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
    \author  Alexander Randeu, a.randeu@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Defines a helper class for planar computation

    This class provides planar computations
*/


// Qt includes
#include <QPoint>
#include <QPointF>
#include <qmath.h>

// AVLib includes
#include "avlog.h"


// local includes

#include "avplanarcalculator.h"
#include "avsciconst.h"


///////////////////////////////////////////////////////////////////////////////

const double AVPlanarCalculator::C_DOUBLE_EPS = 1.0e-20;

///////////////////////////////////////////////////////////////////////////////

AVPlanarCalculator::AVPlanarCalculator()
{
}

///////////////////////////////////////////////////////////////////////////////

AVPlanarCalculator::~AVPlanarCalculator()
{
}

///////////////////////////////////////////////////////////////////////////////

bool AVPlanarCalculator::getAngleBetweenVectors(const QPointF& vector_a,
                                                const QPointF& vector_b,
                                                double& angle)
{
    if (vector_a.x() == 0 && vector_a.y() == 0)
        return false;
    if (vector_b.x() == 0 && vector_b.y() == 0)
        return false;
    double product = vector_a.x()*vector_b.x() + vector_a.y()*vector_b.y();
    double alpha_temp = acos(product/(getVectorLength(vector_a)*getVectorLength(vector_b)));
    angle = AVSciConst::normaliseDeg (AVSciConst::convradTodeg(alpha_temp));
    return true;
}

///////////////////////////////////////////////////////////////////////////////

double AVPlanarCalculator::getAngleDifference(double angle1,double angle2)
{
    double diff=fabs(AVSciConst::normaliseDeg(angle2)-AVSciConst::normaliseDeg(angle1));

    if (diff>180)
        diff=360-diff;

    return diff;
}

///////////////////////////////////////////////////////////////////////////////

double AVPlanarCalculator::getAngleDelta(double angle1, double angle2)
{
    double delta = AVSciConst::normaliseDeg(angle2) - AVSciConst::normaliseDeg(angle1);

    if(delta > 180)
        delta -= 360;
    if(delta <= -180)
        delta += 360;

    return delta;
}

///////////////////////////////////////////////////////////////////////////////

double AVPlanarCalculator::calculateContinousHeading(double new_heading, double old_heading)
{
    double continuous_heading = new_heading;

    double dheading = new_heading-old_heading;
    if (fabs(dheading) > 180.0)
    {
        continuous_heading = new_heading - 360.0 * floor(dheading/360.0 + 0.5);
        //AVLogger->Write(LOG_INFO,"head2cont : %.2f to %.2f",new_heading,continuous_heading);
    }

    return (continuous_heading);
}

///////////////////////////////////////////////////////////////////////////////

double AVPlanarCalculator::getVectorHeading(const QPointF& vector)
{
    AVASSERT(!(vector.x() ==0 && vector.y()==0));

    double rad = atan2(-vector.y(), vector.x());

    return AVSciConst::normaliseDeg( AVSciConst::convdegToheading (AVSciConst::convradTodeg(rad)));
}

///////////////////////////////////////////////////////////////////////////////

QPointF AVPlanarCalculator::getUnitVectorForHeading(double heading)
{
    AVASSERT(0 <= heading && heading < 360);

    double rad = AVSciConst::convdegTorad(AVSciConst::convheadingTodeg(heading));

    QPointF vector;
    vector.setX(qCos(rad));
    vector.setY(-qSin(rad));
    return vector;
}

///////////////////////////////////////////////////////////////////////////////

double AVPlanarCalculator::getDistanceOfPoints(const QPoint& p1,
                                               const QPoint& p2)
{
    QPoint diff_vector = p1 - p2;
    double dist =  sqrt(double(diff_vector.x())*double(diff_vector.x()) +
                double(diff_vector.y())*double(diff_vector.y()));
    return dist;
}

/////////////////////////////////////////////////////////////////////////////

double AVPlanarCalculator::getDistanceOfPoints(const QPointF& p1,
                                               const QPointF& p2)
{
    QPointF diff_vector = p1 - p2;
    double dist =  sqrt(double(diff_vector.x())*double(diff_vector.x()) +
                double(diff_vector.y())*double(diff_vector.y()));
    return dist;
}

/////////////////////////////////////////////////////////////////////////////

double AVPlanarCalculator::getVectorLength(const QPointF& vector)
{
    return (sqrt(vector.x()*vector.x() + vector.y()*vector.y()));
}

/////////////////////////////////////////////////////////////////////////////

double AVPlanarCalculator::getVectorLength(const QPoint& vector)
{
    return (sqrt(double(vector.x())*double(vector.x()) +
            double(vector.y())*double(vector.y())));
}

/////////////////////////////////////////////////////////////////////////////

bool AVPlanarCalculator::intersect2Segs(const QPoint &p11, const QPoint &p12,
                                        const QPoint &p21, const QPoint &p22,
                                        QPoint &is, double &scalep11)
{
    //will only return true, if there is an intersection and the 2 segments
    //are not parallel or colinear
    QPoint u = p12 - p11;
    QPoint v = p22 - p21;
    QPoint w = p11 - p21;

    double ux = u.x();
    double uy = u.y();
    double vx = v.x();
    double vy = v.y();
    double wx = w.x();
    double wy = w.y();

    double d = ux*vy - uy*vx;

    if(fabs(d) < C_DOUBLE_EPS) //s1 and s2 are parallel or colinear
        return false;

    double s = (vx*wy - vy*wx)/d;
    if(s < 0.0 || s > 1.0) return false;    //no intersection with s1

    double t = (ux*wy - uy*wx)/d;
    if(t < 0.0 || t > 1.0) return false;    //no intersection with s2

    double sux = s*ux;
    double suy = s*uy;
    is = p11 + QPoint(static_cast<int>(sux), static_cast<int>(suy));

    scalep11 = s;

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool AVPlanarCalculator::intersect2Segs(const QPointF &p11, const QPointF &p12,
                                        const QPointF &p21, const QPointF &p22,
                                        QPointF &is, double &scalep11)
{
    //will only return true, if there is an intersection and the 2 segments
    //are not parallel or colinear
    QPointF u = p12 - p11;
    QPointF v = p22 - p21;
    QPointF w = p11 - p21;

    double ux = u.x();
    double uy = u.y();
    double vx = v.x();
    double vy = v.y();
    double wx = w.x();
    double wy = w.y();

    double d = ux*vy - uy*vx;

    if(fabs(d) < C_DOUBLE_EPS) //s1 and s2 are parallel or colinear
        return false;

    double s = (vx*wy - vy*wx)/d;
    if(s < 0.0 || s > 1.0) return false;    //no intersection with s1

    double t = (ux*wy - uy*wx)/d;
    if(t < 0.0 || t > 1.0) return false;    //no intersection with s2

    is = p11 + u*s;

    scalep11 = s;

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool AVPlanarCalculator::twoSegsHaveIntersection(const QPoint &p11, const QPoint &p12,
                                                 const QPoint &p21, const QPoint &p22)
{
    QPoint u = p12 - p11;
    QPoint v = p22 - p21;
    QPoint w = p11 - p21;

    double ux = u.x();
    double uy = u.y();
    double vx = v.x();
    double vy = v.y();
    double wx = w.x();
    double wy = w.y();

    double d = ux*vy - uy*vx;

    if(fabs(d) < C_DOUBLE_EPS) //s1 and s2 are parallel or colinear
    {
        QPoint point_on_line;
        double dummy_double;

        if (intersectSegPerpendicularThroughPoint(p21,p22,
                                                  p11,
                                                  point_on_line,dummy_double))
            if (getDistanceOfPoints(point_on_line,p11) < C_DOUBLE_EPS)
                return true;
        if (intersectSegPerpendicularThroughPoint(p21,p22,
                                                  p12,
                                                  point_on_line,dummy_double))
            if (getDistanceOfPoints(point_on_line,p12) < C_DOUBLE_EPS)
                return true;
        if (intersectSegPerpendicularThroughPoint(p11,p12,
                                                  p21,
                                                  point_on_line,dummy_double))
            if (getDistanceOfPoints(point_on_line,p21) < C_DOUBLE_EPS)
                return true;
        if (intersectSegPerpendicularThroughPoint(p11,p12,
                                                  p22,
                                                  point_on_line,dummy_double))
            if (getDistanceOfPoints(point_on_line,p22) < C_DOUBLE_EPS)
                return true;

        return false;
    }

    double s = (vx*wy - vy*wx)/d;
    if(s < 0.0 || s > 1.0) return false;    //no intersection with s1

    double t = (ux*wy - uy*wx)/d;
    if(t < 0.0 || t > 1.0) return false;    //no intersection with s2

    return true;

}

/////////////////////////////////////////////////////////////////////////////

double AVPlanarCalculator::getMinDistanceOfPointToSegment(const QPoint &point,
                                                          const QPoint &p1,const QPoint &p2)
{
    QPoint point_on_line;

    double dummy_double;

    if (intersectSegPerpendicularThroughPoint(p1, p2, point, point_on_line, dummy_double))
    {
        return getDistanceOfPoints(point_on_line,point);
    }

    double d1 = getDistanceOfPoints(p1, point);
    double d2 = getDistanceOfPoints(p2, point);

    return (d1 > d2) ? d2 : d1;
}

/////////////////////////////////////////////////////////////////////////////

double AVPlanarCalculator::getMinDistanceOfPointToSegment(const QPointF &point,
                                                          const QPointF &p1,const QPointF &p2)
{
    QPointF point_on_line;

    double dummy_double;

    if (intersectSegPerpendicularThroughPoint(p1, p2, point, point_on_line, dummy_double))
    {
        return getDistanceOfPoints(point_on_line,point);
    }

    double d1 = getDistanceOfPoints(p1,point);
    double d2 = getDistanceOfPoints(p2,point);

    return (d1 > d2) ? d2 : d1;
}

/////////////////////////////////////////////////////////////////////////////

double AVPlanarCalculator::getMinDistanceOfSegments(const QPointF &p11, const QPointF &p12,
                                                    const QPointF &p21, const QPointF &p22)
{
    double min_distance_found = 111111.11;
    double act_distance;

    QPointF point_on_line;
    double dummy_double;

    if (intersectSegPerpendicularThroughPoint(p21,p22,
                                              p11,
                                              point_on_line,dummy_double))
    {
        act_distance = getDistanceOfPoints(point_on_line,p11);
        if (act_distance < min_distance_found)
            min_distance_found = act_distance;
    }
    if (intersectSegPerpendicularThroughPoint(p21,p22,
                                              p12,
                                              point_on_line,dummy_double))
    {
        act_distance = getDistanceOfPoints(point_on_line,p12);
        if (act_distance < min_distance_found)
            min_distance_found = act_distance;
    }
    if (intersectSegPerpendicularThroughPoint(p11,p12,
                                              p21,
                                              point_on_line,dummy_double))
    {
        act_distance = getDistanceOfPoints(point_on_line,p21);
        if (act_distance < min_distance_found)
            min_distance_found = act_distance;
    }
    if (intersectSegPerpendicularThroughPoint(p11,p12,
                                              p22,
                                              point_on_line,dummy_double))
    {
        act_distance = getDistanceOfPoints(point_on_line,p22);
        if (act_distance < min_distance_found)
            min_distance_found = act_distance;
    }

    act_distance = getDistanceOfPoints(p11,p21);
    if (act_distance < min_distance_found)
        min_distance_found = act_distance;

    act_distance = getDistanceOfPoints(p11,p22);
    if (act_distance < min_distance_found)
        min_distance_found = act_distance;

    act_distance = getDistanceOfPoints(p12,p21);
    if (act_distance < min_distance_found)
        min_distance_found = act_distance;

    act_distance = getDistanceOfPoints(p12,p22);
    if (act_distance < min_distance_found)
        min_distance_found = act_distance;
//     AVLogger->Write(LOG_INFO,"Min distance between seg (%g,%g)(%g,%g) and "
//                              "(%g,%g)(%g,%g) is: %g",
//                     p11.x(),p11.y(),
//                     p12.x(),p12.y(),
//                     p21.x(),p21.y(),
//                     p22.x(),p22.y());

    return min_distance_found;

}

/////////////////////////////////////////////////////////////////////////////

bool AVPlanarCalculator::twoSegsHaveAlmostIntersection(const QPointF &p11, const QPointF &p12,
                                                       const QPointF &p21, const QPointF &p22)
{
    QPointF u = p12 - p11;
    QPointF v = p22 - p21;
    QPointF w = p11 - p21;

    double ux = u.x();
    double uy = u.y();
    double vx = v.x();
    double vy = v.y();
    double wx = w.x();
    double wy = w.y();

    double d = ux*vy - uy*vx;

    if (d==0.0)
        return false;

    if(fabs(d) < C_DOUBLE_EPS) //s1 and s2 are parallel or colinear
    {
        QPointF point_on_line;
        double dummy_double;

        if (intersectSegPerpendicularThroughPoint(p21,p22,
                                                  p11,
                                                  point_on_line,dummy_double))
            if (getDistanceOfPoints(point_on_line,p11) < C_DOUBLE_EPS)
                return true;
        if (intersectSegPerpendicularThroughPoint(p21,p22,
                                                  p12,
                                                  point_on_line,dummy_double))
            if (getDistanceOfPoints(point_on_line,p12) < C_DOUBLE_EPS)
                return true;
        if (intersectSegPerpendicularThroughPoint(p11,p12,
                                                  p21,
                                                  point_on_line,dummy_double))
            if (getDistanceOfPoints(point_on_line,p21) < C_DOUBLE_EPS)
                return true;
        if (intersectSegPerpendicularThroughPoint(p11,p12,
                                                  p22,
                                                  point_on_line,dummy_double))
            if (getDistanceOfPoints(point_on_line,p22) < C_DOUBLE_EPS)
                return true;
    }

    const double MAX_DISTANCE = 0.001;
    const double MAX_ANGLE = 1.0;

        // a * b = |a| * |b| * cos alpha ---> (-90 < alpha < 90) if a * b > 0
    double target_direction_value = (ux * vx +
                                    (uy * vy));

        // calc angle between the vectors

    double vector_a_scalar = sqrt(u.x()*u.x() +
                                  u.y()*u.y());

    double vector_b_scalar = sqrt(v.x()*v.x() +
                                  v.y()*v.y());

    double scalar_product = vector_a_scalar * vector_b_scalar;

    if (scalar_product < 0.0001)
    {
        // TODO check if correct
        return false;
    }

    double tmp = target_direction_value / scalar_product;
    if (tmp > 1.0) tmp = 1.0;
    else if (tmp < -1.0) tmp = -1.0;
    double angle_between_vectors = AVSciConst::convradTodeg(acos(tmp)) - 180.00;
    //AVLogger->Write(LOG_INFO,"Winkel differenz %g",angle_between_vectors);
    if ((angle_between_vectors < MAX_ANGLE) &&
        getMinDistanceOfSegments(p11,p12,p21,p22) < MAX_DISTANCE)
        return true;

    // not linear
    // if the segment are almost linear and minimal distance is low

    double s = (vx*wy - vy*wx)/d;
    if(s < 0.0 || s > 1.0) return false;    //no intersection with s1

    double t = (ux*wy - uy*wx)/d;
    if(t < 0.0 || t > 1.0) return false;    //no intersection with s2

    return true;

}

/////////////////////////////////////////////////////////////////////////////

bool AVPlanarCalculator::intersectLineWithLine(const QPointF &l11, const QPointF &l12,
                                               const QPointF &l21, const QPointF &l22,
                                               QPointF* intersection_point)
{
    QPointF u = l12 - l11;
    QPointF v = l22 - l21;

    double d = u.x()*v.y() - u.y()*v.x();

    if(fabs(d) < C_DOUBLE_EPS) //s1 and s2 are parallel or colinear
    {
        AVLogger->Write(LOG_DEBUG,"AVAviationCalculator::intersectLineWithLine "
                "Lines are linear. Returning false");
        return false;
    }

    // do we need to calculate the point?
    if (intersection_point == 0) return true;

    QPointF w = l11 - l21;

    double s = (v.x()*w.y() - v.y()*w.x())/d;

    *intersection_point = l11 + u*s;

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool AVPlanarCalculator::intersectSegLine(const QPoint &s11, const QPoint &s12,
                                          const QPoint &l11, const QPoint &l12,
                                          QPoint &is, double &scalep11, double &scalel11)
{
    //will only return true, if there is an intersection and the 2 segments
    //are not parallel or colinear
    QPoint u = s12 - s11;
    QPoint v = l12 - l11;
    QPoint w = s11 - l11;

    double ux = u.x();
    double uy = u.y();
    double vx = v.x();
    double vy = v.y();
    double wx = w.x();
    double wy = w.y();

    double d = ux*vy - uy*vx;

    if(fabs(d) < C_DOUBLE_EPS) //s1 and s2 are parallel or colinear
        return false;

    double s = (vx*wy - vy*wx)/d;
    if(s < 0.0 || s > 1.0) return false;    //no intersection with segment

    is = s11 + s*u;

    scalep11 = s;
    scalel11 = (ux*wy - uy*wx)/d;

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool AVPlanarCalculator::intersectSegLine(const QPointF &s11, const QPointF &s12,
                                          const QPointF &l11, const QPointF &l12,
                                          QPointF &is, double &scalep11, double &scalel11)
{
    //will only return true, if there is an intersection and the 2 segments
    //are not parallel or colinear
    QPointF u = s12 - s11;
    QPointF v = l12 - l11;
    QPointF w = s11 - l11;

    double ux = u.x();
    double uy = u.y();
    double vx = v.x();
    double vy = v.y();
    double wx = w.x();
    double wy = w.y();

    double d = ux*vy - uy*vx;

    if(fabs(d) < C_DOUBLE_EPS) //s1 and s2 are parallel or colinear
        return false;

    double s = (vx*wy - vy*wx)/d;
    if(s < 0.0 || s > 1.0) return false;    //no intersection with segment

    is = s11 + u*s;

    scalep11 = s;
    scalel11 = (ux*wy - uy*wx)/d;

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool AVPlanarCalculator::intersectSegPerpendicularThroughPoint(const QPoint &s11,
                                                               const QPoint &s12,
                                                               const QPoint &p,
                                                               QPoint &is, double &scalep11)
{
    QPoint delta = s12 - s11;
    QPoint p1(p.x() + delta.y(), p.y() - delta.x());

    double scalel11 = 0.0;
    return intersectSegLine(s11, s12, p, p1, is, scalep11, scalel11);
}

/////////////////////////////////////////////////////////////////////////////

bool AVPlanarCalculator::intersectSegPerpendicularThroughPoint(const QPointF &s11,
                                                               const QPointF &s12,
                                                               const QPointF &p,
                                                               QPointF &is, double &scalep11)
{
    QPointF delta = s12 - s11;
    QPointF p1(p.x() + delta.y(), p.y() - delta.x());

    double scalel11 = 0.0;
    return intersectSegLine(s11, s12, p, p1, is, scalep11, scalel11);
}

/////////////////////////////////////////////////////////////////////////////

bool AVPlanarCalculator::intersectSegPerpendicularThroughPoint(const QPoint &l11,
                                                               const QPoint &l12,
                                                               const QPoint &p,
                                                               const QPoint &s11,
                                                               const QPoint &s12,
                                                               QPoint &is, double &scales11)
{
    // get perpend. line to l11/l12 through p
    QPoint delta = l12 - l11;
    QPoint p1(p.x() + delta.y(), p.y() - delta.x());

    double scalel11 = 0.0;
    return intersectSegLine(s11, s12, p, p1, is, scales11, scalel11);
}

/////////////////////////////////////////////////////////////////////////////

bool AVPlanarCalculator::intersectLinePerpendicularThroughPoint(const QPointF &l11,
                                                                const QPointF &l12,
                                                                const QPointF &p,
                                                                QPointF &is)
{
    // get perpend. line to l11/l12 through p
    AVASSERT((l11.x() != l12.x()) || (l11.y() != l12.y()));
    QPointF delta = l12 - l11;
    QPointF p1(p.x() + delta.y(), p.y() - delta.x());

    return intersectLineWithLine(l11,l12,p,p1,&is);
}

/////////////////////////////////////////////////////////////////////////////

bool AVPlanarCalculator::intersectLineCircle(double linex1, double liney1,
                                             double linex2, double liney2,
                                             double centerx, double centery, double radius,
                                             double &intersectx1, double &intersecty1,
                                             double &intersectx2, double &intersecty2)
{
    // first check if there is an intersection
    double center_dist =
        normalDistancePointToLine(centerx, centery, linex1, liney1, linex2, liney2);
    if ( center_dist > radius) return false;

    // line:   y = k*x + d
    // circle: (x - centerx)**2 + (y - centery)**2 = r**2

    double dx = linex2 - linex1;
    double dy = liney2 - liney1;
    if (fabs(dx) < C_DOUBLE_EPS && fabs(dy) < C_DOUBLE_EPS)
    {
        // line is a point
        // check if it is on the circle
        if (fabs(center_dist - radius) > C_DOUBLE_EPS) return false;
        intersectx1 = linex1;
        intersecty1 = liney1;
        intersectx2 = linex1;
        intersecty2 = liney1;
    }
    else if (fabs(dx) < C_DOUBLE_EPS)
    {
        // vertical line
        intersectx1 = linex1;
        intersectx2 = linex1;

        double x_minus_centerx = intersectx1 - centerx;
        double b = -2.0*centery;
        double c = centery*centery + x_minus_centerx*x_minus_centerx - radius*radius;

        double n = b*b - 4.0*c;
        AVASSERT(n >= 0.0);
        double sqrt_n = sqrt(n);
        intersecty1 = (-b + sqrt_n)/2.0;
        intersecty2 = (-b - sqrt_n)/2.0;
    }
    else
    {
        // create the line (no vertical line)
        double k = dy/dx;
        double d = liney1 - k*linex1;

        // now solve equation: circle intersects line
        double d_minus_centery = d - centery;
        double a = 1.0 + k*k;
        double b = 2.0*(k*d_minus_centery - centerx);
        double c = centerx*centerx + d_minus_centery*d_minus_centery - radius*radius;

        double n = b*b - 4.0*a*c;
        AVASSERT(n >= 0.0);
        double sqrt_n = sqrt(n);
        intersectx1 = (-b + sqrt_n)/(2.0*a);
        intersectx2 = (-b - sqrt_n)/(2.0*a);

        intersecty1 = k*intersectx1 + d;
        intersecty2 = k*intersectx2 + d;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////

double AVPlanarCalculator::normalDistancePointToLine(double pointx, double pointy,
                                                     double linex1, double liney1,
                                                     double linex2, double liney2)
{
    double ux = linex2 -linex1;
    double uy = liney2 -liney1;

    double vx = pointx - linex1;
    double vy = pointy - liney1;

    double n  = ux*vx + uy*vy;
    double d2 = ux*ux + uy*uy;

    double distance;
    if (d2 < C_DOUBLE_EPS) {
        // line is defined as 2 equal points -> line = point
        double dx = linex1 - pointx;
        double dy = liney1 - pointy;
        distance = sqrt(dx*dx + dy*dy);
    } else {
        double m = n / d2;
        double newpx = linex1 + m*ux;
        double newpy = liney1 + m*uy;
        double dx = newpx - pointx;
        double dy = newpy - pointy;
        distance = sqrt(dx*dx + dy*dy);
    }

    return distance;
}

///////////////////////////////////////////////////////////////////////////////

double AVPlanarCalculator::isLeft(const QPoint &p0, const QPoint &p1,
                                  const QPoint &p2 )
{
//  this can give an integer overrun on 32 bit machines!!
//     int res = ( (p1.x() - p0.x()) * (p2.y() - p0.y()) -
//                 (p2.x() - p0.x()) * (p1.y() - p0.y()) );
    double a1, b1, c1, d1;
    a1 = p1.x() - p0.x();
    b1 = p2.y() - p0.y();
    c1 = p2.x() - p0.x();
    d1 = p1.y() - p0.y();

    return (a1*b1 - c1*d1);
}

///////////////////////////////////////////////////////////////////////////////

double AVPlanarCalculator::isLeft(const QPointF &p0, const QPointF &p1,
                                  const QPointF &p2 )
{
    double a1, b1, c1, d1;
    a1 = p1.x() - p0.x();
    b1 = p2.y() - p0.y();
    c1 = p2.x() - p0.x();
    d1 = p1.y() - p0.y();

    return (a1*b1 - c1*d1);
}

///////////////////////////////////////////////////////////////////////////////

int AVPlanarCalculator::isLeft(double c12, double c13)
{
    double c = c13 - c12;
    if (c < 0.0) c += 360.0;
    if (c > 0.0 && c < 180.0) return -1;
    if (c > 180.0) return 1;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

namespace
{
    template<class Polygon, typename T>
    bool avPlanarContains(const Polygon &polygon, const T &point)
    {
        int imax = polygon.count() - 1;
        if (imax < 0) return false;

        int wn = 0;    // the winding number counter

        // loop through all edges of the polygon
        int i;
        T p1, p2;

        for (i = 0; i < imax; ++i)
        {
            p1 = polygon[i];
            p2 = polygon[i + 1];

            if (p1.y() <= point.y())
            {
                if (p2.y() > point.y())  // an upward crossing
                {
                    if (AVPlanarCalculator::isLeft( p1, p2, point) > 0.0)
                    {
                        ++wn;            // have a valid up intersect
                    }
                }
            }
            else
            {
                if (p2.y() <= point.y())     // a downward crossing
                {
                    if (AVPlanarCalculator::isLeft( p1, p2, point) < 0.0)
                    {
                        --wn;            // have a valid down intersect
                    }
                }
            }
        }

        //test if the polygon is closed
        if(imax > 0)
        {
            p1 = polygon[imax];
            p2 = polygon[0];

            if(!(p1 == p2))
            {
                if (p1.y() <= point.y())
                {
                    if (p2.y() > point.y())  // an upward crossing
                    {
                        if (AVPlanarCalculator::isLeft( p1, p2, point) > 0.0)
                        {
                            ++wn;            // have a valid up intersect
                        }
                    }
                }
                else
                {
                    if (p2.y() <= point.y())     // a downward crossing
                    {
                        if (AVPlanarCalculator::isLeft( p1, p2, point) < 0.0)
                        {
                            --wn;            // have a valid down intersect
                        }
                    }
                }
            }
        }
        return (wn != 0);
    }
}

///////////////////////////////////////////////////////////////////////////////

bool AVPlanarCalculator::contains(const QPolygon &points, const QPoint &p)
{
    return avPlanarContains(points, p);
}

/////////////////////////////////////////////////////////////////////////////

bool AVPlanarCalculator::contains(const QVector<QPoint> &polygon, const QPoint &p)
{
    return avPlanarContains(polygon, p);
}

/////////////////////////////////////////////////////////////////////////////

bool AVPlanarCalculator::contains(const QVector<QPointF> &polygon, const QPointF &p)
{
    return avPlanarContains(polygon, p);
}

/////////////////////////////////////////////////////////////////////////////

bool AVPlanarCalculator::isSimple(const QPolygon &points)
{
    //! Algorithm has O(n^2) complexity and thus is not optimal (O(nlogn))!

    unsigned num_points = points.count();

    AVASSERT(num_points > 1);

    // by definition:
    if (num_points == 2)
    {
        return true;
    }

    for (unsigned i = 0; i < num_points; i++)
    {
        QPoint begin_seg_1 = points.point(i%num_points);
        QPoint end_seg_1 = points.point((i+1)%num_points);
        for (unsigned j = i + 1; j < (i + num_points); j++)
        {
            QPoint begin_seg_2 = points.point(j%num_points);
            QPoint end_seg_2 = points.point((j+1)%num_points);

            // intersection check - routine:

            // check for consecutive segments in polygon
            if ((end_seg_1 == begin_seg_2) || (end_seg_2 == begin_seg_1) ||
                (begin_seg_1 == begin_seg_2) || (end_seg_1 == end_seg_2))
            {
                // means no intersection
                continue;
            }

            // test for existence of an intersect point
            double lsign, rsign;
            lsign = isLeft(begin_seg_1, end_seg_1, begin_seg_2);
            rsign = isLeft(begin_seg_1, end_seg_1, end_seg_2);
            if (lsign * rsign > 0)
            {
                // no intersection
                continue;
            }

            lsign = isLeft(begin_seg_2, end_seg_2, begin_seg_1);
            rsign = isLeft(begin_seg_2, end_seg_2, end_seg_1);
            if (lsign * rsign > 0)
            {
                // no intersection
                continue;
            }

            // otherwise the segments straddle each other
            return false;
        }
    }

    return true;
}

// End of file
