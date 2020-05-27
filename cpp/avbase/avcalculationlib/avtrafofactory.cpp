///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Andreas Schuller, a.schuller@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Implementation of the AVTrafoFactory class
*/


// Qt includes


// AVLib includes

#include "avgausskruegertrafo.h"
#include "avutmtrafo.h"
#include "avgeoref.h"
#include "avgeoreftrafo.h"
#include "avpath.h"
#include "avmeteosattrafo.h"
#include "avsystemtrafo.h"
#include "avweatherradartrafo.h"
#include "avlog.h"


// local includes
#include "avtrafofactory.h"


///////////////////////////////////////////////////////////////////////////////
// Trafo Factory
///////////////////////////////////////////////////////////////////////////////

const QString AVTrafoFactory::SYSTEM_TRAFO        = "SystemTrafo";
const QString AVTrafoFactory::GEO_REF_TRAFO       = "GeoRefTrafo";
const QString AVTrafoFactory::METEO_SAT_TRAFO     = "MeteoSatTrafo";
const QString AVTrafoFactory::WEATHER_RADAR_TRAFO = "WeatherRadarTrafo";
const QString AVTrafoFactory::GAUSS_KRUEGER_TRAFO = "GaussKruegerTrafo";
const QString AVTrafoFactory::GAUSS_KRUEGER_SYSTEM_TRAFO = "GaussKruegerSystemTrafo";
const QString AVTrafoFactory::UTM_TRAFO           = "UtmTrafo";
const QString AVTrafoFactory::UTM_SYSTEM_TRAFO    = "UtmSystemTrafo";

///////////////////////////////////////////////////////////////////////////////

AVTrafoFactory::AVTrafoFactory()
    : m_trafoCreators(0)
{
    m_trafoCreators = new CallBackMap;
    AVASSERT(m_trafoCreators);

    initialize();
}

///////////////////////////////////////////////////////////////////////////////

AVTrafoFactory::~AVTrafoFactory()
{
    qDeleteAll(m_trafoPool);
    delete m_trafoCreators;
}

///////////////////////////////////////////////////////////////////////////////

AVCoordinateTransformBase* AVTrafoFactory::
createSystemTrafo(const QString& name, const QStringList& params)
{
     if( params.count() == 2)
     {
         bool convOk = false;

         double rlat  = params[0].toDouble(&convOk);

         if(!convOk)
         {
            AVLogError <<  "AVTrafoFactory::createSystemTrafo: couldn't create trafo with name '"
                       << name << "', because the first parameter in parameter list '"
                       << params.join(", ") << "' is malformed";
         }

         double rlong = params[1].toDouble(&convOk);

         if(!convOk)
         {
            AVLogError <<  "AVTrafoFactory::createSystemTrafo: couldn't create trafo with name '"
                       << name << "', because the second parameter in parameter list '"
                       << params.join(", ") << "' is malformed";
         }

         AVCoordinateTransformBase*trafo = new (LOG_HERE) AVSystemTrafo(rlat, rlong);
         trafo->setName(name);
         return trafo;
     }

     if(params.empty())
     {
        AVCoordinateTransformBase* trafo = new (LOG_HERE) AVSystemTrafo();
        trafo->setName(name);
        return trafo;
     }

     AVLogError << "AVTrafoFactory::createSystemTrafo: couldn't create trafo with name '"
                << name << "', because the parameter list has an invalid size of "
                << params.count();
     return 0;
}

///////////////////////////////////////////////////////////////////////////////

AVCoordinateTransformBase* AVTrafoFactory::
createGeoRefTrafo(const QString& name, const QStringList& params)
{
    if(params.count() == 1)
    {
        QString tiePointFile = params[0];

        QFileInfo geoRefFile(AVPath(tiePointFile).expandPath());

        if (!geoRefFile.exists() || !geoRefFile.isReadable())
        {
            AVLogError << "AVTrafoFactory::createGeoRefTrafo: can not load geo reference data "
                          "because geo reference data file '" << geoRefFile.absoluteFilePath()
                       << "' does not exist or is not readable";
            return 0;
        }

        AVGeoRef* geoRefReader = new AVGeoRef();
        AVASSERT(geoRefReader != 0);

        if (!geoRefReader->readTiePointFile(geoRefFile.absoluteFilePath(), false))
        {
            AVLogError << "AVTrafoFactory::createGeoRefTrafo: can not load geo reference data "
                          "because geo reference data from file '"
                       << geoRefFile.absoluteFilePath() << "' is not valid";
            delete geoRefReader;
            return 0;
        }

        AVCoordinateTransformBase* trafo  = 0;
        trafo = new AVGeoRefTrafo(geoRefReader, true);
        AVASSERT(trafo);
        trafo->setName(name);
        return trafo;
    }

    AVLogError << "AVTrafoFactory::createGeoRefTrafo: couldn't create trafo with name '"
               << name << "', because the parameter list has an invalid size of " << params.count();
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

AVCoordinateTransformBase* AVTrafoFactory::
createWeatherRadarTrafo(const QString& name, const QStringList& params)
{
    if(!params.empty() && params.count() != 6)
    {
        AVLogWarning << "AVTrafoFactory::createWeatherRadarTrafo: parameters are not supported "
                     << "when creating trafo with name '" << name << "' and parameters '"
                     << params.join(", ") << "'";
        return 0;
    }

    AVCoordinateTransformBase* trafo  = 0;

    if (!params.isEmpty())
    {
        double v[6];

        QStringList::const_iterator it    = params.begin();
        QStringList::const_iterator endIt = params.end();
        for (uint i = 0; it != endIt; ++it, ++i)
        {
            bool ok = true;
            v[i] = (*it).toDouble(&ok);

            if (!ok)
            {
                AVLogError << "AVTrafoFactory::createWeatherRadarTrafo: parameter " << i << " ("
                           << (*it) << ") is not parsable as a double";
                return 0;
            }
        }

        trafo = new AVWeatherRadarTrafo(v[0], v[1], v[2], v[3], v[4], v[5]);
    }
    else
        trafo = new AVWeatherRadarTrafo();
    AVASSERT(trafo);
    trafo->setName(name);
    return trafo;
}

///////////////////////////////////////////////////////////////////////////////

AVCoordinateTransformBase* AVTrafoFactory::
createMeteoSatTrafo(const QString& name, const QStringList& params)
{
    if(!params.empty())
    {
        AVLogWarning << "AVTrafoFactory::createMeteoSatTrafo: parameters are not "
                        "supported when creating trafo with name '"
                    << name << "' and parameters '" << params.join(", ") << "'";
        return 0;
    }

    AVCoordinateTransformBase* trafo  = 0;

    trafo = new AVMeteoSatTrafo();
    AVASSERT(trafo);
    trafo->setName(name);
    return trafo;
}

///////////////////////////////////////////////////////////////////////////////

AVCoordinateTransformBase* AVTrafoFactory::
createGaussKruegerSystemTrafo(const QString& name, const QStringList& params)
{
    if(params.count() == 1)
    {
        bool convOk = false;
        int refMeridian = params[0].toInt(&convOk);

        if( !convOk
            || !(refMeridian == AVGaussKruegerSystemTrafo::RM_BESSEL_M28
            || refMeridian   == AVGaussKruegerSystemTrafo::RM_BESSEL_M31
            || refMeridian   == AVGaussKruegerSystemTrafo::RM_BESSEL_M34
            || refMeridian   == AVGaussKruegerSystemTrafo::RM_ETRS89_M6
            || refMeridian   == AVGaussKruegerSystemTrafo::RM_ETRS89_M9
            || refMeridian   == AVGaussKruegerSystemTrafo::RM_ETRS89_M12
            || refMeridian   == AVGaussKruegerSystemTrafo::RM_ETRS89_M15))
        {
            AVLogError << "AVTrafoFactory::createGaussKruegerSystemTrafo: "
                           "couldn't create trafo with name '"
                       << name << "', because the first parameter in parameter list '"
                       << params.join(", ") << "' is malformed";
            return 0;
        }

        AVCoordinateTransformBase* trafo = new AVGaussKruegerSystemTrafo(
            static_cast<AVGaussKruegerSystemTrafo::ReferenceMeridian>(refMeridian));
        AVASSERT(trafo);

        trafo->setName(name);
        return trafo;
    }

    AVLogError << "AVTrafoFactory::createGaussKruegerSystemTrafo: couldn't create trafo with name '"
               << name << "', because the parameter list has an invalid size of " << params.count();

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

AVCoordinateTransformBase* AVTrafoFactory::
createGaussKruegerTrafo(const QString& name, const QStringList& params)
{
    if(params.count() == 2 || params.count() == 3)
    {
        bool convOk = false;
        double refMeridian = params[0].toDouble(&convOk);

        if(!convOk)
        {
            AVLogError << "AVTrafoFactory::createGaussKruegerTrafo: couldn't create trafo with "
                          "name '" << name << "', because the first parameter in parameter list '"
                       << params.join(", ") << "' is malformed";
            return 0;
        }

        AVCoordinateTransformBase* trafo = 0;
        if(params.count() == 2)
        {
            int ellipsoidType = params[1].toInt(&convOk);

            if( !convOk
                || !(ellipsoidType == AVCoordEllipsoid::ET_WGS84
                || ellipsoidType   == AVCoordEllipsoid::ET_BESSEL))

            {
                AVLogError << "AVTrafoFactory::createGaussKruegerTrafo: couldn't create trafo "
                              "with name '" << name
                              << "', because the second parameter in parameter list '"
                              << params.join(", ") << "' is malformed or invalid";
                return 0;
            }

            AVCoordEllipsoid refEllipsoid(
                static_cast<AVCoordEllipsoid::EllipsoidType>(ellipsoidType));

            trafo = new AVGaussKruegerTrafo(refMeridian, refEllipsoid);
        }
        else
        {
            double e_a = params[1].toDouble(&convOk);

            if(!convOk)
            {
                AVLogError << "AVTrafoFactory::createGaussKruegerTrafo: couldn't create trafo "
                        "with name '" << name
                        << "', because the second parameter in parameter list '"
                        << params.join(", ") << "' is malformed";
                return 0;
            }

            double e_f = params[2].toDouble(&convOk);

            if(!convOk)
            {
                AVLogError << "AVTrafoFactory::createGaussKruegerTrafo: couldn't create trafo "
                              "with name '" << name
                           << "', because the third parameter in parameter list '"
                           << params.join(", ") << "' is malformed";
            }

            AVCoordEllipsoid refEllipsoid(AVCoordEllipsoid::ET_CUSTOM);
            if(!refEllipsoid.setEllipsoid_af(e_a, e_f))
            {
                AVLogError << "AVTrafoFactory::createGaussKruegerTrafo: couldn't create trafo "
                              "with name '" << name << "', cause of bad reference ellipsoid "
                              "parameters in parameter list '" << params.join(", ") << "'";
                return 0;
            }
            trafo = new AVGaussKruegerTrafo(refMeridian, refEllipsoid);
        }

        AVASSERT(trafo);
        trafo->setName(name);
        return trafo;
    }

    AVLogError << "AVTrafoFactory::createGaussKruegerSystemTrafo: couldn't create trafo with "
                  "name '" << name << "', because the parameter list has an invalid size of "
                  << params.count();

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

AVCoordinateTransformBase* AVTrafoFactory::
createUtmSystemTrafo(const QString& name, const QStringList& params)
{
    if(params.count() == 1)
    {
        bool convOk = false;
        int refMeridian = params[0].toInt(&convOk);

        if( !convOk
            || !(refMeridian == AVUtmSystemTrafo::RM_BESSEL_M28
            || refMeridian   == AVUtmSystemTrafo::RM_BESSEL_M31
            || refMeridian   == AVUtmSystemTrafo::RM_BESSEL_M34
            || refMeridian   == AVUtmSystemTrafo::RM_ETRS89_M6
            || refMeridian   == AVUtmSystemTrafo::RM_ETRS89_M9
            || refMeridian   == AVUtmSystemTrafo::RM_ETRS89_M12
            || refMeridian   == AVUtmSystemTrafo::RM_ETRS89_M15))
        {
            AVLogError <<  "AVTrafoFactory::createUtmSystemTrafo: couldn't create trafo with "
                    "name '" << name << "', because the first parameter in parameter list '"
                    << params.join(", ") << "' is malformed";
            return 0;
        }

        AVCoordinateTransformBase* trafo = new AVUtmSystemTrafo(
            static_cast<AVUtmSystemTrafo::ReferenceMeridian>(refMeridian));
        AVASSERT(trafo);

        trafo->setName(name);
        return trafo;
    }

    AVLogError << "AVTrafoFactory::createUtmSystemTrafo: couldn't create trafo with "
                  "name '" << name << "', because the parameter list has an invalid size of "
                  << params.count();
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

AVCoordinateTransformBase* AVTrafoFactory::
createUtmTrafo(const QString& name, const QStringList& params)
{
    if(params.count() == 2 || params.count() == 3)
    {
        bool convOk = false;
        double refMeridian = params[0].toDouble(&convOk);

        if(!convOk)
        {
            AVLogError << "AVTrafoFactory::createUtmTrafo: couldn't create trafo with name '"
                    << name << "', because the first parameter in parameter list '"
                    << params.join(", ") << "' is malformed";
            return 0;
        }

        AVCoordinateTransformBase* trafo = 0;
        if(params.count() == 2)
        {
            int ellipsoidType = params[1].toInt(&convOk);

            if( !convOk
                || !(ellipsoidType == AVCoordEllipsoid::ET_WGS84
                || ellipsoidType   == AVCoordEllipsoid::ET_BESSEL))

            {
                AVLogError <<  "AVTrafoFactory::createUtmTrafo: couldn't create trafo with name '"
                           << name << "', because the second parameter in parameter list '"
                           << params.join(", ") << "' is malformed or invalid";
                return 0;
            }

            AVCoordEllipsoid refEllipsoid(
                static_cast<AVCoordEllipsoid::EllipsoidType>(ellipsoidType));

            trafo = new AVUtmTrafo(refMeridian, refEllipsoid);
        }
        else
        {
            convOk = false;
            double e_a = params[1].toDouble(&convOk);

            if(!convOk)
            {
                AVLogError << "AVTrafoFactory::createUtmTrafo: couldn't create trafo with name '"
                           << name << "', because the second parameter in parameter list '"
                           << params.join(", ") << "' is malformed";
                return 0;
            }

            double e_f = params[2].toDouble(&convOk);

            if(!convOk)
            {
                AVLogError << "AVTrafoFactory::createUtmTrafo: couldn't create trafo with name '"
                           << name << "', because the third parameter in parameter list '"
                           << params.join(", ") << "' is malformed";
                return 0;
            }

            AVCoordEllipsoid refEllipsoid(AVCoordEllipsoid::ET_CUSTOM);
            if(!refEllipsoid.setEllipsoid_af(e_a, e_f))
            {
                AVLogError << "AVTrafoFactory::createUtmTrafo: couldn't create trafo with name '"
                           << name << "', cause of bad reference ellipsoid parameters in "
                           "parameter list '" << params.join(", ") << "'";
                return 0;
            }
            trafo = new AVUtmTrafo(refMeridian, refEllipsoid);
        }

        AVASSERT(trafo);
        trafo->setName(name);
        return trafo;
    }

    AVLogError << "AVTrafoFactory::createUtmSystemTrafo: couldn't create trafo with name '"
               << name << "', because the parameter list has an invalid size of "
               << params.count();

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

void AVTrafoFactory::initialize()
{
    m_trafoCreators->insert(SYSTEM_TRAFO,    createSystemTrafo);
    m_trafoCreators->insert(METEO_SAT_TRAFO, createMeteoSatTrafo);
    m_trafoCreators->insert(WEATHER_RADAR_TRAFO, createWeatherRadarTrafo);
    m_trafoCreators->insert(GEO_REF_TRAFO,       createGeoRefTrafo);
    m_trafoCreators->insert(GAUSS_KRUEGER_TRAFO, createGaussKruegerTrafo);
    m_trafoCreators->insert(GAUSS_KRUEGER_SYSTEM_TRAFO, createGaussKruegerSystemTrafo);
    m_trafoCreators->insert(UTM_TRAFO, createUtmTrafo);
    m_trafoCreators->insert(UTM_SYSTEM_TRAFO, createUtmSystemTrafo);
}

///////////////////////////////////////////////////////////////////////////////

AVCoordinateTransformBase* AVTrafoFactory::
trafoByTypeAndName(const QString& type, const QString& name, const QStringList& params)
{
    if(type.isEmpty())
    {
        AVLogError << "AVTrafoFactory::trafoByType: got empty type";
        return 0;
    }

    // build unique name for internal use, this uid has nothing to do with trafos
    // real uid
    QString uniqueTrafoName = name + params.join("");

    if(uniqueTrafoName.isEmpty()) return 0;

    AVCoordinateTransformBase* trafo = m_trafoPool.value(uniqueTrafoName,0);
    if(trafo != 0) return trafo;

    CallBackMap::const_iterator it = m_trafoCreators->find(type);

    if(it == m_trafoCreators->end())
    {
        AVLogError << "AVTrafoFactory::trafoByType: unknown trafo type '" << type << "'";
        return 0;
    }

    trafo = (*it)(name, params);

    if(trafo != 0)
    {
        m_trafoPool.insert(uniqueTrafoName, trafo);
    }

    return trafo;
}

///////////////////////////////////////////////////////////////////////////////

// End of File
