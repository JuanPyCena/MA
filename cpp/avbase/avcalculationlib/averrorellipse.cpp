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


#include "averrorellipse.h"

#include "avlog.h"

/////////////////////////////////////////////////////////////////////////////

const double AVErrorEllipse::MAX_DOUBLE_DEVIATION = 0.00001;

/////////////////////////////////////////////////////////////////////////////

double AVErrorEllipse::calcErrorEllipseRotation(const double &var_x, const double &var_y, const double &cov_xy)
{
    // the error ellipse is degenerate or (almost) circle
    if (var_x < MAX_DOUBLE_DEVIATION ||
        var_y < MAX_DOUBLE_DEVIATION ||
        fabs(var_x - var_y) < MAX_DOUBLE_DEVIATION)
    {
        return 0;
    }

    return 0.5 * atan( -2*cov_xy / (var_x - var_y) );
}

/////////////////////////////////////////////////////////////////////////////

double AVErrorEllipse::calcErrorEllipseSemiAxisLength(const double &var_x, const double &var_y, const double &cov_xy,
                                                      AVErrorEllipse::SemiAxisType semi_axis_type)
{
    double trace(var_x + var_y);
    double det(var_x * var_y - cov_xy * cov_xy);

    // abs avoids -0.0 values
    double quad_term(sqrt(fabs(trace*trace/4 - det)));
    double eig1(trace/2 + quad_term);
    double eig2(trace/2 - quad_term);

    if (eig1 < 0 || eig2 < 0)
    {
        AVLogger->Write(LOG_ERROR,"AVErrorEllipse::getErrorEllipseSemiAxisLength: "
            "given covariance matrix is not positive-semidefinite, returning 0");
        return 0.0;
    }

    double semi_axis_length(0.0);
    switch(semi_axis_type)
    {
    case AMT_XAXIS:
        semi_axis_length = (var_x > var_y) ?
            sqrt(AVmax<double>(eig1,eig2)) : sqrt(AVmin<double>(eig1,eig2));
        break;
    case AMT_YAXIS:
        semi_axis_length = (var_y > var_x) ?
            sqrt(AVmax<double>(eig1,eig2)) : sqrt(AVmin<double>(eig1,eig2));
        break;
    case AMT_MAJOR:
        semi_axis_length = sqrt(AVmax<double>(eig1,eig2));
        break;
    case AMT_MINOR:
        semi_axis_length = sqrt(AVmin<double>(eig1,eig2));
        break;
    default:
        break;
    }

    return semi_axis_length;
}


// End of file
