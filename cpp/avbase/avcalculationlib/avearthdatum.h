///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  DI Alexander Randeu, a.randeu@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief
*/


#ifndef __AVEARTHDATUM_H__
#define __AVEARTHDATUM_H__

// local includes
#include "avcalculationlib_export.h"
#include "avcoordellipsoid.h"

///////////////////////////////////////////////////////////////////////////////

class AVCALCULATIONLIB_EXPORT AVEarthDatum
{
public:
    enum Datum {
        ED_WGS84_TO_MGI    = 0,    //!< WGS84 to MGI/BESSEL
        ED_WGS84_TO_ETRS89 = 1,    //!< WGS84 to ETRS89/GRS80
        ED_WGS84_TO_BESSEL_POTSDAM = 2,     //!< WGS84 to BESSEL/DHDN POTSDAM 2001
        ED_WGS84_TO_PULKOVO42_KRASSOVSKI40 = 3, //!< WGS84 to PULKOVO-42/KRASSOVSKI 1940
        ED_WGS84_TO_PULKOVO42_KRASSOVSKI40_42_83 = 4 //!< WGS84 to PULKOVO-42/KRASSOVSKI 1940,
                                                     //!< System 42/83
    };

    //! constructor
    /*! \param datum earht datum \sa Datum
     */
    explicit AVEarthDatum(Datum datum);

    //! destructor
    virtual ~AVEarthDatum();

    //! Converts la1/lo1 given in the datum used in the constructor to the
    //! reference datum
    /*! e.g. for Datum::ED_WGS84_TO_MGI WGS84 is the actual system and MGI is the
        reference system. WGS84 -> MGI
     */
    bool toReference(double la1, double lo1, double h1,
                     double &la2, double &lo2, double &h2);

    //! Converts la1/lo1 given in the reference datum to the datum used in constructor
    /*! e.g. for Datum::ED_WGS84_TO_MGI WGS84 is the actual system and MGI is the
        reference system. MGI -> WGS84
     */
    bool fromReference(double la1, double lo1, double h1,
                       double &la2, double &lo2, double &h2);

protected:

    AVCoordEllipsoid m_ref_ell;
    AVCoordEllipsoid m_target_ell;

    double m_shiftx;        //!< meter
    double m_shifty;        //!< meter
    double m_shiftz;        //!< meter
    double m_rotx;          //!< radiant
    double m_roty;          //!< radiant
    double m_rotz;          //!< radiant
    double m_scale;         //!< []
    bool   m_do_shift;

    double m_rot_matrix[3][3];

    //! convert la/lo (in rads) and h (in meter) to cartesian coordinates X (in meter)
    //! based on the given ellipsoid
    void latlong2xyz(double la, double lo, double h, double X[3],
                     const AVCoordEllipsoid &ell);

    //! convert cartesian coordinates X (in meter) to la/lo (in rads) and h (in meter)
    //! based on the given ellipsoid
    void xyz2latlong(const double X[3], double &la, double &lo, double &h,
                     const AVCoordEllipsoid &ell);
};

#endif /* __AVEARTHDATUM_H__ */

// End of file
