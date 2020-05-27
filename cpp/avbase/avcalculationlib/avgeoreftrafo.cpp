///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Dr. Thomas Leitner, t.leitner@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Implamant a coordinate transformation class based on AVGeoRef for
             arbitrary projections and transformations based on tie points.
*/


// QT
#include <qfileinfo.h>

// local includes
#include "avgeoref.h"
#include "avgeoreftrafo.h"
#include "avlog.h"
#include "avmd5.h"


///////////////////////////////////////////////////////////////////////////////

AVGeoRefTrafo::AVGeoRefTrafo() : AVCoordinateTransformBase("GeoRefTrafo"),
    m_georef(0),
    m_delete_georef(true)
{
    m_georef = new AVGeoRef();
    AVASSERT(m_georef);

    if (!m_georef->isValid()) {
        m_georef->readTiePointFile("avgeoreftrafo.udl", false);
    }
}

///////////////////////////////////////////////////////////////////////////////

AVGeoRefTrafo::AVGeoRefTrafo(AVGeoRef* georef, bool delete_here)
    : AVCoordinateTransformBase("GeoRefTrafo"),
      m_georef(georef),
      m_delete_georef(delete_here)
{
    AVASSERT(m_georef);
}

///////////////////////////////////////////////////////////////////////////////

AVGeoRefTrafo::~AVGeoRefTrafo()
{
    if (m_delete_georef) delete m_georef;
}

///////////////////////////////////////////////////////////////////////////////

bool AVGeoRefTrafo::getResFactor(double &res) const
{
    res = 1.0;
    return m_georef->isValid();
}

///////////////////////////////////////////////////////////////////////////////

bool AVGeoRefTrafo::xy2latlong(double x, double y, double &la, double &lo) const
{
    if (!m_georef->isValid()) return false;

    m_georef->xy2uv(x, y, la, lo);
    return m_georef->isValid();
}

///////////////////////////////////////////////////////////////////////////////

bool AVGeoRefTrafo::latlong2xy(double la, double lo, double &x, double &y) const
{
    if (!m_georef->isValid()) return false;

    m_georef->uv2xy(la, lo, x, y);
    return m_georef->isValid();
}

///////////////////////////////////////////////////////////////////////////////

QString AVGeoRefTrafo::uniqueName() const
{
    QFileInfo file_info( m_georef->getTiePointFileName() );
    QString md5_hash;

    if( !AVMD5EncodeHexFromFile( file_info.absoluteFilePath(), md5_hash ) )
    {
        LOGGER_ROOT.Write(LOG_ERROR, "AVGeoRefTrafo::uniqueName: "
                         "could not create md5 sum from file %s",
                          qPrintable(file_info.absoluteFilePath()) );
    }

    LOGGER_ROOT.Write(LOG_DEBUG,
                      "AVGeoRefTrafo::uniqueName: tie point file name '%s'",
                      qPrintable(file_info.absoluteFilePath()) );

    QString name = m_name + file_info.fileName() + md5_hash;

    LOGGER_ROOT.Write(LOG_DEBUG,
                      "AVGeoRefTrafo::uniqueName:  '%s'", qPrintable(name) );
    return name;
}

// End of file
