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


#include <cmath>
#include <cstdlib>
#include <qfileinfo.h>

#include "avenvironment.h"
#include "avlog.h"
#include "avgeoref.h"


///////////////////////////////////////////////////////////////////////////////

AVGeoRef::AVGeoRef(const QString &fn, bool radflg)
    : tie_point_fn( fn )
{
    ok = false;
    number_tp = pol_xnum = pol_ynum = pol_xexp = pol_yexp = 0;

    int index1=0;
    int index2=0;

    for(index1=0; index1 < MAX_ORD_POLY; ++index1)
        for(index2=0; index2 < MAX_ORD_POLY; ++index2)
    {
        pol_x[index1][index2] = false;
        pol_y[index1][index2] = false;
        pol_xl[index1][index2] = false;
        pol_yl[index1][index2] = false;
        pol_xinv[index1][index2] = false;
        pol_yinv[index1][index2] = false;
    }

    for(index1=0; index1 < MAX_POINTS; ++index1)
    {
        tp_xin[index1] = 0.0;
        tp_yin[index1] = 0.0;
        tp_xref[index1] = 0.0;
        tp_yref[index1] = 0.0;
    }

    readTiePointFile(tie_point_fn, radflg);
}

///////////////////////////////////////////////////////////////////////////////

AVGeoRef::AVGeoRef(int pol_type, int n, int *x, int *y, double *u, double *v)
{
    ok = false;
    number_tp = pol_xnum = pol_ynum = pol_xexp = pol_yexp = 0;

    int index1=0;
    int index2=0;

    for(index1=0; index1 < MAX_ORD_POLY; ++index1)
        for(index2=0; index2 < MAX_ORD_POLY; ++index2)
    {
        pol_x[index1][index2] = false;
        pol_y[index1][index2] = false;
        pol_xl[index1][index2] = false;
        pol_yl[index1][index2] = false;
        pol_xinv[index1][index2] = false;
        pol_yinv[index1][index2] = false;
    }

    for(index1=0; index1 < MAX_POINTS; ++index1)
    {
        tp_xin[index1] = 0.0;
        tp_yin[index1] = 0.0;
        tp_xref[index1] = 0.0;
        tp_yref[index1] = 0.0;
    }

    ok = init(pol_type, n, x, y, u, v);
}

///////////////////////////////////////////////////////////////////////////////

AVGeoRef::AVGeoRef()
{
    ok = false;
    number_tp = pol_xnum = pol_ynum = pol_xexp = pol_yexp = 0;

    int index1=0;
    int index2=0;

    for(index1=0; index1 < MAX_ORD_POLY; ++index1)
        for(index2=0; index2 < MAX_ORD_POLY; ++index2)
    {
        pol_x[index1][index2] = false;
        pol_y[index1][index2] = false;
        pol_xl[index1][index2] = false;
        pol_yl[index1][index2] = false;
        pol_xinv[index1][index2] = false;
        pol_yinv[index1][index2] = false;
    }

    for(index1=0; index1 < MAX_POINTS; ++index1)
    {
        tp_xin[index1] = 0.0;
        tp_yin[index1] = 0.0;
        tp_xref[index1] = 0.0;
        tp_yref[index1] = 0.0;
    }
}

///////////////////////////////////////////////////////////////////////////////

AVGeoRef::~AVGeoRef()
{
}

///////////////////////////////////////////////////////////////////////////////

bool AVGeoRef::readTiePointFile(const QString &fn, bool radflg)
{
    int pol_type, x[MAX_POINTS], y[MAX_POINTS];
    double u[MAX_POINTS], v[MAX_POINTS];

    // sanity checks
    if (fn.isEmpty()) return false;

    tie_point_fn =  fn;

    // construct QFile and QFileInfo objects
    QString fname = fn;
    QFile fi(fname);
    QFileInfo fileInfo(fi);

    // if the filename is not absolute and the file does not exist
    // we try again prepending $APP_MAPS/
    if (fileInfo.isRelative() && !fileInfo.exists()) {
        QString mapDir = AVEnvironment::getApplicationMaps();
        if (!mapDir.isNull())
        {
            fname.prepend(QString(mapDir) + "/");
            fi.setFileName(fname);
        }
    }

    // save filename
    ifn = fname;

    // try to open now
    if (!fi.exists()) return false;
    if (!fi.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

    // read description
    if (!readLine(fi, desc)) return false;

    // read image-file name
    if (!readLine(fi, ifn)) return false;

    // read tie points
    QString line;
    if (!readLine(fi, line)) return false;
    pol_type = line.toInt();
    int n = 0;
    while (readLine(fi, line)) {
        if (sscanf(qPrintable(line), "%d %d %lg %lg",
                   &x[n], &y[n], &u[n], &v[n]) == 4) {
            n++;
            if (n >= MAX_POINTS) break;
        }
    }
    fi.close();

    // convert to radians if required
    if (radflg) {
        int i;
        for (i = 0; i < n; i++) {
            u[i] *= 3.141592653589 / 180.0;
            v[i] *= 3.141592653589 / 180.0;
        }
    }

    // call internal init routine
    ok = init(pol_type, n, x, y, u, v);
    return ok;
}

///////////////////////////////////////////////////////////////////////////////

bool AVGeoRef::init(int pol_type, int n, int *x, int *y, double *u, double *v)
{
    int  i, j, ix, iy;

    // init vars.

    pol_xnum = pol_ynum = pol_xexp = pol_yexp = 0;
    number_tp = 0;
    for (i = 0; i < MAX_ORD_POLY; i++) {
        for (j = 0; j < MAX_ORD_POLY; j++) {
            pol_xl[i][j]   = pol_yl[i][j]   = false;
            pol_x[i][j]    = pol_y[i][i]    = 0.0;
            pol_xinv[i][j] = pol_yinv[i][i] = 0.0;
        }
    }
    for (i = 0; i < MAX_POINTS; i++) {
        tp_xin[i]  = tp_yin[i]  = 0.0;
        tp_xref[i] = tp_yref[i] = 0.0;
    }

    // init depending on pol_type

    if (pol_type == 1) {
        return false;                            // not available
    } else if (pol_type == 2) {
        pol_xexp = 1;
        pol_yexp = 1;
        pol_xnum = 3;
        pol_ynum = 3;
        for (ix = 0; ix < 2; ix++) {
            for (iy = 0; iy < 2; iy++) {
                pol_xl[ix][iy] = true;
                pol_yl[ix][iy] = true;
            }
        }
        pol_xl[1][1] = false;
        pol_yl[1][1] = false;
    } else if (pol_type == 3) {
        pol_xexp = 1;
        pol_yexp = 1;
        pol_xnum = 4;
        pol_ynum = 4;
        for (ix = 0; ix < 2; ix++) {
            for (iy = 0; iy < 2; iy++) {
                pol_xl[ix][iy] = true;
                pol_yl[ix][iy] = true;
            }
        }
    } else if (pol_type == 4) {
        pol_xexp = 2;
        pol_yexp = 2;
        pol_xnum = 6;
        pol_ynum = 6;
        for (ix = 0; ix < 3; ix++) {
            for (iy = 0; iy < 4 - ix; iy++) {
                pol_xl[ix][iy] = true;
                pol_yl[ix][iy] = true;
            }
        }
    } else if (pol_type == 5) {
        pol_xexp = 2;
        pol_yexp = 2;
        pol_xnum = 9;
        pol_ynum = 9;
        for (ix = 0; ix < 3; ix++) {
            for (iy = 0; iy < 3; iy++) {
                pol_xl[ix][iy] = true;
                pol_yl[ix][iy] = true;
            }
        }
    } else if (pol_type == 6) {
        pol_xexp = 3;
        pol_yexp = 3;
        pol_xnum = 16;
        pol_ynum = 16;
        for (ix = 0; ix < 4; ix++) {
            for (iy = 0; iy < 4; iy++) {
                pol_xl[ix][iy] = true;
                pol_yl[ix][iy] = true;
            }
        }
    } else {
        return false;
    }

    // check if the number of tie points it exceeded

    if (n > MAX_POINTS) return false;

    // copy tie points to local storage

    for (i = 0; i < n; i++) {
        tp_xin[i]  = static_cast<double>(x[i]);
        tp_yin[i]  = static_cast<double>(y[i]);
        tp_xref[i] = u[i];
        tp_yref[i] = v[i];
    }
    number_tp = n;

    // calculate polynomial and return

    if (mth_calccoef()) return true;
    return false;
}

///////////////////////////////////////////////////////////////////////////////

void AVGeoRef::uv2xy(double u, double v, double &x, double &y) const
{
    mth_evalpol2(true, u, v, x, y);
}

///////////////////////////////////////////////////////////////////////////////

void AVGeoRef::xy2uv(double x, double y, double &u, double &v) const
{
    mth_evalpol2(false, x, y, u, v);
}

///////////////////////////////////////////////////////////////////////////////
//                             I N T E R N A L
///////////////////////////////////////////////////////////////////////////////

double AVGeoRef::mth_clcpoly(const double poly[MAX_ORD_POLY][MAX_ORD_POLY],
                             const bool   lpoly[MAX_ORD_POLY][MAX_ORD_POLY],
                             int exp, double x_in, double y_in) const
{
    int i, j, enu;
    double outval, f1, f2;

    enu = exp + 1;
    outval = 0.0;

    f2 = 1.0;
    for (i = 0; i < enu; i++) {
        f1 = 1.0;
        for (j = 0; j < enu; j++) {
            if (lpoly[i][j]) outval = outval + poly[i][j] * f1 * f2;
            f1 *= x_in;
        }
        f2 *= y_in;
    }
    return outval;
}

///////////////////////////////////////////////////////////////////////////////

void AVGeoRef::mth_evalpol2(bool inverse, double x_in, double y_in,
                            double &xout, double &yout) const
{
    xout = x_in;
    yout = y_in;
    if (!inverse) {               // regular polynomials used
        if (pol_xnum > 0) {
            xout = mth_clcpoly(pol_x, pol_xl, pol_xexp, x_in, y_in );
        }
        if (pol_ynum > 0) {
            yout = mth_clcpoly(pol_y, pol_yl, pol_yexp, x_in, y_in );
        }
    } else {                      // inverse polynomials used
        if (pol_xnum > 0) {
            xout = mth_clcpoly(pol_xinv, pol_xl, pol_xexp, x_in, y_in );
        }
        if (pol_ynum > 0) {
            yout = mth_clcpoly(pol_yinv, pol_yl, pol_yexp, x_in, y_in );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVGeoRef::mth_gausjord(double matrix[NMATDIM][NMATDIM], int act_dim,
                            int ncol)
{
    int i, j, k;
    double pivot, f;
    for (k = 0; k < act_dim; k++) {
        pivot = matrix[k][k];
        if (fabs(pivot) < 1e-40) {
            AVLogger->Write(LOG_WARNING, "pivot < 1e-40 !!");
            return;
        }
        matrix[k][k] = 1.0;
        for (j = 0; j < ncol; j++) {
            matrix[k][j] /= pivot;
        }
        for (i = 0; i < ncol; i++) {
            if (i != k) {
                f = matrix[i][k];
                matrix[i][k] = - f / pivot;
                for (j = 0; j < ncol; j++) {
                    if (j != k) matrix[i][j] -= f * matrix[k][j];
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

bool AVGeoRef::mth_calccoef()
{
    int count, loop, k, x, y;
    static double amat[MAX_POINTS][NMATDIM], nmat[NMATDIM][NMATDIM];
    static double fact[NMATDIM];
    bool status = false;

    // Calculate Regular (K=1) and Inverse (K=2) X Polynomial

    for (k = 1; k <= 2; k++) {
        for (y = 0; y < NMATDIM; y++) {
            for (x = 0; x < NMATDIM; x++) {
                nmat[y][x] = 0.0;
            }
            for (x = 0; x < MAX_POINTS; x++) {
                amat[x][y] = 0.0;
            }
        }

        for (loop = 0; loop < number_tp; loop++) {
            if (k == 1) {
                status = mth_poly_neq(pol_xexp,      pol_xnum,
                                      tp_xin[loop],  tp_yin[loop],
                                      tp_xref[loop], &amat[loop][0],
                                      pol_xl);
            } else {
                status = mth_poly_neq(pol_xexp,      pol_xnum,
                                      tp_xref[loop], tp_yref[loop],
                                      tp_xin[loop],  &amat[loop][0],
                                      pol_xl);
            }
            if (!status) return status;
        }

        status = mth_mknmat(number_tp, pol_xnum, amat, nmat, fact);
        if (!status) return status;

        mth_gausjord(nmat, pol_xnum, pol_xnum + 1);

        count = 0;
        for (y = 0; y < pol_xexp + 1; y++) {
            for (x = 0; x < pol_xexp + 1; x++) {
                if (pol_xl[y][x]) {
                    if (k == 1) {
                        pol_x[y][x] = nmat[count][pol_xnum] / fact[count];
                    } else {
                        pol_xinv[y][x] = nmat[count][pol_xnum] / fact[count];
                    }
                    count++;
                }
            }
        }
    }

    // Calculate Regular (K=1) and Inverse (K=2) Y Polynomial

    for (k = 1; k <= 2; k++) {
        for (y = 0; y < NMATDIM; y++) {
            for (x = 0; x < NMATDIM; x++) {
                nmat[y][x] = 0.0;
            }
            for (x = 0; x < MAX_POINTS; x++) {
                amat[x][y] = 0.0;
            }
        }

        for (loop = 0; loop < number_tp; loop++) {
            if (k == 1) {
                status = mth_poly_neq( pol_yexp,      pol_ynum,
                                       tp_xin[loop],  tp_yin[loop],
                                       tp_yref[loop], &amat[loop][0],
                                       pol_yl );
            } else {
                status = mth_poly_neq( pol_yexp,      pol_ynum,
                                       tp_xref[loop], tp_yref[loop],
                                       tp_yin[loop],  &amat[loop][0],
                                       pol_yl );
            }
            if (!status) return status;
        }

        status = mth_mknmat(number_tp, pol_ynum, amat, nmat, fact );
        if (!status) return status;

        mth_gausjord(nmat, pol_ynum, pol_ynum + 1 );

        count = 0;
        for (y = 0; y < pol_yexp + 1; y++) {
            for (x = 0; x < pol_yexp + 1; x++) {
                if (pol_yl[y][x]) {
                    if (k == 1) {
                        pol_y[y][x] = nmat[count][pol_ynum] / fact[count];
                    } else {
                        pol_yinv[y][x] = nmat[count][pol_ynum] / fact[count];
                    }
                    count++;
                }
            }
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVGeoRef::mth_mknmat(int rows, int cols,
                          double amat[MAX_POINTS][NMATDIM],
                          double nmat[NMATDIM][NMATDIM],
                          double fact[NMATDIM])
{
    int i, j, k;
    double sum, r8eps = 1.0e-8;
    for (j = 0; j < cols; j++) {
        sum = 0.0;
        for (i = 0; i < rows; i++) {
            sum += amat[i][j];
        }
        if (fabs(sum) < r8eps) {
            fact[j] = 1.0;
        } else {
            fact[j] = sum / static_cast<double>(rows);
            for (i = 0; i < rows; i++) {
                amat[i][j] /= fact[j];
            }
        }
    }
    for (i = 0; i < cols + 1; i++) {
        for (j = 0; j < cols + 1; j++) {
            for (k = 0; k < rows; k++) {
                nmat[i][j] += amat[k][i] * amat[k][j];
            }
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVGeoRef::mth_poly_neq(int exp, int num_coef, double x, double y,
                            double ref, double vect[NMATDIM],
                            bool l_poly[MAX_ORD_POLY][MAX_ORD_POLY])
{
    int i, j, count = 0;
    double fi, fj;
    for (i = 0; i < exp + 1; i++) {
        if (i == 0) fi = 1.0;
        else        fi = pow(y, i);
        for (j = 0; j < exp + 1; j++) {
            if (j == 0) fj = 1.0;
            else        fj = pow(x, j);
            if (l_poly[i][j]) {
                vect[count] = fi * fj;
                count++;
                if (count == num_coef) {
                    vect[num_coef] = ref;
                    return true;
                }
            }
        }
    }
    vect[num_coef] = ref;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVGeoRef::readLine(QFile &f, QString &l)
{
    char buf[512];
    do {
        int n=f.readLine(buf,512);
        l=buf;
        if (n<0)
            return false;
    } while (l.startsWith("#"));
    return true;
}

// End of file
