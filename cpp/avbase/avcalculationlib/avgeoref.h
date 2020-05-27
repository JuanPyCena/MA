///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author Dr. Thomas Leitner, t.leitner@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief  Coordinate mapping based on tie points.
*/

#if !defined(AVGEOREF_INCLUDED)
#define AVGEOREF_INCLUDED

#include <qstring.h>
#include <qfile.h>

#include "avcalculationlib_export.h"

///////////////////////////////////////////////////////////////////////////////

#define MAX_POINTS   500          // maximum # of tie points
#define MAX_ORD_POLY 10           /* maximum polynomial order               */
#define NMATDIM      ((MAX_ORD_POLY * MAX_ORD_POLY) + 1)

class AVCALCULATIONLIB_EXPORT AVGeoRef
{
public:
    //! Initializes the georef module by reading the data (tie points) from
    //! the specified file.
    /*! \param fn filename to read.
        \param radflg use radians rather than degrees.
        \note use the routine isValid() to check if the class is initialized
        properly.
    */
    AVGeoRef(const QString &fn, bool radflg);

    //! Constructs the object by specifying the polynomial
    //! type to be used (pol_type = 1...6) and the 'n' tie points.
    /*! The tie points are given as integer x/y coordinates and the
        respective lat/long coordinates in u/v. The method uv2xy can
        subsequently be used in order to get estimates of x/y for arbitrary
        u/v coordinates. The method xy2uv provides a reverse estimate.

        \param pol_type polynomial type (1..6)
        \param n the number of tie points
        \param x integer X coordinate array of tie points
        \param y integer Y coordinate array of tie points
        \param u double lat array of tie points
        \param v double long array of tie points

        Please use method isValid() to check if the construction succeeded.
    */
    AVGeoRef(int pol_type, int n, int *x, int *y, double *u, double *v);

    //! Initializes an empty module.
    /*! constructs an empty object, use readTiePointFile() for reading
        the tie points from a data file
    */
    AVGeoRef();

    //! destroys AVGeoRef object
    ~AVGeoRef();

    //! reads a tie-point file
    /*! \param fn filename to read.
        \param radflg use radians rather than degrees.
        \returns true if successful, false if not
        \note use the routine isValid() to check if the class is initialized
        properly.
    */
    bool readTiePointFile(const QString &fn, bool radflg);

    //! Returns the description of the mapping/georeferencing data-set
    QString getDesc() const { return desc; };

    //! Returns the tie point file name
    QString getTiePointFileName() const { return tie_point_fn; }

    //! Returns the name of the associated image file.
    QString getImgFileName() const { return ifn;  };

    //! check if the class has been initialized properly.
    /*! \return true if the class is properly initialized, false if not, for
         instance if there are too many tie points, invalid polynomial type,
         mth_calccoef, file not found etc.
    */
    bool isValid() const { return ok; };

    //! map u/v coordinates to x/y.
    /*! \param u u coordinate for input
        \param v v coordinate for input
        \param x x coordinate on output
        \param y y coordinate on output
    */
    void uv2xy(double u, double v, double &x, double &y) const;

    //! map x/y coordinates to u/v.
    /*! \param x x coordinate for input
        \param y y coordinate for input
        \param u u coordinate on output
        \param v v coordinate on output
    */
    void xy2uv(double x, double y, double &u, double &v) const;

private:
    //! internal init routine
    bool init(int pol_type, int n, int *x, int *y, double *u, double *v);

    //! internal routine to read a line but skip comment lines
    bool readLine(QFile &f, QString &l);

    //! evaluate 2-dimensional polynomial; function value = result
    /*! \param poly array containing polynomial coefficients
        \param lpoly array containing logical polynomial flags
        \param exp highest used exponent
        \param x_in X input coordinate
        \param y_in Y input coordinate
        \return result of the polynomial evaluation
    */
    double mth_clcpoly(const double poly[MAX_ORD_POLY][MAX_ORD_POLY],
                       const bool   lpoly[MAX_ORD_POLY][MAX_ORD_POLY],
                       int exp, double x_in, double y_in) const;

    //! evaluate 2-dimensional polynomial
    /*! \param inverse true if inverse transformation demanded
        \param x_in x input coordinate
        \param y_in y input coordinate
        \param x_out x output coordinate
        \param y_out y output coordinate
    */
    void mth_evalpol2(bool inverse, double x_in, double y_in,
                      double &xout, double &yout) const;

    //! solve quadratic equation system using Gauss-Jordan algorithm
    //! unknowns are restored on columns ACT_DIM+1 .. NCOL of MATRIX
    /*! \param matrix    coefficient matrix
        \param act_dim   number of defined matrix coefficients
        \param ncol      number of columns to be considered
    */
    void mth_gausjord(double matrix[NMATDIM][NMATDIM], int act_dim, int ncol);

    //! calculation of polynomial coefficients
    /*! \param back   continuation flag
        \return true if successful, false if not enough points for
        the calculation
    */
    bool mth_calccoef();

    //! Reduce a matrix by dividing by the mean value of each
    //! column and multiply this matrix by its transverse.
    /*! \param rows    used number of rows
        \param cols    used number of columns
        \param amat    input matrix
        \param nmat    output matrix
        \param fact    reduction factor vector
        \return true if successful, false if not
    */
    bool mth_mknmat(int rows, int cols,
                    double amat[MAX_POINTS][NMATDIM],
                    double nmat[NMATDIM][NMATDIM],
                    double fact[NMATDIM]);

    //! Calculate normal-equations-matrix to calculate distortion
    //! polynomials-coefficients
    /*! \param exp       highest exponent of polynomial
        \param num_coef  number of coefficients
        \param x         x-coordinate of point
        \param y         y-coordinate of point
        \param ref       destination coordinate of point
        \param vect      normal equations matrix
        \param nmatdim   dimension of nmat
        \param l_poly    should this coefficient be used ?
        \return true if successful, false if not
    */
    bool mth_poly_neq(int exp, int num_coef, double x, double y,
                      double ref, double vect[NMATDIM],
                      bool l_poly[MAX_ORD_POLY][MAX_ORD_POLY]);

    // misc data
    bool    ok;
    QString desc;
    QString tie_point_fn;
    QString ifn;

    // polynomial parameters
    int    pol_xnum, pol_ynum, pol_xexp, pol_yexp;
    bool   pol_xl[MAX_ORD_POLY][MAX_ORD_POLY];
    bool   pol_yl[MAX_ORD_POLY][MAX_ORD_POLY];

    // input reference (tie) points
    int    number_tp;
    double tp_xin[MAX_POINTS],   tp_yin[MAX_POINTS];
    double tp_xref[MAX_POINTS],  tp_yref[MAX_POINTS];

    // polynomial storage
    double pol_x[MAX_ORD_POLY][MAX_ORD_POLY];
    double pol_y[MAX_ORD_POLY][MAX_ORD_POLY];
    double pol_xinv[MAX_ORD_POLY][MAX_ORD_POLY];
    double pol_yinv[MAX_ORD_POLY][MAX_ORD_POLY];
};

#endif

// End of file
