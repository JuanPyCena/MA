///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2011
//
// Module:    AVCALCULATIONLIB - AviBit Calculation Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Clemens Ender, c.ender@avibit.com
    \author  Dr. Thomas Leitner, t.leitner@avibit.com
    \author  Thomas Pock, t.pock@avibit.com
    \author  Alexander Randeu, a.randeu@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Define system transformation from lat/long (WGS84) to a
             system x/y coordinate systems in meters.
*/

#if !defined(AVSYSTEMTRAFO_H_INCLUDED)
#define AVSYSTEMTRAFO_H_INCLUDED

// AVLib includes
#include "avcalculationlib_export.h"
#include "avcoordinatetransformbase.h"

// Local includes
#include <cmath>

///////////////////////////////////////////////////////////////////////////////
//! Implement system transformation from lat/long (WGS84) to a system x/y
//! coordinate systems in meters.
/*!
    Note that the system X/Y system coordinates has its origin at 47.5/14.0 if
    the default constructor is used and there is no origin.cfg file.
*/
class AVCALCULATIONLIB_EXPORT AVSystemTrafo : public AVCoordinateTransformBase {
public:
    //! Default constructor. Creates an AVSystemTrafo object
    /*! The origin will be initialized with the values configured in the default "origin.cfg" file.
        If that file does not exist the default values (47.5 / 14.0) are used.
        Note: The default "origin.cfg" file will only be loaded once, during the first
        AVSystemTrafo construction.
    */
    AVSystemTrafo();

    //! Creates an AVSystemTrafo object
    /*! The origin will be initialized with the values configured in the specified origin
        configuration file. If that file does not exist the default values (47.5 / 14.0) are used.
    */
    explicit AVSystemTrafo(const QString& cfg_file_name);

    //! Creates an AVSystemTrafo object
    /*! The origin will be initialized with the lat/long values passed as parameters in degrees.
    */
    explicit AVSystemTrafo(double rlat, double rlong);

    //! destroys the AVSystemTrafo object
    ~AVSystemTrafo() override{};

    //! convert x/y system coordinates in meter to lat/long (in degrees)
    //! based on WGS84 with the origin defined in the constructor
    bool xy2latlong(double x, double y, double &la, double &lo) const override;

    //! convert lat/long in degrees to x/y system coordinates in meter,
    //! with the origin defined in the constructor
    bool latlong2xy(double la, double lo, double &x, double &y) const override;

    //! return x/y to m resolution scaling factor (meters per pixel)
    bool getResFactor(double &res) const override;

    //! returns the unique trafo name
    //! the trafo name is built as follows:
    //!       trafo name + rlat + rlong ,
    //!       whereas the trafo name can be set using the setName() method.
    QString uniqueName() const override;

    //! returns a string representation of this trafo (origin in LAT/LON [deg])
    virtual QString toString() const;

    //! conversion parameters
    static const double RES;

protected:
	static const double A;
	static const double B;
	static const double E2;
	static double       DEFAULT_LATP;
	static double		DEFAULT_LONGP;
	static double		DEFAULT_EN;
    static bool         DEFAULT_ORIGIN_FILE_LOADED;
	double              m_latp;
	double				m_longp;
    double              m_en;

    //! convert lat/long in degrees to x/y system coordinates in meter,
    //! with the specified origin.
    /*! \note this function is time consuming, because it always recalculates
        en for the given origin!
     */
    virtual bool latlong2xyRef(double la, double lo, double &x, double &y,
                               double rla, double rlo) const;

    //! convert lat/long in radians to x/y system coordinates in meters,
    //! with the origin defined in the constructor
    bool latlong2xy_rad(double la, double lo, double &x, double &y) const;

    //! convert lat/long in radians to x/y system coordinates in meter,
    //! with the specified origin [rad]
    bool latlong2xyRef_rad(double la, double lo, double &x, double &y,
                           double rla, double rlo, double en) const;

    //! convert x/y system coordinates in meters to lat/long in radians,
    //! with the origin defined in the constructor
    bool xy2latlong_rad(double x, double y, double &la, double &lo) const;

    //! convert x/y system coordinates in meters to lat/long in radians,
    //! with the specified origin [rad]
    bool xy2latlongRef_rad(double x, double y, double &la, double &lo,
                           double rla, double rlo, double en) const;
};

#endif

// End of file
