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
#include "avgausskruegertrafo.h"

///////////////////////////////////////////////////////////////////////////////

AVGaussKruegerTrafo::AVGaussKruegerTrafo(double ref_mer, const AVCoordEllipsoid& ref_ell)
{
    AVASSERT(!ref_ell.isEmpty());

    m_coord_ell    = ref_ell;
    m_ref_meridian = AVSciConst::convdegTorad(ref_mer);

    double a_plus_b = m_coord_ell.geta() + m_coord_ell.getb();
    double n        = (m_coord_ell.geta() - m_coord_ell.getb())/a_plus_b;
    double n2       = n*n;
    double n4       = n2*n2;
    m_alpha         = a_plus_b/2.0*(1.0 + 0.25*n2 + 1.0/64.0*n4);
    m_beta          = 1.5*n - 27.0/32.0*n2*n + 269.0/512.0*n4*n;
    m_gamma         = 21.0/16.0*n2 - 55.0/32.0*n4;
    m_delta         = 151.0/96.0*n2*n - 417.0/128.0*n4*n;
    m_epsilon       = 1097.0/512.0*n4;

    m_alpha1        = a_plus_b/2.0*(1.0 + 0.25*n2 + 1.0/64.0*n4);
    m_beta1         = -1.5*n + 9.0/16.0*n2*n - 3.0/32.0*n4*n;
    m_gamma1        = 15.0/16.0*n2 - 15.0/32.0*n4;
    m_delta1        = -35.0/48.0*n2*n + 105.0/256.0*n4*n;
    m_epsilon1      = 315.0/512.0*n4;
}

///////////////////////////////////////////////////////////////////////////////

AVGaussKruegerTrafo::~AVGaussKruegerTrafo()
{
}

///////////////////////////////////////////////////////////////////////////////

bool AVGaussKruegerTrafo::xy2latlong(double x, double y, double &la, double &lo) const
{
    double fussp_breite = fusspunktsbreite(x);
    double cos_fussp_breite = cos(fussp_breite);
    double etaf2        = m_coord_ell.getes2()*cos_fussp_breite*cos_fussp_breite;
    double Nf           = m_coord_ell.geta()*m_coord_ell.geta()/
                          (m_coord_ell.getb()*sqrt(1.0 + etaf2));
    double tf           = tan(fussp_breite);
    double Nf2          = Nf*Nf;
    double Nf4          = Nf2*Nf2;
    double y2           = y*y;
    double y4           = y2*y2;
    double tf2          = tf*tf;
    double tf4          = tf2*tf2;
    double etaf4        = etaf2*etaf2;

    la = fussp_breite +
         tf/(2.0*Nf2)*(-1.0 - etaf2)*y2 +
         tf/(24.0*Nf4)*(5.0 + 3.0*tf2 + 6.0*etaf2 - 6.0*tf2*etaf2 - 3.0*etaf4 - 9.0*tf2*etaf4)*y4 +
         tf/(720.0*Nf4*Nf2)*(-61.0 - 90.0*tf2 - 45.0*tf4 - 107.0*etaf2 + 162.0*tf2*etaf2 +
                             45.0*tf4*etaf2)*y4*y2 +
         tf/(40320.0*Nf4*Nf4)*(1385.0 + 3633.0*tf2 + 4095.0*tf4 + 1575.0*tf4*tf2)*y4*y4;

    lo = m_ref_meridian +
         y/(Nf*cos_fussp_breite) +
         (-1.0 - 2.0*tf2 - etaf2)*y2*y/(6.0*Nf2*Nf*cos_fussp_breite) +
         (5.0 + 28.0*tf2 + 24.0*tf4 + 6.0*etaf2 + 8.0*tf2*etaf2)*y4*y/
         (120.0*Nf4*Nf*cos_fussp_breite) +
         (-61.0 - 662.0*tf2 - 1320.0*tf4 - 720.0*tf4*tf2)*y4*y2*y/
         (5040.0*Nf4*Nf2*Nf*cos_fussp_breite);

    la = AVSciConst::convradTodeg(la);
    lo = AVSciConst::convradTodeg(lo);

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVGaussKruegerTrafo::latlong2xy(double la, double lo, double &x, double &y) const
{
    la = AVSciConst::convdegTorad(la);
    lo = AVSciConst::convdegTorad(lo);

    double mer_bogen = meridianbogenlaenge(la);
    double cos_la    = cos(la);
    double eta2      = m_coord_ell.getes2()*cos_la*cos_la;
    double N         = m_coord_ell.geta()*m_coord_ell.geta()/
                       (m_coord_ell.getb()*sqrt(1.0 + eta2));
    double t         = tan(la);
    double l         = lo - m_ref_meridian;
    double cos2_la   = cos_la*cos_la;
    double cos4_la   = cos2_la*cos2_la;
    double l2        = l*l;
    double l4        = l2*l2;
    double t2        = t*t;
    double t4        = t2*t2;

    x = mer_bogen +
        t/2.0*N*cos2_la*l2 +
        t/24.0*N*cos4_la*(5.0 - t2 + 9.0*eta2 + 4.0*eta2*eta2)*l4 +
        t/720.0*N*cos4_la*cos2_la*(61.0 - 58.0*t2 + t4 + 270.0*eta2 - 330.0*t2*eta2)*l4*l2 +
        t/40320.0*N*cos4_la*cos4_la*(1385.0 - 3111.0*t2 + 543.0*t4 - t4*t2)*l4*l4;

    y = N*cos_la*l +
        N*cos2_la*cos_la*(1.0 - t2 + eta2)*l2*l/6.0 +
        N*cos4_la*cos_la*(5.0 - 18.0*t2 + t4 + 14.0*eta2 - 58.0*t2*eta2)*l4*l/120.0 +
        N*cos4_la*cos2_la*cos_la*(61.0 - 479.0*t2 + 179.0*t4 - t4*t2)*l4*l2*l/5040.0;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

double AVGaussKruegerTrafo::meridianbogenlaenge(double phi) const
{
    return m_alpha1*(phi + m_beta1*sin(2.0*phi) + m_gamma1*sin(4.0*phi) +
            m_delta1*sin(6.0*phi) + m_epsilon1*sin(8.0*phi));
}

///////////////////////////////////////////////////////////////////////////////

double AVGaussKruegerTrafo::fusspunktsbreite(double x) const
{
    double p1       = x/m_alpha;
    return p1 + m_beta*sin(2.0*p1) + m_gamma*sin(4.0*p1) + m_delta*sin(6.0*p1) +
            m_epsilon*sin(8.0*p1);
}

// End of file
