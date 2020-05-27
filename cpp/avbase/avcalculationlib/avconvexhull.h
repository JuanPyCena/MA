///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
// QT-Version: QT5
//
// Module:    AVCALCULATIONLIB - Avibit Calculation Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  DI Ulrich Feichter, u.feichter@avibit.com
    \author  QT5-PORT: Ulrich Feichter, u.feichter@avibit.com
    \brief   Implementation of Andrew's monotone chain 2D convex hull algorithm.
*/

#if !defined(AVCONVEXHULL_H_INCLUDED)
#define AVCONVEXHULL_H_INCLUDED

// Qt includes
#include <QtAlgorithms>
#include <QPoint>
#include <QVector>
#include <QPointF>

// AVlib includes


//! Implementation of Andrew's monotone chain 2D convex hull algorithm.
/*! The implementation was tested for QPoint and QPointF as template parameter.
 * Asymptotic complexity: O(n log n).
 * algorithm taken and adapted from http://en.wikibooks.org/wiki/Algorithm_Implementation/Geometry/Convex_hull/Monotone_chain
 */
template<class pointtype>
class AVConvexHull
{
    friend class AVConvexHullTest;

public:
    //! Returns a list of points on the convex hull of points in counter-clockwise order. points may contain repetitions.
    //! Note: the last point in the returned list is the same as the first one.
    static QVector<pointtype> convexHull(QVector<pointtype> points);

protected:

    //! functional used to sort a point list by x in ascending order.
    //! If x of two points is equal they are sorted by y in ascending order
    static bool sortFunction(const pointtype &p1, const pointtype &p2);

    //! 2D cross product of OA and OB vectors of type QPoint, i.e. z-component of their 3D cross product.
    /*! Returns a positive value, if OAB makes a counter-clockwise turn,
     * negative for clockwise turn, and zero if the points are collinear.
     * return value must be big enough to hold 2*max(|coordinate|)^2, so a qint64 is used
     */
    static qint64 cross(const QPoint &O, const QPoint &A, const QPoint &B);

    //! 2D cross product of OA and OB vectors of type QPointF, i.e. z-component of their 3D cross product.
    /*! Returns a positive value, if OAB makes a counter-clockwise turn,
     * negative for clockwise turn, and zero if the points are collinear.
     */
    static double cross(const QPointF &O, const QPointF &A, const QPointF &B);
};

///////////////////////////////////////////////////////////////////////////////

template<class pointtype>
QVector<pointtype> AVConvexHull<pointtype>::convexHull(QVector<pointtype> points)
{
    int num_points = points.count();
    int k = 0;
    QVector<pointtype> hull(2*num_points);

    // Sort points lexicographically
    qSort(points.begin(), points.end(), sortFunction);

    // Build lower hull
    for (int cnt = 0; cnt < num_points; cnt++)
    {
        while (k >= 2 && cross(hull[k-2], hull[k-1], points[cnt]) <= 0)
            k--;
        hull[k++] = points[cnt];
    }

    // Build upper hull
    for (int cnt = num_points-2, t = k+1; cnt >= 0; cnt--)
    {
        while (k >= t && cross(hull[k-2], hull[k-1], points[cnt]) <= 0)
            k--;
        hull[k++] = points[cnt];
    }

    hull.resize(k);
    return hull;
}
///////////////////////////////////////////////////////////////////////////////

template<class pointtype>
bool AVConvexHull<pointtype>::sortFunction(const pointtype &p1, const pointtype &p2)
{
    return (p1.x() < p2.x() || (p1.x() == p2.x() && p1.y() < p2.y()));
}

///////////////////////////////////////////////////////////////////////////////

template<class pointtype>
qint64 AVConvexHull<pointtype>::cross(const QPoint &O, const QPoint &A, const QPoint &B)
{
    return (static_cast<qint64>(A.x()) - O.x()) * (static_cast<qint64>(B.y()) - O.y()) -
           (static_cast<qint64>(A.y()) - O.y()) * (static_cast<qint64>(B.x()) - O.x());
}

///////////////////////////////////////////////////////////////////////////////

template<class pointtype>
double AVConvexHull<pointtype>::cross(const QPointF &O, const QPointF &A, const QPointF &B)
{
   return (A.x() - O.x()) * (B.y() - O.y()) - (A.y() - O.y()) * (B.x() - O.x());
}

#endif

// End of file
