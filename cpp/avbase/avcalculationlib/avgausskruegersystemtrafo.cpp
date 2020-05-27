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

// local includes
#include "avgausskruegersystemtrafo.h"

///////////////////////////////////////////////////////////////////////////////

AVGaussKruegerSystemTrafo::AVGaussKruegerSystemTrafo(ReferenceMeridian ref_mer_enum)
    : m_ref_meridian(ref_mer_enum)
{
    // Reference Meridian
    double ref_mer = 0.0;
    switch(m_ref_meridian) {
        case RM_BESSEL_M28: {
            ref_mer = 10.0 + 20.0/60.0;
            break;
        }
        case RM_BESSEL_M31: {
            ref_mer = 13.0 + 20.0/60.0;
            break;
        }
        case RM_BESSEL_M34: {
            ref_mer = 16.0 + 20.0/60.0;
            break;
        }
        case RM_ETRS89_M6: {
            ref_mer = 6.0;
            break;
        }
        case RM_ETRS89_M9: {
            ref_mer = 9.0;
            break;
        }
        case RM_ETRS89_M12:
        case RM_ETRS89_M12NUERNBERG:
        case RM_BESSEL_M12NUERNBERG: {
            ref_mer = 12.0;
            break;
        }
        case RM_ETRS89_M15: {
            ref_mer = 15.0;
            break;
        }
        case RM_KRASSOVSKI1940_M24_TALLINN: {
            ref_mer = 24.0;
            break;
        }
        case RM_KRASSOVSKI_42_83_M12_GERMANY: {
            ref_mer = 12.0;
            break;
        }
        default: {
            AVASSERT(false);
            break;
        }
    }

    switch(m_ref_meridian) {
        case RM_BESSEL_M28:
        case RM_BESSEL_M31:
        case RM_BESSEL_M34: {
            // GK Trafo
            AVCoordEllipsoid ell_bessel(AVCoordEllipsoid::ET_BESSEL);
            m_gk_trafo = new AVGaussKruegerTrafo(ref_mer, ell_bessel);
            AVASSERT(m_gk_trafo != 0);

            // earth datum
            m_earth_datum = new AVEarthDatum(AVEarthDatum::ED_WGS84_TO_MGI);
            AVASSERT(m_earth_datum != 0);
            break;
        }
        case RM_BESSEL_M12NUERNBERG: {
            // GK Trafo
            AVCoordEllipsoid ell_bessel(AVCoordEllipsoid::ET_BESSEL);
            m_gk_trafo = new AVGaussKruegerTrafo(ref_mer, ell_bessel);
            AVASSERT(m_gk_trafo != 0);

            // earth datum
            m_earth_datum = new AVEarthDatum(AVEarthDatum::ED_WGS84_TO_BESSEL_POTSDAM);
            AVASSERT(m_earth_datum != 0);
            break;
        }
        case RM_ETRS89_M6:
        case RM_ETRS89_M9:
        case RM_ETRS89_M12:
        case RM_ETRS89_M12NUERNBERG:
        case RM_ETRS89_M15: {
            // GK Trafo
            AVCoordEllipsoid ell_grs80(AVCoordEllipsoid::ET_GRS80);
            m_gk_trafo = new AVGaussKruegerTrafo(ref_mer, ell_grs80);
            AVASSERT(m_gk_trafo != 0);

            // earth datum
            m_earth_datum = new AVEarthDatum(AVEarthDatum::ED_WGS84_TO_ETRS89);
            AVASSERT(m_earth_datum != 0);
            break;
        }
        case RM_KRASSOVSKI1940_M24_TALLINN: {
            // GK Trafo
            AVCoordEllipsoid ell_kras40(AVCoordEllipsoid::ET_KRASSOVSKI40);
            m_gk_trafo = new AVGaussKruegerTrafo(ref_mer, ell_kras40);
            AVASSERT(m_gk_trafo != 0);

            // earth datum
            m_earth_datum = new AVEarthDatum(AVEarthDatum::ED_WGS84_TO_PULKOVO42_KRASSOVSKI40);
            AVASSERT(m_earth_datum != 0);
            break;
        }
        case RM_KRASSOVSKI_42_83_M12_GERMANY: {
            // GK Trafo
            AVCoordEllipsoid ell_kras40(AVCoordEllipsoid::ET_KRASSOVSKI40);
            m_gk_trafo = new AVGaussKruegerTrafo(ref_mer, ell_kras40);
            AVASSERT(m_gk_trafo != 0);

            // earth datum
            m_earth_datum =
                    new AVEarthDatum(AVEarthDatum::ED_WGS84_TO_PULKOVO42_KRASSOVSKI40_42_83);
            AVASSERT(m_earth_datum != 0);
            break;
        }
        default: {
            AVASSERT(false);
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

AVGaussKruegerSystemTrafo::~AVGaussKruegerSystemTrafo()
{
    delete m_gk_trafo;
    delete m_earth_datum;
}

///////////////////////////////////////////////////////////////////////////////

bool AVGaussKruegerSystemTrafo::xy2latlong(double x, double y, double &la, double &lo) const
{
    // remove offset from coordinates if needed
    double offset_x = 0.0;
    double offset_y = 0.0;
    switch (m_ref_meridian) {
        case RM_ETRS89_M6: {
            offset_y = -2500000.0;
            break;
        }
        case RM_ETRS89_M9: {
            offset_y = -3500000.0;
            break;
        }
        case RM_ETRS89_M12: {
            offset_y = -4500000.0;
            break;
        }
        case RM_ETRS89_M12NUERNBERG:
        case RM_BESSEL_M12NUERNBERG: {
            offset_x = 5000000.0;
            offset_y = -500000.0;
            break;
        }
        case RM_ETRS89_M15: {
            offset_y = -5500000.0;
            break;
        }
        case RM_KRASSOVSKI1940_M24_TALLINN: {
            offset_x = 6536000.0;
            offset_y = -24000.0;
            break;
        }
        case RM_KRASSOVSKI_42_83_M12_GERMANY: {
            offset_y = -4500000.0;
            break;
        }
        default: {
            break;
        }
    }

    x += offset_x;
    y += offset_y;

    // GK Trafo
    double la1 = 0.0;
    double lo1 = 0.0;
    if (!m_gk_trafo->xy2latlong(x, y, la1, lo1))
    {
        AVLogger->Write(LOG_ERROR, "AVGaussKruegerSystemTrafo::xy2latlong: "
            "Cannot convert x/y (%f/%f) to lat/lon", x, y);
        return false;
    }

    // assume h=0
    double h1 = 0.0;
    double h = 0.0;

    // earth datum
    if (!m_earth_datum->fromReference(la1, lo1, h1, la, lo, h))
    {
        AVLogger->Write(LOG_ERROR, "AVGaussKruegerSystemTrafo::xy2latlong: "
            "Cannot convert datum for lat/lon (%f/%f)", la1, lo1);
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVGaussKruegerSystemTrafo::latlong2xy(double la, double lo, double &x, double &y) const
{
    // earth datum
    double la1 = 0.0;
    double lo1 = 0.0;

    // assume h=0
    double h1 = 0.0;
    double h = 0.0;

    if (!m_earth_datum->toReference(la, lo, h, la1, lo1, h1))
    {
        AVLogger->Write(LOG_ERROR, "AVGaussKruegerSystemTrafo::latlong2xy: "
            "Cannot convert datum for lat/lon (%f/%f)", la1, lo1);
        return false;
    }

    // GK Trafo
    if (!m_gk_trafo->latlong2xy(la1, lo1, x, y))
    {
        AVLogger->Write(LOG_ERROR, "AVGaussKruegerSystemTrafo::latlong2xy: "
            "Cannot convert x/y (%f/%f) to lat/lon", x, y);
        return false;
    }

    // add offset to coordinates if needed
    double offset_x = 0.0;
    double offset_y = 0.0;
    switch (m_ref_meridian) {
        case RM_ETRS89_M6: {
            offset_y = 2500000.0;
            break;
        }
        case RM_ETRS89_M9: {
            offset_y = 3500000.0;
            break;
        }
        case RM_ETRS89_M12: {
            offset_y = 4500000.0;
            break;
        }
        case RM_ETRS89_M12NUERNBERG:
        case RM_BESSEL_M12NUERNBERG: {
            offset_x = -5000000.0;
            offset_y = 500000.0;
            break;
        }
        case RM_ETRS89_M15: {
            offset_y = 5500000.0;
            break;
        }
        case RM_KRASSOVSKI1940_M24_TALLINN: {
            offset_x = -6536000.0;
            offset_y = 24000.0;
            break;
        }
        case RM_KRASSOVSKI_42_83_M12_GERMANY: {
            offset_y = 4500000.0;
            break;
        }
        default: {
            break;
        }
    }

    x += offset_x;
    y += offset_y;

    return true;
}

// End of file
