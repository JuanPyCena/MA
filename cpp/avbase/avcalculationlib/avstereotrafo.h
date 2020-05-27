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
    \brief   Declares class for stereographic transformations based on
             EUROCONTROL TransLib SASSC-SRD-980154 and
             ARTAS IRS 46127301-506/V7.1/29.10.2004 Appendix A2
*/

#ifndef __AVSTEREOTRAFO_H__
#define __AVSTEREOTRAFO_H__

// AVLib includes
#include "avcalculationlib_export.h"
#include "avcoordellipsoid.h"
#include "avcoordinatetransform3dbase.h"

// forward delaractions

///////////////////////////////////////////////////////////////////////////////
//! AVStereoTrafo class for coordinate transformations
/*! Based on Declares class for stereographic transformations based on
    EUROCONTROL TransLib SASSC-SRD-980154 and
    ARTAS IRS 46127301-506/V7.1/29.10.2004 Appendix A2
    Transforms stereographic coordinates to geodesian coordinates.

    TODO FIXXME: Make common base class with AVSSRTrafo!!
*/

class AVCALCULATIONLIB_EXPORT AVStereoTrafo : public AVCoordinateTransform3dBase
{
    friend class AVStereoTrafoTest;
public:

    /*! creates a new transformation object
        as default the radar position is set to (0.,0.,0.)
        and the coordinate ellipsoid is set to WGS84
     */
    explicit AVStereoTrafo(const QString& name = QString::null)
        : AVCoordinateTransform3dBase(name),
          m_glatr(0.),
          m_glonr(0.),
          m_hxy(0.),
          m_huv(0.),
          m_RT(0.),
          m_radar_set_done(false),
          ell(AVCoordEllipsoid::ET_WGS84)
    {
        for (uint i = 0; i < 3; ++i)
        {
            for (uint j = 0; j < 3; ++j)
            {
                m_R2_mat[i][j] = 0.0;
            }
            m_T2_vec[i] = 0.0;
        }
    }

    //! destroys the object
    ~AVStereoTrafo() override{};

    /*! sets the radar position in geodetic coordinates belonging
        to the ellipsoid set with setEllipsoid()
        \param geod_lat geodetic latitude of radar <deg.>
        \param geod_lon geodetic longitude of radar <deg.>
        \param geod_h geodetic height of radar <m> (denoted
                      as h_xy in ARTAS IRS, chapter 10.9)
        \return true if successful, false if not (e.g. see note)
        \note geod_lat must be between -90deg. <= geod_lat <= + 90deg.
     */
    bool setRadarPos(const double &geod_lat, const double &geod_lon, const double &geod_h) override
    {
        if(geod_lat < -90. || geod_lat > 90.) return false;

        m_glatr = DEG2RAD*geod_lat;
        m_glonr = DEG2RAD*geod_lon;
        m_hxy = geod_h;

        m_huv = 0.0; // set to 0!
        m_RT  = (ell.geta()*(1.0 - ell.gete2()))/pow(1.0 - ell.gete2()*sin(m_glatr)*sin(m_glatr),
                                                     1.5);

        m_R2_mat[0][0] = -sin(m_glonr);
        m_R2_mat[0][1] =  cos(m_glonr);
        m_R2_mat[0][2] =  0.0;
        m_R2_mat[1][0] = -sin(m_glatr)*cos(m_glonr);
        m_R2_mat[1][1] = -sin(m_glatr)*sin(m_glonr);
        m_R2_mat[1][2] =  cos(m_glatr);
        m_R2_mat[2][0] =  cos(m_glatr)*cos(m_glonr);
        m_R2_mat[2][1] =  cos(m_glatr)*sin(m_glonr);
        m_R2_mat[2][2] =  sin(m_glatr);

        double eta_R = ell.geta()/sqrt(1.0 - ell.gete2()*sin(m_glatr)*sin(m_glatr));

        m_T2_vec[0] = (m_hxy + eta_R)*cos(m_glatr)*cos(m_glonr);
        m_T2_vec[1] = (m_hxy + eta_R)*cos(m_glatr)*sin(m_glonr);
        m_T2_vec[2] = (m_hxy + eta_R*(1.0 - ell.gete2()))*sin(m_glatr);

        m_radar_set_done = true;
        return true;
    }

    /*! returns the radar position in geodetic coordinates
        \param geod_lat geodetic latitude of radar <deg.>
        \param geod_lon geodetic longitude of radar <deg.>
        \param geod_h geodetic height of radar <m> (denoted
                      as h_xy in ARTAS IRS, chapter 10.9)
     */
    void getRadarPos(double &geod_lat, double &geod_lon, double &geod_h) const override
    {
        geod_lat = RAD2DEG*m_glatr;
        geod_lon = RAD2DEG*m_glonr;
        geod_h = m_hxy;
    }

    //! returns the geod. radar height
    double getRadarGeodHeightM() const override { return m_hxy; }

    /*! sets the ellipsoid used for geodetic coordinates
        \param el coordinate ellipsoid
     */
    virtual void setEllipsoid(const AVCoordEllipsoid &el)
    {
        ell = el;

        if (m_radar_set_done) setRadarPos(m_glatr, m_glonr, m_hxy);
    }

    /*! returns the ellipsoid used for geodetic coordinates
        \param el coordinate ellipsoid
     */
    virtual AVCoordEllipsoid getEllipsoid() const
    {
        return ell;
    }

    //! convert u/v/w to lat/long/h in degrees/meters
    /*! Calls stereoObs2Geod().
     */
    bool uvw2latlonh(const double &u, const double &v, const double &H, double &geod_lat, double &geod_lon,
                     double &geod_h) const override
    {
        return stereoObs2Geod(u, v, H, geod_lat, geod_lon, geod_h);
    }

    //! lat/long in degrees/meters to u/v/w
    /*! Calls geod2StereoObs().
     */
    bool latlonh2uvw(const double &geod_lat, const double &geod_lon, const double &geod_h, double &u, double &v,
                     double &H) const override
    {
        return geod2StereoObs(geod_lat, geod_lon, geod_h, u, v, H);
    }

    /*! Transforms stereographic coordinates SC (u, v, H) to
        geodesic coordinates GC (lat, lon, h)
        \param u stereographic coordinate <m>
        \param v stereographic coordinate <m>
        \param H altitude of target above earth (not uv-plane !!) <m>
        \param geod_lat geodetic latitude GC lat <deg,>
        \param geod_lon geodetic longitude GC lon <deg,>
        \param geod_h geodetic height GC h <m>
        \return true if successful, false if not
        \note implementation belongs to ARTAS IRS, 10.9 STE->SYS, 10.7 SYS->GEO,
              10.10 GEO->LGH, SASSC-TransLib-SR-25
     */
    virtual bool stereoObs2Geod(const double &u,
                                const double &v,
                                const double &H,
                                double &geod_lat,
                                double &geod_lon,
                                double &geod_h) const;

    /*! Transforms geodesic coordinates GC (lat, lon, h) to
        stereographic coordinates SC (u, v, H)
        \param geod_lat geodetic latitude GC lat <deg,>
        \param geod_lon geodetic longitude GC lon <deg,>
        \param geod_h geodetic height GC h <m>
        \param u stereographic coordinate <m>
        \param v stereographic coordinate <m>
        \param H altitude of target above earth (not uv-plane !!) <m>
        \return true if successful, false if not
        \note implementation belongs to ARTAS IRS, 10.10 LGH->GEO, SASSC-TransLib-SR-25,
              10.7 GEO->SYS, 10.9 SYS->STE
     */
    virtual bool geod2StereoObs(const double &geod_lat,
                                const double &geod_lon,
                                const double &geod_h,
                                double &u,
                                double &v,
                                double &H) const;

protected:
	static const double PI;
	static const double RAD2DEG;
	static const double DEG2RAD;

    double m_glatr;   //!< geod. latitude of radar <rad>
    double m_glonr;   //!< geod. longitude of radar <rad>
    double m_hxy;     //!< geod. height of radar <m>
    double m_huv;     //!< geod. height of stereographic reference plane <m>
    double m_RT;      //!< radius of tangent sphere to ellipsoid at radar point <m>
    double m_R2_mat[3][3]; //!< helper matrix
    double m_T2_vec[3];    //!< helper vector
    bool   m_radar_set_done; //!< if the radar pos was already set

    AVCoordEllipsoid ell;   //!< coordinate ellipsoid used for geodesian coord.

    /*! Transforms stereographic coordinates SC (u, v, H) to
        system cartesian coordinates (xc, yc, zc)
        \param u stereographic coordinate <m>
        \param v stereographic coordinate <m>
        \param H altitude of target above earth (not uv-plane !!) <m>
        \param xc system cartesian x coordinate <m>
        \param yc system cartesian y coordinate <m>
        \param zc system cartesian z coordinate <m>
        \return true if successful, false if not
        \note implementation belongs to ARTAS IRS, 10.9 STE->SYS
     */
    virtual bool stereoObs2SysCart(const double &u,
                                   const double &v,
                                   const double &H,
                                   double &xc,
                                   double &yc,
                                   double &zc) const;

    /*! Transforms system cartesian coordinates (xc, yc, zc) to
        stereographic coordinates SC (u, v, H)
        \param xc system cartesian x coordinate <m>
        \param yc system cartesian y coordinate <m>
        \param zc system cartesian z coordinate <m>
        \param u stereographic coordinate <m>
        \param v stereographic coordinate <m>
        \param H altitude of target above earth (not uv-plane !!) <m>
        \return true if successful, false if not
        \note implementation belongs to ARTAS IRS, 10.9 SYS->STE
     */
    virtual bool sysCart2StereoObs(const double &xc,
                                   const double &yc,
                                   const double &zc,
                                   double &u,
                                   double &v,
                                   double &H) const;

    /*! Transforms system cartesian coordinates (xc, yc, zc) to
        cartesian geocentric coordinates (xg, yg, zg)
        \param xc system cartesian x coordinate <m>
        \param yc system cartesian y coordinate <m>
        \param zc system cartesian z coordinate <m>
        \param xg system cartesian x coordinate <m>
        \param yg system cartesian y coordinate <m>
        \param zg system cartesian z coordinate <m>
        \return true if successful, false if not
        \note implementation belongs to ARTAS IRS, 10.7 SYS->GEO
     */
    virtual bool sysCart2GeocenSysCart(const double &xc,
                                       const double &yc,
                                       const double &zc,
                                       double &xg,
                                       double &yg,
                                       double &zg) const;

    /*! Transforms cartesian geocentric coordinates (xg, yg, zg) to
        system cartesian coordinates (xc, yc, zc)
        \param xg system cartesian x coordinate <m>
        \param yg system cartesian y coordinate <m>
        \param zg system cartesian z coordinate <m>
        \param xc system cartesian x coordinate <m>
        \param yc system cartesian y coordinate <m>
        \param zc system cartesian z coordinate <m>
        \return true if successful, false if not
        \note implementation belongs to ARTAS IRS, 10.7 GEO->SYS
     */
    virtual bool geocenSysCart2SysCart(const double &xg,
                                       const double &yg,
                                       const double &zg,
                                       double &xc,
                                       double &yc,
                                       double &zc) const;

    /*! Transforms geocentric radar system coordinates GRSC (xgs, ygs, zgs) to
        radar observation coordinates ROC (rho, theta, H)
        \param xgs geocentric radar cartesian coordinate xgs <m>
        \param ygs geocentric radar cartesian coordinate ygs <m>
        \param zgs geocentric radar cartesian coordinate zgs <m>
        \param geod_lat geodetic latitude GC lat <rad>
        \param geod_lon geodetic longitude GC lon <rad>
        \param geod_h geodetic height GC h <m>
        \return true if successful, false if not
        \note implementation belongs to SASSC-TransLib-SR-25
     */
    virtual bool geocenSysCart2Geod_rad(const double &xgs,
                                        const double &ygs,
                                        const double &zgs,
                                        double &geod_lat,
                                        double &geod_lon,
                                        double &geod_h) const;

    /*! Transforms radar observation coordinates ROC (rho, theta, H) to
        geocentric radar system coordinates GRSC (xgs, ygs, zgs)
        \param geod_lat geodetic latitude GC lat <rad>
        \param geod_lon geodetic longitude GC lon <rad>
        \param geod_h geodetic height GC h <m>
        \param xgs geocentric radar cartesian coordinate xgs <m>
        \param ygs geocentric radar cartesian coordinate ygs <m>
        \param zgs geocentric radar cartesian coordinate zgs <m>
        \return true if successful, false if not
        \note implementation belongs to SASSC-TransLib-SR-24
     */
    virtual bool geod_rad2GeocenSysCart(const double &geod_lat,
                                        const double &geod_lon,
                                        const double &geod_h,
                                        double &xgs,
                                        double &ygs,
                                        double &zgs) const;

};

#endif /* __AVSTEREOTRAFO_H__ */

// End of file
