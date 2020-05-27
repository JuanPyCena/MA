///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVCALCULATIONLIB - AviBit Calculation Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Alexander Randeu, a.randeu@avibit.com
    \author  QT4-PORT: Gerhard Scheikl, g.scheikl@avibit.com
    \brief   Declares class coordinate ellipsoide needed by coordinate
             transformations
*/

#if !defined(AVCOORDELLIPSOID_H_INCLUDED)
#define AVCOORDELLIPSOID_H_INCLUDED

// system includes
// required for math symbols (sqrt etc.)
#include <iostream>    // cmath does not work without it under solaris
#include <cmath>
// do not use 'using namespace std;' here because this is a header file!!!

// AVLib includes
#include "avcalculationlib_export.h"
#include "avlog.h"

// forward delaractions

/*! AVCoordEllipsoid class for coordinate ellipsoids needed by coordinate
    transformations.
 */

class AVCALCULATIONLIB_EXPORT AVCoordEllipsoid
{
public:

    //! Predefined Ellipsoids
    enum EllipsoidType {
        ET_CUSTOM = 0,      //!< custom ellipsoid parameters, you must use
                            //!< setEllipsoid_af() to set the parameters
        ET_WGS84  = 1,      //!< parameters for WGS84
        ET_BESSEL = 2,      //!< parameters for BESSEL Ellipsiod
        ET_GRS80  = 3,      //!< parameters for GRS80 Ellipsiod
        ET_KRASSOVSKI40 = 4 //!< parameters for KRASSOVSKI 1940 Ellipsiod
    };

    //! creates the object and sets ET_WGS84 as the default ellipsoid
    /*! \param et predefined ellipsoid, if ET_CUSTOM is used, all data
                  members are set to 0. In this case you must set the
                  ellipsoid parameters with setEllispoid_af() before using
                  the newly created object
    */
    explicit AVCoordEllipsoid(const EllipsoidType &et = ET_WGS84)
    {
        setEll(et);
    };

    //! destroys the object
    virtual ~AVCoordEllipsoid() { };

    //! sets the required ellipsoid parameters
    /*! the parameters e_a and e_f have to be greater than 0.0, else the
        paramters wont be set, and the function returns false.
        \param e_a semi major axis of ellipsoid <m>
        \param e_f flattening of the ellipsoid <>
        \return true if successful, false if not
    */
    virtual bool setEllipsoid_af(const double &e_a, const double &e_f)
    {
        if(e_a > 0. && e_f > 0.) {
            a = e_a;
            f = e_f;
            return calcParams_af();
        }
        else return false;
    }

    /*! sets the required ellipsoid parameters for predifined ellipsoids
        \param et predefined ellipsoid, if ET_CUSTOM is used, all data
                  members are set to 0.. In this case you must set the
                  ellipsoid parameters with setEllispoid_af()
    */
    virtual void setEllipsoid(const EllipsoidType &et)
    {
        setEll(et);
    }

    //! return the semi major axis a of the ellipsoid <m>
    virtual double geta() const { return a; };

    //! return the semi minor axis b of the ellipsoid <m>
    virtual double getb() const { return b; };

    //! return the flattening f of the ellipsoid <>
    virtual double getf() const { return f; };

    //! return the eccentricity e of the ellipsoid <>
    virtual double gete() const { return e; };

    //! return the squared eccentricite e^2 of the ellipsoid <>
    virtual double gete2() const { return e2; };

    //! return the 2. eccentricity e' of the ellipsoid <>
    virtual double getes() const { return es; };

    //! return the squared 2. eccentricite e'^2 of the ellipsoid <>
    virtual double getes2() const { return es2; };

    //! returns true if a and/or b is 0.0
    bool isEmpty() const {
        return (a == 0.0 || f == 0.0);
    }

protected:

    double a;   //!< semi major axis of ellipsoid <m>
    double b;   //!< semi minor axis of ellipsoid <m>
    double f;   //!< flattening of ellipsoid <>
    double e;   //!< eccentricity of ellipsoid <>
    double e2;  //!< e^2 <>
    double es;  //!< <>
    double es2; //!< es^2 <>

    /*! sets the internal ellispoid parameters
        \return true if succesful, false if not
     */
    virtual bool calcParams_af()
    {
        b   = a*(1.0 - f);
        e2  = 1.0 - (b/a)*(b/a);
        es2 = (a/b)*(a/b) - 1.0;
        // win32 has cmath, but does not define the namespace std
#if defined(_OS_WIN32_) || defined(Q_OS_WIN32)
        e  = sqrt(e2);
        es = sqrt(es2);
#else
        e  = std::sqrt(e2);
        es = std::sqrt(es2);
#endif
        return true;
    }

    /*! sets the required ellipsoid parameters for predifined ellipsoids
        \param et predefined ellipsoid, if ET_CUSTOM is used, all data
                  members are set to 0.. In this case you must set the
                  ellipsoid parameters with setEllispoid_af()
    */
    void setEll(const EllipsoidType &et)
    {
        switch (et) {
        case ET_WGS84:
            setEllipsoid_af(6378137.0, 1./298.257223560);
            break;
        case ET_BESSEL:
            setEllipsoid_af(6377397.15508, 3.34277318185e-3);
            break;
        case ET_GRS80:
            setEllipsoid_af(6378137.0, 1.0/298.257222101);
            break;
        case ET_KRASSOVSKI40:
            setEllipsoid_af(6378245.0, 1.0/298.3);
            break;
        case ET_CUSTOM:
            a   = 0.;
            b   = 0.;
            f   = 0.;
            e   = 0.;
            e2  = 0.;
            es  = 0.;
            es2 = 0.;
            break;
        default:
            AVASSERT(false);
        }
    }
};

#endif

// End of file
