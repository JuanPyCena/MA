///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2011
//
// Module:    AVCALCULATIONLIB - AviBit Calculation Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Thomas Pock, t.pock@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Configuration of the origin for the AVSystemTrafo Class
*/


// local includes
#include "avsystemtrafocfg.h"

///////////////////////////////////////////////////////////////////////////////
//                        STATIC VARIABLE DEFINITIONS
///////////////////////////////////////////////////////////////////////////////

QStringList   AVSystemTrafoConfig::ORIGIN_CONFIG_FILENAMES;
const QString AVSystemTrafoConfig::DEFAULT_ORIGIN_CONFIG_FILENAME = "origin";

///////////////////////////////////////////////////////////////////////////////

AVSystemTrafoConfig::AVSystemTrafoConfig(AVConfig2Container& cfg_container, const QString& name)
    : AVConfig2HybridConfig(name, name, &cfg_container)
{
    AVLogInfo << "AVSystemTrafoConfig::AVSystemTrafoConfig: initializing system trafo config with name (" << name << ")";

    if (//TODO1AVConfigBase::process_uses_avconfig2 &&
        AVConfig2Global::isSingletonInitialized())
    {
        AVConfig2Global::singleton().assignLoadedMetadataTo(cfg_container);
    }

    registerParameter("origin_lat", &m_origin_lat,
                      "Latitude of the origin in degrees").
                      setDeprecatedName("Origin-Latitude");

    registerParameter("origin_lon", &m_origin_lon,
                      "Longitude of the origin in degrees").
                      setDeprecatedName("Origin-Longitude");

    registerParameter("origin_geod_h", &m_origin_geod_h,
                      "Geodetic Height of the origin in meters").
                      setDeprecatedName("Origin-Geodetic-Height");

    bool success = getContainer().loadConfig(name);
    if (!success)
    {
        AVLogError << "AVSystemTrafoConfig::AVSystemTrafoConfig: could not load config with name (" << name << ")";
    }

    AVConfig2Container::RefreshResult refresh_result = getContainer().refreshAllParams();
    if (!refresh_result.refreshSuccessful())
    {
        AVLogError << "AVSystemTrafoConfig::AVSystemTrafoConfig: "
            "could not refresh config with name (" << name << "): " << refresh_result.getErrorMessage();
    }

    if (!success || !refresh_result.refreshSuccessful())
    {
        AVLogInfo << "AVSystemTrafoConfig::AVSystemTrafoConfig: saving config because of errors";
        AVConfig2Saver saver;
        saver.setSaveValueMode(AVConfig2Saver::SVM_KEEP_LOADED_VALUES);
        saver.save(getContainer());
    }

    if (!refresh_result.refreshSuccessful())
    {
        AVLogFatal << "AVSystemTrafo: " << refresh_result.getErrorMessage();
    }

    bool log = false;
    if (!ORIGIN_CONFIG_FILENAMES.contains(name))
    {
        ORIGIN_CONFIG_FILENAMES.append(name);
        log = true;
    }

    bool succ = refreshParams();

    if (log)
    {
        if (succ)
            LOGGER_AVCOMMON_AVLIB.Write(LOG_INFO,
                "AVSystemTrafoConfig: "
                "Initialized with configuration file '%s' for the 1st time: "
                "LAT/LON/GeodH=%.4lf/%.4lf/%.2lf. "
                "Inhibiting eventual further logs with this origin...",
                qPrintable(name), m_origin_lat, m_origin_lon, m_origin_geod_h);
        else
            LOGGER_AVCOMMON_AVLIB.Write(LOG_INFO,
                "AVSystemTrafoConfig: "
                "Loading of specified config file '%s' not successful! "
                "Initialized with (hard-coded) default configuration for the 1st time: "
                "LAT/LON/GeodH=%.4lf/%.4lf/%.2lf. Inhibiting further logs with this origin... ",
                qPrintable(name), m_origin_lat, m_origin_lon, m_origin_geod_h);
    }
}

// End of file
