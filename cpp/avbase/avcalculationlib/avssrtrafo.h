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
    \brief   Declares class for ssr coordinate transformations based on
             EUROCONTROL TransLib SASSC-SRD-980154
*/

#if !defined(AVSSRTRAFO_H_INCLUDED)
#define AVSSRTRAFO_H_INCLUDED


#include "qstring.h"

// AVLib includes
#include "avcalculationlib_export.h"
#include "avcoordellipsoid.h"
#include "avcoordinatetransform3dbase.h"

// forward delaractions

///////////////////////////////////////////////////////////////////////////////
//! AVSSRTrafo class for coordinate transformations based on
//! EUROCONTROL TransLib SASSC-SRD-980154.
/*! Transforms NLR radar observation coordinates to geodesian coordinates.
 */

class AVCALCULATIONLIB_EXPORT AVSSRTrafo : public AVCoordinateTransform3dBase
{
public:

    /*! creates a new transformation object
        as default the radar position is set to (0.,0.,0.)
        and the coordinate ellipsoid is set to WGS84
     */
    explicit AVSSRTrafo(const QString& name = QString::null)
        : AVCoordinateTransform3dBase(name),
          m_glatr(0.), m_glonr(0.), m_ghr(0.),
          ell(AVCoordEllipsoid::ET_WGS84) {};

    //! destroys the object
    ~AVSSRTrafo() override{};

    /*! sets the radar position in geodetic coordinates belonging
        to the ellipsoid set with setEllipsoid()
        \param geod_lat geodetic latitude of radar <deg.>
        \param geod_lon geodetic longitude of radar <deg.>
        \param geod_h geodetic height of radar <m>
        \return true if successful, false if not (e.g. see note)
        \note geod_lat must be between -90deg. <= geod_lat <= + 90deg.
     */
    bool setRadarPos(const double &geod_lat, const double &geod_lon, const double &geod_h) override
    {
        if(geod_lat < -90. || geod_lat > 90.) return false;

        m_glatr = DEG2RAD*geod_lat;
        m_glonr = DEG2RAD*geod_lon;
        m_ghr = geod_h;

        return true;
    }

    /*! returns the radar position in geodetic coordinates
        \param geod_lat geodetic latitude of radar <deg.>
        \param geod_lon geodetic longitude of radar <deg.>
        \param geod_h geodetic height of radar <m>
     */
    void getRadarPos(double &geod_lat, double &geod_lon, double &geod_h) const override
    {
        geod_lat = RAD2DEG*m_glatr;
        geod_lon = RAD2DEG*m_glonr;
        geod_h = m_ghr;
    }

    //! returns the geod. radar height
    double getRadarGeodHeightM() const override { return m_ghr; }

    /*! sets the ellipsoid used for geodetic coordinates
        \param el coordinate ellipsoid
     */
    virtual void setEllipsoid(const AVCoordEllipsoid &el)
    {
        ell = el;
    }

    /*! returns the ellipsoid used for geodetic coordinates
        \param el coordinate ellipsoid
     */
    virtual AVCoordEllipsoid getEllipsoid() const
    {
        return ell;
    }

    //! convert u/v/w to lat/long/h in degrees/meters
    /*! Calls radarCart2Geod().
     */
    bool uvw2latlonh(const double &xr, const double &yr, const double &zr, double &geod_lat, double &geod_lon,
                     double &geod_h) const override
    {
        return radarCart2Geod(xr, yr, zr, geod_lat, geod_lon, geod_h);
    }

    //! lat/long in degrees/meters to u/v/w
    /*! Calls geod2RadarCart().
     */
    bool latlonh2uvw(const double &geod_lat, const double &geod_lon, const double &geod_h, double &xr, double &yr,
                     double &zr) const override
    {
        return geod2RadarCart(geod_lat, geod_lon, geod_h, xr, yr, zr);
    }

    /*! Transforms radar observation coordinates ROC (rho, theta, H) to
        geodesic coordinates GC (lat, lon, h)
        \param rho range ROC rho <m>
        \param theta azimuth ROC theta <deg.> (0deg.=north, 90deg.=east)
        \param H altitude ROC H <m>
        \param geod_lat geodetic latitude GC lat <deg,>
        \param geod_lon geodetic longitude GC lon <deg,>
        \param geod_h geodetic height GC h <m>
        \return true if successful, false if not
        \note implementation belongs to SASSC-TransLib-SR-28,27 and 25
     */
    virtual bool radarObs2Geod(const double &rho,
                               const double &theta,
                               const double &H,
                               double &geod_lat,
                               double &geod_lon,
                               double &geod_h) const;

    /*! Transforms geodesic coordinates GC (lat, lon, h) to radar observation
        coordinates ROC (rho, theta, H)
        \param geod_lat geodetic latitude GC lat <deg,>
        \param geod_lon geodetic longitude GC lon <deg,>
        \param geod_h geodetic height GC h <m>
        \param rho range ROC rho <m>
        \param theta azimuth ROC theta <deg.> (0deg.=north, 90deg.=east)
        \param H altitude ROC H <m>
        \return true if successful, false if not
        \note implementation belongs to SASSC-TransLib-SR-24,26 and an own
              implementation from radar cartesian to radar obs. coord.
     */
    virtual bool geod2RadarObs(const double &geod_lat,
                               const double &geod_lon,
                               const double &geod_h,
                               double &rho,
                               double &theta,
                               double &H) const;

    /*! Transforms radar cartesian coordinates RCC (xr, yr, zr) to
        geodesic coordinates GC (lat, lon, h)
        \param xr radar cartesian coordinate xr <m>
        \param yr radar cartesian coordinate yr <m>
        \param zr radar cartesian coordinate zr <m>
        \param geod_lat geodetic latitude GC lat <deg,>
        \param geod_lon geodetic longitude GC lon <deg,>
        \param geod_h geodetic height GC h <m>
        \return true if successful, false if not
     */
    virtual bool radarCart2Geod(const double &xr,
                                const double &yr,
                                const double &zr,
                                double &geod_lat,
                                double &geod_lon,
                                double &geod_h) const;

    /*! Transforms radar cartesian coordinates RCC (xr, yr, H) to
        geodesic coordinates GC (lat, lon, h)
        \param xr radar cartesian coordinate xr (slant range corrected) <m>
        \param yr radar cartesian coordinate yr (slant range corrected) <m>
        \param H altitude above sea level (e.g. mode-C height with QNH correction) <m>
        \param geod_lat geodetic latitude GC lat <deg,>
        \param geod_lon geodetic longitude GC lon <deg,>
        \param geod_h geodetic height GC h <m>
        \return true if successful, false if not
     */
    virtual bool radarCartHeight2Geod(const double &xr,
                                      const double &yr,
                                      const double &H,
                                      double &geod_lat,
                                      double &geod_lon,
                                      double &geod_h) const;

    /*! Transforms geodesic coordinates GC (lat, lon, h) to
        radar cartesian coordinates RCC (xr, yr, zr)
        \param geod_lat geodetic latitude GC lat <deg,>
        \param geod_lon geodetic longitude GC lon <deg,>
        \param geod_h geodetic height GC h <m>
        \param xr radar cartesian coordinate xr <m>
        \param yr radar cartesian coordinate yr <m>
        \param zr radar cartesian coordinate zr <m>
        \return true if successful, false if not
     */
    virtual bool geod2RadarCart(const double &geod_lat,
                                const double &geod_lon,
                                const double &geod_h,
                                double &xr,
                                double &yr,
                                double &zr) const;

    /*! Transforms geodesic coordinates GC (lat, lon, h) to
        radar cartesian coordinates RCC (xr, yr, H)
        \param geod_lat geodetic latitude GC lat <deg,>
        \param geod_lon geodetic longitude GC lon <deg,>
        \param geod_h geodetic height GC h <m>
        \param xr radar cartesian coordinate xr (slant range corrected) <m>
        \param yr radar cartesian coordinate yr (slant range corrected) <m>
        \param H altitude above sea level (e.g. mode-C height with QNH correction) <m>
        \return true if successful, false if not
        \note corrently not implemented!
     */
    virtual bool geod2RadarCartHeight(const double &geod_lat,
                                      const double &geod_lon,
                                      const double &geod_h,
                                      double &xr,
                                      double &yr,
                                      double &H) const;

    /*! Transforms radar observation coordinates ROC (rho, theta, z) to
        geodesic coordinates GC (lat, lon, h)
        \param rho range ROC rho <m>
        \param theta azimuth ROC theta <deg.> (0deg.=north, 90deg.=east)
        \param z radar cartesian coordinate z <m> (realtive to radar plane)
        \param geod_lat geodetic latitude GC lat <deg,>
        \param geod_lon geodetic longitude GC lon <deg,>
        \param geod_h geodetic height GC h <m>
        \return true if successful, false if not
        \note implementation belongs to SASSC-TransLib-SR-28,27 and 25
     */
    virtual bool radarObsZ2Geod(const double &rho,
                                const double &theta,
                                const double &z,
                                double &geod_lat,
                                double &geod_lon,
                                double &geod_h) const;

    /*! Transforms geodesic coordinates GC (lat, lon, h) to radar observation
        coordinates ROC (rho, theta, z)
        \param geod_lat geodetic latitude GC lat <deg,>
        \param geod_lon geodetic longitude GC lon <deg,>
        \param geod_h geodetic height GC h <m>
        \param rho range ROC rho <m>
        \param theta azimuth ROC theta <deg.> (0deg.=north, 90deg.=east)
        \param z radar cartesian coordinate z <m> (realtive to radar plane)
        \return true if successful, false if not
        \note implementation belongs to SASSC-TransLib-SR-24,26 and an own
              implementation from radar cartesian to radar obs. coord.
     */
    virtual bool geod2RadarObsZ(const double &geod_lat,
                                const double &geod_lon,
                                const double &geod_h,
                                double &rho,
                                double &theta,
                                double &z) const;

    QString uniqueName() const override;

protected:
	static const double PI;
	static const double RAD2DEG;
	static const double DEG2RAD;

    double m_glatr;   //!< geod. latitude of radar <rad>
    double m_glonr;   //!< geod. longitude of radar <rad>
    double m_ghr;     //!< geod. height of radar <m>

    AVCoordEllipsoid ell;   //!< coordinate ellipsoid used for geodesian coord.

    /*! Transforms radar observation coordinates ROC (rho, theta, H) to
        radar cartesian coordinates RCC (xr, yr, zr)
        \param rho range ROC rho <m>
        \param theta azimuth ROC theta <rad> (0deg.=north, 90deg.=east)
        \param H altitude ROC H <m>
        \param xr radar cartesian coordinate xr <m>
        \param yr radar cartesian coordinate yr <m>
        \param zr radar cartesian coordinate zr <m>
        \return if rho < H the function returns false and xr=yr=zr=0., else
                true
        \note implementation belongs to SASSC-TransLib-SR-28
     */
    virtual bool radarObs_rad2RadarCart(const double &rho,
                                        const double &theta,
                                        const double &H,
                                        double &xr,
                                        double &yr,
                                        double &zr) const;

    /*! Transforms radar cartesian coordinates RCC (xr, yr, zr) to radar
        observation coordinates ROC (rho, theta, H)
        \param xr radar cartesian coordinate xr <m>
        \param yr radar cartesian coordinate yr <m>
        \param zr radar cartesian coordinate zr <m>
        \param rho range ROC rho <m>
        \param theta azimuth ROC theta <rad> (0deg.=north, 90deg.=east)
        \param H altitude ROC H <m>
        \return true if successful, false if not
     */
    virtual bool radarCart2RadarObs_rad(const double &xr,
                                        const double &yr,
                                        const double &zr,
                                        double &rho,
                                        double &theta,
                                        double &H) const;

    /*! Transforms radar cartesian coordinates RCC (xr, yr, zr) to
        geocentric radar system coordinates GRSC (xgs, ygs, zgs)
        \param xr radar cartesian coordinate xr <m>
        \param yr radar cartesian coordinate yr <m>
        \param zr radar cartesian coordinate zr <m>
        \param xgs geocentric radar cartesian coordinate xgs <m>
        \param ygs geocentric radar cartesian coordinate ygs <m>
        \param zgs geocentric radar cartesian coordinate zgs <m>
        \return true if successful, false if not
        \note implementation belongs to SASSC-TransLib-SR-27
     */
    virtual bool radarCart2GeocenSysCart(const double &xr,
                                         const double &yr,
                                         const double &zr,
                                         double &xgs,
                                         double &ygs,
                                         double &zgs) const;

    /*! Transforms geocentric radar system coordinates GRSC (xgs, ygs, zgs) to
        radar cartesian coordinates RCC (xr, yr, zr)
        \param xgs geocentric radar cartesian coordinate xgs <m>
        \param ygs geocentric radar cartesian coordinate ygs <m>
        \param zgs geocentric radar cartesian coordinate zgs <m>
        \param xr radar cartesian coordinate xr <m>
        \param yr radar cartesian coordinate yr <m>
        \param zr radar cartesian coordinate zr <m>
        \return true if successful, false if not
        \note implementation belongs to SASSC-TransLib-SR-26
     */
    virtual bool geocenSysCart2RadarCart(const double &xgs,
                                         const double &ygs,
                                         const double &zgs,
                                         double &xr,
                                         double &yr,
                                         double &zr) const;

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

///////////////////////////////////////////////////////////////////////////////

class AVCALCULATIONLIB_EXPORT AVSSRRadarObsTrafo : public AVSSRTrafo
{
public:

    /*! creates a new transformation object
        as default the radar position is set to (0.,0.,0.)
        and the coordinate ellipsoid is set to WGS84
     */
    explicit AVSSRRadarObsTrafo(const QString& name = QString::null)
        : AVSSRTrafo(name) {};

    //! destroys the object
    ~AVSSRRadarObsTrafo() override{};

    //! convert u/v/w to lat/long/h in degrees/meters
    /*! Calls AVSSRTrafo::radarObs2Geod().
     */
    bool uvw2latlonh(const double &rho, const double &theta, const double &H, double &geod_lat, double &geod_lon,
                     double &geod_h) const override
    {
        return radarObs2Geod(rho, theta, H, geod_lat, geod_lon, geod_h);
    }

    //! lat/long in degrees/meters to u/v/w
    /*! Calls AVSSRTrafo::geod2RadarObs().
     */
    bool latlonh2uvw(const double &geod_lat, const double &geod_lon, const double &geod_h, double &rho, double &theta,
                     double &H) const override
    {
        return geod2RadarObs(geod_lat, geod_lon, geod_h, rho, theta, H);
    }
};

///////////////////////////////////////////////////////////////////////////////

class AVCALCULATIONLIB_EXPORT AVSSRRadarCartTrafo : public AVSSRTrafo
{
public:

    /*! creates a new transformation object
        as default the radar position is set to (0.,0.,0.)
        and the coordinate ellipsoid is set to WGS84
     */
    explicit AVSSRRadarCartTrafo(const QString& name = QString::null)
        : AVSSRTrafo(name) {};

    //! destroys the object
    ~AVSSRRadarCartTrafo() override{};

    //! convert u/v/w to lat/long/h in degrees/meters
    /*! Calls AVSSRTrafo::radarCart2Geod().
     */
    bool uvw2latlonh(const double &xr, const double &yr, const double &zr, double &geod_lat, double &geod_lon,
                     double &geod_h) const override
    {
        return radarCart2Geod(xr, yr, zr, geod_lat, geod_lon, geod_h);
    }

    //! lat/long in degrees/meters to u/v/w
    /*! Calls AVSSRTrafo::geod2RadarCart().
     */
    bool latlonh2uvw(const double &geod_lat, const double &geod_lon, const double &geod_h, double &xr, double &yr,
                     double &zr) const override
    {
        return geod2RadarCart(geod_lat, geod_lon, geod_h, xr, yr, zr);
    }
};

///////////////////////////////////////////////////////////////////////////////

class AVCALCULATIONLIB_EXPORT AVSSRRadarCartHeightTrafo : public AVSSRTrafo
{
public:

    /*! creates a new transformation object
        as default the radar position is set to (0.,0.,0.)
        and the coordinate ellipsoid is set to WGS84
     */
    explicit AVSSRRadarCartHeightTrafo(const QString& name = QString::null)
        : AVSSRTrafo(name) {};

    //! destroys the object
    ~AVSSRRadarCartHeightTrafo() override{};

    //! convert u/v/w to lat/long/h in degrees/meters
    /*! Calls AVSSRTrafo::radarCartHeight2Geod().
     */
    bool uvw2latlonh(const double &rho, const double &theta, const double &H, double &geod_lat, double &geod_lon,
                     double &geod_h) const override
    {
        return radarCartHeight2Geod(rho, theta, H, geod_lat, geod_lon, geod_h);
    }

    //! lat/long in degrees/meters to u/v/w
    /*! Calls AVSSRTrafo::geod2RadarCartHeight().
     */
    bool latlonh2uvw(const double &geod_lat, const double &geod_lon, const double &geod_h, double &rho, double &theta,
                     double &H) const override
    {
        return geod2RadarCartHeight(geod_lat, geod_lon, geod_h, rho, theta, H);
    }
};

///////////////////////////////////////////////////////////////////////////////

class AVCALCULATIONLIB_EXPORT AVSMRRadarObsTrafo : public AVSSRTrafo
{
public:

    /*! creates a new transformation object
        as default the radar position is set to (0.,0.,0.)
        and the coordinate ellipsoid is set to WGS84
        \param radar_height_above_ground radar height above ground in meter
     */
    explicit AVSMRRadarObsTrafo(const QString& name = QString::null,
                                double radar_height_above_ground = 0.0)
        : AVSSRTrafo(name),
          m_radar_height_above_ground(radar_height_above_ground) {}

    //! destroys the object
    ~AVSMRRadarObsTrafo() override {}

    //! set the radar height above ground in meter
    virtual void setRadarHeightAboveGround(double height) { m_radar_height_above_ground = height; }

    //! returns the radar height above ground in meter
    virtual double getRadarHeightAboveGround() const { return m_radar_height_above_ground; }

    //! convert u/v/w to lat/long/h in degrees/meters
    /*! Calls AVSSRTrafo::radarObsZ2Geod().
     */
    bool uvw2latlonh(const double &rho, const double &theta, const double &dummy, double &geod_lat, double &geod_lon,
                     double &geod_h) const override
    {
        Q_UNUSED(dummy);
        return radarObsZ2Geod(rho, theta, -m_radar_height_above_ground,
                              geod_lat, geod_lon, geod_h);
    }

    //! lat/long in degrees/meters to u/v/w
    /*! Calls AVSSRTrafo::geod2RadarObsZ().
     */
    bool latlonh2uvw(const double &geod_lat, const double &geod_lon, const double &geod_h, double &rho, double &theta,
                     double &dummy) const override
    {
        return geod2RadarObsZ(geod_lat, geod_lon, geod_h, rho, theta, dummy);
    }

protected:

    double m_radar_height_above_ground; //!< height of radar above ground in meter
};

#endif

// End of file
