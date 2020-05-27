///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVCALCULATIONLIB - AviBit Calculation Library
//
/////////////////////////////////////////////////////////////////////////////

/*!
  \file
  \author  Dietmar G�sseringer, d.goesseringer@avibit.com
  \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
  \brief   base for 2-D geometry: vectors & coordinates
*/

#if !defined(AVVECTORCOORDS_H_INCLUDED)
#define AVVECTORCOORDS_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////

// System includes
#include <iostream>         // needed HERE for Windows: def. of the operator<<()
#include <math.h>           // for accessing trigonometric functions
using namespace std;

// QT includes
#include <QtGlobal>        // for knowing about Q_INT's

// AV* includes
#include "avcalculationlib_export.h"
#include "avmisc.h"
#include "avsciconst.h"
#include "avaviationcalculator.h"

///////////////////////////////////////////////////////////////////////////////

// CONSTANTS needed by class AVVectorCoords
const double FloatCompareTolerance = 0.0000000001;
                            //!< max difference for floats to state equality

///////////////////////////////////////////////////////////////////////////////
/*!
    \class AVVectorCoords avsensorgeometry.h
    \brief The AVVectorCoords class for 2-D coordinates and 2-D vectors

    The AVVectorCoords class provides basic 2-D vector algebra functionality.
    An instantiated object can be interpreted both as vector and coordinates.

    Coordinates/vectors can be scaled by using the multiplication operator
    (* and *=) and added/substracted using the addition operator (+ and +=)
    and the substraction operator (- and -=), respectively.

    Conversion between cartesian and polar (both deg and rad) is supplied
    through getter and setter methods.

    Ground direction vectors use the speed() and heading() methods.
*/
class AVCALCULATIONLIB_EXPORT AVVectorCoords
{

// ------  CONSTRUCTORS, INITIALIZERS, ASSIGNMENT-OPERATORS

public:

    //! Default constructor
    AVVectorCoords() : m_x(0.0), m_y(0.0) {}

    //! Named Constructor: Constructs a new object by passing cartesian coordinates as arguments.
    /*!
        ex: AVVectorCoords coords1 = AVVectorCoords::Cartesian(1031.4, 453.2);
    */
	static AVVectorCoords Cartesian(double x = 0.0, double y = 0.0);

    //! Named Constructor: Constructs a new object by passing cartesian coordinates as arguments.
	static AVVectorCoords Cart(double x = 0.0, double y = 0.0);

    //! Named Constructor: Constructs a new object by passing start & end point in cartesian coords
    /*! \param start_x x coordinate of start point
        \param start_y y coordinate of start point
        \param end_x x coordinate of end point
        \param end_y y coordinate of end point
     */
    static AVVectorCoords Cart(double start_x, double start_y,
                               double end_x, double end_y);

    //! Named Constructor: Constructs a new object by passing radius and angle [deg] as arguments.
    /*!
        - ex: AVVectorCoords coords1 = AVVectorCoords::PolarDeg(845.4, 96.2);
        \param phi_deg  angle [degrees]
    */
	static AVVectorCoords PolarDeg(double r = 0.0, double phi_deg = 0.0);

    //! Named Constructor: Constructs a new object by passing radius and angle [deg] as arguments.
	static AVVectorCoords PolD(double r = 0.0, double phi_deg = 0.0);

    //! Named Constructor: Constructs a new object by passing radius and angle [rad] as arguments.
    /*!
        - ex: AVVectorCoords coords1 = AVVectorCoords::Cartesian(1031.4, 2.58);
        \param phi_deg  angle [radians]
    */
	static AVVectorCoords PolarRad(double r = 0.0, double phi_rad = 0.0);

    //! Named Constructor: Constructs a new object by passing radius and angle [rad] as arguments.
	static AVVectorCoords PolR(double r = 0.0, double phi_rad = 0.0);

    //! Named Constructor: Constructs a new object by passing speed and heading as arguments.
    /*!
        - ex: AVVectorCoords vector1 = AVVectorCoords::Velocity(603.2, 47.9);
     */
	static AVVectorCoords Velocity(double heading = 0.0, double speed = 0.0);

    //! Copy - Constructor
    AVVectorCoords (const AVVectorCoords& rhs) :
        m_x(rhs.m_x),
        m_y(rhs.m_y) {}

    //! Assignment operator=
    AVVectorCoords& operator=(const AVVectorCoords& rhs) {
        if (this == &rhs) return *this;

        // assign to all data members
        m_x = rhs.m_x;
        m_y = rhs.m_y;

        return *this;
    }

    //! Destructor
    ~AVVectorCoords() {}


// ------  DATA MEMBERS

protected:
    double   m_x;       //!< x-component of coordinates [m] / vector [m/s]
    double   m_y;       //!< y-component of coordinates [m] / vector [m/s]

private:
    //! Constructor. Constructs a coordinates/vector object.
    /*!
        Private. Use public named constructors for object construction.
    */
    AVVectorCoords(double x, double y) : m_x(x), m_y(y) {}


// ------  METHODS

public:

    // ------  Accessors & Mutators (Getters & Setters)

    //! Setter method. Sets this AVVectorCoords to the passed cartesian coordinates.
    void setCartesian(qint32 x, qint32 y) {
        m_x = static_cast<double>(x);
        m_y = static_cast<double>(y);
    }

    //! Setter method. Sets this AVVectorCoords to the passed cartesian coordinates.
    void setCartesian(double x, double y) {
        m_x = x;
        m_y = y;
    }

    //! Setter method. Sets this AVVectorCoords to the polar coordinates (angle in [degrees]).
    void setPolarDeg(double r, double phi_deg) {
        double phi_rad  = phi_deg * AV_PI / 180.0;
        m_x = r * cos(phi_rad);
        m_y = r * sin(phi_rad);
    }

    //! Setter method. Sets this AVVectorCoords to the polar coordinates (angle in [radians]).
    void setPolarRad(double r, double phi_rad) {
        m_x = r * cos(phi_rad);
        m_y = r * sin(phi_rad);
    }

    //! Setter method. Sets this AVVectorCoords to the speed and heading [deg] components.
    void setVelocity(double heading, double speed) {
        setPolarDeg(speed, 90.0 - heading);
    }

    //! Getter method. Returns the X coordinate of this AVVectorCoords object.
    double getCartesianX() const { return m_x; }

    //! Getter method. Returns the X coordinate of this AVVectorCoords object.
    double& x() { return m_x; }

    //! Getter method. Returns the X coordinate of this AVVectorCoords object.
    double x() const { return m_x; }

    //! Getter method. Returns the Y coordinate of this AVVectorCoords object.
    double getCartesianY() const { return m_y; }

    //! Getter method. Returns the Y coordinate of this AVVectorCoords object.
    double& y() { return m_y; }

    //! Getter method. Returns the Y coordinate of this AVVectorCoords object.
    double y() const { return m_y; }

    //! Getter method. Gets the X and Y coordinates of this AVVectorCoords object.
    void getCartesian(double& x, double& y) const {
        x = m_x;
        y = m_y;
    }

    //! Getter method. Gets the cartesian coordinates of this AVVectorCoords object.
    void getCartesian(qint32& x, qint32& y) const {
        x = static_cast<qint32>(m_x);
        y = static_cast<qint32>(m_y);
    }

    //! Getter method. Returns the radial component of the polar coordinates.
    double getPolarRadius() const {
        if (isNull()) return 0.0;
        else          return sqrt(m_x * m_x + m_y * m_y);
    }

    //! Getter method. Returns the norm, i.e. length or amagnitude of the vector.
    double norm() const {
        if (isNull()) return 0.0;
        else          return sqrt(m_x * m_x + m_y * m_y);
    }

    //! Getter method. Returns the angular component of the polar coordinates. [radians]
    double getPolarAngleRad() const {
        if (isNull()) return 0.0;
        else          return atan2(m_y, m_x);
    }

    //! Getter method. Returns the angular component of the polar coordinates. [degrees]
    double getPolarAngleDeg() const {
        return getPolarAngleRad() * 180.0 / AV_PI;
    }

    //! Getter method. Gets the polar coordinates of this AVVectorCoords object. [radians]
    void getPolarRad(double& r, double& phi_deg) const {
        r       = getPolarRadius();
        phi_deg = getPolarAngleRad();
    }

    //! Getter method. Gets the polar coordinates of this AVVectorCoords object. [degrees]
    void getPolarDeg(double& r, double& phi_deg) const {
        r       = getPolarRadius();
        phi_deg = getPolarAngleDeg();
    }

    double minAngleBetweenRad(const AVVectorCoords& rhs) const {
        double alpha =
            AVAviationCalculator::adjustHeadingRad(getPolarAngleRad()) -
            AVAviationCalculator::adjustHeadingRad(rhs.getPolarAngleRad());
        alpha = AVAviationCalculator::adjustHeadingRad(alpha);
        if (alpha > AV_PI) alpha = 2 * AV_PI - alpha;
        return alpha;
    }

    //! Getter method. Returns the speed of this AVVectorCoords object. Equivalent to polar radius.
    double speed() const {
        return getPolarRadius();
    }

    //! Getter method. Returns the heading of this AVVectorCoords object. [degrees]
    double heading() const {
        if (m_x == 0 && m_y == 0)
            return 0.0;
        else
        {
            double h = 90.0 - getPolarAngleDeg();
            if (h < 0.0) h += 360.0;
            return h;
        }
    }

    //! Getter method. Gets the speed and heading of this AVVectorCoords object.
    void getVelocity(double& headingv, double& speedv) const {
        speedv   = speed();
        headingv = heading();
    }

    //! Returns true if the vector/coordinates are equal to (0,0); returns false otherwise.
    bool isNull() const {
        if (m_x == 0 && m_y == 0) return true;
        else                      return false;
    }

    // ------  Mutator methods.

    //! Normalise the vector (afterwards: magnitude = 1)
    bool normalise() {
        double _norm = norm();
        if (_norm < 0.001) return false;
        m_x /= _norm;
        m_y /= _norm;
        return true;
    }

    void rotateHeading(double angle_heading) {
        double angle_rad = AVSciConst::convdegTorad(angle_heading);
        double newx =  m_x*cos(angle_rad) + m_y*sin(angle_rad);
        double newy = -m_x*sin(angle_rad) + m_y*cos(angle_rad);
        m_x = newx;
        m_y = newy;
    }

    void rotateGrad(double angle_grad){
        double angle_rad = AVSciConst::convdegTorad(angle_grad);
        double newx = m_x*cos(angle_rad) - m_y*sin(angle_rad);
        double newy = m_x*sin(angle_rad) + m_y*cos(angle_rad);
        m_x = newx;
        m_y = newy;
    }

    // ------  Comparison methods.

    double minAngleBetweenDeg(const AVVectorCoords& rhs) const {
        return minAngleBetweenRad(rhs) * 180.0 / AV_PI;
    }

    //! Returns the distance to another point as specified in the argument AVVectorCoords.
    double distance(const AVVectorCoords& other) const {
        AVVectorCoords vec = *this - other;
        return vec.getPolarRadius();
    }

    //! Returns the difference of the polar components dRadius (=dSpeed) and dAngle (=dHeading).
    void polarDifference(const AVVectorCoords& other, double& dSpeed, double& dHeading) const {
        if (!(isNull()) && !(other.isNull()))
        {
            dSpeed    =  (speed()   - other.speed()  );
            dHeading  =  (heading() - other.heading());
            if (dHeading < -180.0)     dHeading += 360.0;
            else if (dHeading > 180.0) dHeading -= 360.0;
        }
        else
        {
            dSpeed   = 0.0;
            dHeading = 0.0;
        }
    }

    //! Returns true if the vector is perpendicular to the given vector
    bool isPerpendicular(const AVVectorCoords& rhs) const {
        return AVFLOATEQ(0.0, *this * rhs, FloatCompareTolerance);
    }

    //! Returns true if the vectors have the same heading
    bool isSameDirection(const AVVectorCoords& rhs) const {
        return AVFLOATEQ(getPolarAngleDeg(), rhs.getPolarAngleDeg(),
                         FloatCompareTolerance);
    }

    // ------  Operators

    //! Vector sum: Return sum of x- and y-components of two AVVectorCoords objects.
    AVVectorCoords operator+(const AVVectorCoords& rhs) const {
        return AVVectorCoords(m_x + rhs.m_x, m_y + rhs.m_y);
    }

    //! Vector sum: Add x- and y-components of another AVVectorCoords object.
    AVVectorCoords& operator+=(const AVVectorCoords& rhs) {
        m_x += rhs.m_x;
        m_y += rhs.m_y;
        return *this;
    }

    //! Vector substraction: Return difference of x- and y-components of two AVVectorCoords objects
    AVVectorCoords operator-(const AVVectorCoords& rhs) const {
        return AVVectorCoords(m_x - rhs.m_x, m_y - rhs.m_y);
    }

    //! Vector substraction: Substracts x- and y-components of another AVVectorCoords object.
    AVVectorCoords& operator-=(const AVVectorCoords& rhs) {
        m_x -= rhs.m_x;
        m_y -= rhs.m_y;
        return *this;
    }

    //! Vector multiplication with scalar.
    friend AVVectorCoords operator*(double lhs,
                                    const AVVectorCoords& rhs) {
        return AVVectorCoords(lhs * rhs.m_x, lhs * rhs.m_y);
    }

    //! Vector multiplication with scalar.
    AVVectorCoords operator*(double rhs) const {
        return AVVectorCoords(m_x * rhs, m_y * rhs);
    }

    //! Vector multiplication with vector.
    double operator*(const AVVectorCoords& rhs) const {
        return (m_x * rhs.m_x + m_y * rhs.m_y);
    }

    //! Vector multiplication with scalar.
    AVVectorCoords& operator*=(double rhs) {
        m_x *= rhs;
        m_y *= rhs;
        return *this;
    }

    //! Vector division by scalar.
    AVVectorCoords operator/(double rhs) const {
        if (rhs != 0) return AVVectorCoords(m_x / rhs, m_y / rhs);
        else          return AVVectorCoords(0,0);
    }

    //! Vector division with scalar.
    AVVectorCoords& operator/=(double rhs) {
        if (rhs != 0.0)
        {
            m_x /= rhs;
            m_y /= rhs;
        }
        return *this;
    }

    //! Equality operator. Tests if two AVVectorCoords objects are equal.
    /*! Equality check for doubles: Test if euclidean distance between both
        AVVectorCoords objects is below the FloatCompareTolerance level
        (constant).
    */
    bool operator==(const AVVectorCoords& rhs) {
        return ( fabs(m_x-rhs.m_x) < FloatCompareTolerance &&
                 fabs(m_y-rhs.m_y) < FloatCompareTolerance );
    }

    //! Returns the absulute value of the vector
    /*! this is essentially the same as the polar radius
        \sa getPolarRadius()
     */
    double abs() const {
        return getPolarRadius();
    }

    // ------  Output methods.

    //! Returns a string representation of this AVVectorCoords (Cartesian).
    QString toString(int width = 0, int precision = 2) const {
        QString str, str_x, str_y;
        AVsprintf(str_x, "%.*f", precision, m_x);
        AVsprintf(str_y, "%.*f", precision, m_y);
        AVsprintf(str, "(%s,%s)", qPrintable(str_x.rightJustified(width, '\0')),
                  qPrintable(str_y.rightJustified(width, '\0')));
        return str;
    }

    //! Returns a string representation of this AVVectorCoords (Polar),
    QString toStringPolar() const {
        QString str;
        AVsprintf(str, "(%3.0fdeg%3.0f)", heading(), speed());
        return str;
    }

    //! Formatted output operator. Formatted as "(x,y)".
    friend ostream& operator<<(ostream& os, const AVVectorCoords& rhs) {
        os << qPrintable(rhs.toString());
        return os;
    }
};

#endif

// End of file
