///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2013
//
// Module:    AVCALCULATIONLIB - AviBit Calculation Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Ulrich Feichter, u.feichter@avibit.com
    \author  QT4-PORT: Ulrich Feichter, u.feichter@avibit.com
    \brief   Error ellipse calculations from variances and covariance
 */

#ifndef AVERRORELLIPSE_INCLUDED
#define AVERRORELLIPSE_INCLUDED

// avlib includes
#include "avcalculationlib_export.h"
#include "avmacros.h"

// local includes

// forward declarations


////////////////////////////////////////////////////////////////////////////
//! Class containing able to calculate the error ellipse values for a two dimensional Gaussian distribution.
/*! A two dimensional Gaussian distribution is described by its mean values and its covariance matrix Q consisting of:
 *  \verbatim
 *      [  var_x  cov_xy ]
 *  Q = [  cov_yx var_y  ]
 *  \verbatim
 *  where var_x is the variance (sigma squared) in x direction, and var_y the variance in y direction. cov_xy and cov_yx
 *  must be equal and give the covariance between x and y.
 *
 *  Areas of constant standard deviation of the distribution can be approximated by an ellipse. The semiaxs lengths
 *  and the orientation of the major semiaxis can be calculated with this class.
 */
class AVCALCULATIONLIB_EXPORT AVErrorEllipse
{
    static const double MAX_DOUBLE_DEVIATION;

public:

    enum SemiAxisType
    {
        AMT_XAXIS = 0, //!< x semi axis of error ellipse before rotation
        AMT_YAXIS = 1, //!< y semi axis of error ellipse before rotation
        AMT_MAJOR = 2, //!< major semi axis of error ellipse
        AMT_MINOR = 3  //!< minor semi axis of error ellipse
    };

    //! returns rotation of the error ellipse major semiaxis in [rad], given a variance in x, a variance in y and a
    //! covariance
    /*! The return value lies between -pi/2 and pi/2 and is positive for rotation in counter-clockwise direction.
     *  0 means along to X axis (mathematical angle)
     */
    static double calcErrorEllipseRotation(const double &var_x, const double &var_y, const double &cov_xy);

    //! returns the length of the respective error ellipse semiaxis, given a variance in x, a variance in y and a
    //! covariance
    static double calcErrorEllipseSemiAxisLength(const double &var_x, const double &var_y, const double &cov_xy,
                                                 SemiAxisType semi_axis_type = AMT_MAJOR);
};

#endif // AVERRORELLIPSE_INCLUDED

// End of file
