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




// AVLib includes
#include "avlog.h"
#include "avsciconst.h"

// local includes
#include "avearthdatum.h"

///////////////////////////////////////////////////////////////////////////////

#define DOUBLE_EPS static_cast<double>(1.0e-20)

///////////////////////////////////////////////////////////////////////////////

AVEarthDatum::AVEarthDatum(Datum datum)
{
    switch(datum) {
    case ED_WGS84_TO_MGI: {
        m_shiftx = -586.0;  // meter
        m_shifty =  -89.0;  // meter
        m_shiftz = -468.0;  // meter
        m_rotx   = AVSciConst::convdegTorad(5.1/3600.0);
        m_roty   = AVSciConst::convdegTorad(1.4/3600.0);
        m_rotz   = AVSciConst::convdegTorad(5.4/3600.0);
        m_scale  = 1.0 - 1.1e-6;

        m_ref_ell    = AVCoordEllipsoid(AVCoordEllipsoid::ET_BESSEL);
        m_target_ell = AVCoordEllipsoid(AVCoordEllipsoid::ET_WGS84);
        break;
    }
    case ED_WGS84_TO_ETRS89: {
        m_shiftx = 0.0;  // meter
        m_shifty = 0.0;  // meter
        m_shiftz = 0.0;  // meter
        m_rotx   = 0.0;
        m_roty   = 0.0;
        m_rotz   = 0.0;
        m_scale  = 1.0;

        m_ref_ell    = AVCoordEllipsoid(AVCoordEllipsoid::ET_GRS80);
        m_target_ell = AVCoordEllipsoid(AVCoordEllipsoid::ET_WGS84);
        break;
    }
    case ED_WGS84_TO_BESSEL_POTSDAM: {
        m_shiftx = -598.1;  // meter
        m_shifty = -73.7;   // meter
        m_shiftz = -418.2;  // meter
        m_rotx   = AVSciConst::convdegTorad(0.202/3600.0);
        m_roty   = AVSciConst::convdegTorad(0.045/3600.0);
        m_rotz   = AVSciConst::convdegTorad(-2.455/3600.0);
        m_scale  = 1.0 - 6.7e-6;

        m_ref_ell    = AVCoordEllipsoid(AVCoordEllipsoid::ET_BESSEL);
        m_target_ell = AVCoordEllipsoid(AVCoordEllipsoid::ET_WGS84);
        break;
    }
    case ED_WGS84_TO_PULKOVO42_KRASSOVSKI40: {
        // params from http://www.geo.ut.ee/~raivo/bursa.html
        m_shiftx = -21.58719;  // meter
        m_shifty = 97.54127;   // meter
        m_shiftz = 60.92546;  // meter
        m_rotx   = AVSciConst::convdegTorad(1.01378/3600.0);
        m_roty   = AVSciConst::convdegTorad(0.58117/3600.0);
        m_rotz   = AVSciConst::convdegTorad(0.23480/3600.0);
        m_scale  = 1.0 + 4.6121e-6;

        m_ref_ell    = AVCoordEllipsoid(AVCoordEllipsoid::ET_KRASSOVSKI40);
        m_target_ell = AVCoordEllipsoid(AVCoordEllipsoid::ET_WGS84);
        break;
    }
    case ED_WGS84_TO_PULKOVO42_KRASSOVSKI40_42_83: {
        // http://www.mapref.org/GeodeticReferenceSystemsDE.html#Zweig674
        // Country: Germany
        // Name: System  42/83
        // Central Point: Pulkow
        // Spheroid: Krasovsky
        // Datum - > WGS84
        // dx[m]=24
        // dy[m]=-123
        // dz[m]=-94
        // rx["]=-0.02
        // ry["]=0.25
        // rz["]=0.13
        // ds[ppm]=1.1
        // Ihde, Lindstrot 1995
        // WGS84->Datum by Randeu
        // dx[m]=-23.99993719
        // dy[m]=122.99985869
        // dz[m]=93.99991376
        // rx["]=0.02
        // ry["]=-0.25
        // rz["]=-0.13
        // ds[ppm]=-1.1
        m_shiftx = -23.99993719;  // meter
        m_shifty = 122.99985869;  // meter
        m_shiftz = 93.99991376;   // meter
        m_rotx   = AVSciConst::convdegTorad(0.02/3600.0);
        m_roty   = AVSciConst::convdegTorad(-0.25/3600.0);
        m_rotz   = AVSciConst::convdegTorad(-0.13/3600.0);
        m_scale  = 1.0 - 1.1e-6;

        m_ref_ell    = AVCoordEllipsoid(AVCoordEllipsoid::ET_KRASSOVSKI40);
        m_target_ell = AVCoordEllipsoid(AVCoordEllipsoid::ET_WGS84);
        break;
    }
    default: {
        AVASSERT(false);
        break;
    }
    }

    AVASSERT(fabs(m_scale) > DOUBLE_EPS);

    // check for shift
    m_do_shift = false;
    if (fabs(m_shiftx) > DOUBLE_EPS ||
        fabs(m_shifty) > DOUBLE_EPS ||
        fabs(m_shiftz) > DOUBLE_EPS) m_do_shift = true;

    // create rotation matrix
    m_rot_matrix[0][0] = cos(m_roty)*cos(m_rotz);
    m_rot_matrix[0][1] = cos(m_rotx)*sin(m_rotz) + sin(m_rotx)*sin(m_roty)*cos(m_rotz);
    m_rot_matrix[0][2] = sin(m_rotx)*sin(m_rotz) - cos(m_rotx)*sin(m_roty)*cos(m_rotz);
    m_rot_matrix[1][0] = -cos(m_roty)*sin(m_rotz);
    m_rot_matrix[1][1] = cos(m_rotx)*cos(m_rotz) - sin(m_rotx)*sin(m_roty)*sin(m_rotz);
    m_rot_matrix[1][2] = sin(m_rotx)*cos(m_rotz) + cos(m_rotx)*sin(m_roty)*sin(m_rotz);
    m_rot_matrix[2][0] = sin(m_roty);
    m_rot_matrix[2][1] = - sin(m_rotx)*cos(m_roty);
    m_rot_matrix[2][2] = cos(m_rotx)*cos(m_roty);

}

///////////////////////////////////////////////////////////////////////////////

AVEarthDatum::~AVEarthDatum()
{
}

///////////////////////////////////////////////////////////////////////////////

bool AVEarthDatum::toReference(double la1, double lo1, double h1,
                               double &la2, double &lo2, double &h2)
{
    // e.g. for Datum::ED_WGS84_TO_MGI WGS84 is the actual system and MGI is the
    // reference system. WGS84 -> MGI

    la1 = AVSciConst::convdegTorad(la1);
    lo1 = AVSciConst::convdegTorad(lo1);

    // convert to X,Y,Z
    double X[3];
    latlong2xyz(la1, lo1, h1, X, m_target_ell);

    double XT[3];
    XT[0] = 0.0;
    XT[1] = 0.0;
    XT[2] = 0.0;
    // rotate
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            XT[i] += m_rot_matrix[i][j]*X[j];
        }
    }

    // scale
    for (int k = 0; k < 3; ++k) {
        XT[k] *= m_scale;
    }

    // shift
    if (m_do_shift) {
        XT[0] += m_shiftx;
        XT[1] += m_shifty;
        XT[2] += m_shiftz;
    }

    // convert to la/lo
    xyz2latlong(XT, la2, lo2, h2, m_ref_ell);

    la2 = AVSciConst::convradTodeg(la2);
    lo2 = AVSciConst::convradTodeg(lo2);

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVEarthDatum::fromReference(double la1, double lo1, double h1,
                                 double &la2, double &lo2, double &h2)
{
    // e.g. for Datum::ED_WGS84_TO_MGI WGS84 is the actual system and MGI is the
    // reference system. MGI -> WGS84

    la1 = AVSciConst::convdegTorad(la1);
    lo1 = AVSciConst::convdegTorad(lo1);

    // convert to X,Y,Z
    double X[3];
    latlong2xyz(la1, lo1, h1, X, m_ref_ell);

    // unshift
    if (m_do_shift) {
        X[0] -= m_shiftx;
        X[1] -= m_shifty;
        X[2] -= m_shiftz;
    }

    // unrotate
    double XT[3];
    XT[0] = 0.0;
    XT[1] = 0.0;
    XT[2] = 0.0;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            XT[i] += m_rot_matrix[j][i]*X[j];
        }
    }

    // unscale
    for (int k = 0; k < 3; ++k) {
        XT[k] /= m_scale;
    }


    // convert to la/lo
    xyz2latlong(XT, la2, lo2, h2, m_target_ell);

    la2 = AVSciConst::convradTodeg(la2);
    lo2 = AVSciConst::convradTodeg(lo2);

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void AVEarthDatum::latlong2xyz(double la, double lo, double h, double X[3],
                               const AVCoordEllipsoid &ell)
{
    double cos_la = cos(la);
    double cos_lo = cos(lo);
    double sin_la = sin(la);
    double sin_lo = sin(lo);

    double N = ell.geta()*ell.geta()/(ell.getb()*sqrt(1.0 + ell.getes2()*cos_la*cos_la));

    X[0] = (N + h)*cos_la*cos_lo;
    X[1] = (N + h)*cos_la*sin_lo;
    X[2] = ((1.0 - ell.gete2())*N + h)*sin_la;

//     qDebug("latlong2xyz: la=%.6f lo=%.6f, h=%.4f, x=%.4f, y=%.4f, z=%.4f",
//            AVSciConst::convradTodeg(la), AVSciConst::convradTodeg(lo), h,
//            X[0], X[1], X[2]);

}

///////////////////////////////////////////////////////////////////////////////

void AVEarthDatum::xyz2latlong(const double X[3], double &la, double &lo, double &h,
                               const AVCoordEllipsoid &ell)
{
    double s = sqrt(X[0]*X[0] + X[1]*X[1]);
    double d = atan2(X[2]*ell.geta(), s*ell.getb());
    double sin_d = sin(d);
    double cos_d = cos(d);

    la = atan((X[2] + ell.getes2()*ell.getb()*sin_d*sin_d*sin_d)/
              (s - ell.gete2()*ell.geta()*cos_d*cos_d*cos_d));

    lo = atan2(X[1], X[0]);

    double cos_la = cos(la);
    double N = ell.geta()*ell.geta()/(ell.getb()*sqrt(1.0 + ell.getes2()*cos_la*cos_la));
    h = s/cos_la - N;

//     qDebug("xyz2latlong: la=%.6f lo=%.6f, h=%.4f, x=%.4f, y=%.4f, z=%.4f",
//            AVSciConst::convradTodeg(la), AVSciConst::convradTodeg(lo), h,
//            X[0], X[1], X[2]);
}

// End of file
