///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - AviBit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  DI Alexander Wemmer, a.wemmer@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief
*/




#include "avlog.h"

#include "avvehicle.h"

/////////////////////////////////////////////////////////////////////////////

QMap<AVVehicle::VehicleClass, QString> AVVehicle::m_classToTextMapping;
QMap<QString, int> AVVehicle::m_textToClassMapping;

/////////////////////////////////////////////////////////////////////////////

AVVehicle::AVVehicle()
{

}

/////////////////////////////////////////////////////////////////////////////

AVVehicle::~AVVehicle()
{

}

/////////////////////////////////////////////////////////////////////////////

void AVVehicle::setupMappings()
{
    for (int vehicle_class = 0; vehicle_class <= VEH_CLASS_UNKNOWN; ++vehicle_class)
    {
        switch (vehicle_class)
        {
            case (VEH_CLASS_INVALID):
                m_classToTextMapping.insert(VEH_CLASS_INVALID, "INVALID");
                m_textToClassMapping.insert("INVALID", vehicle_class);
                break;
            case (VEH_CLASS_FOLLOWME):
                m_classToTextMapping.insert(VEH_CLASS_FOLLOWME, "FOLLOWME");
                m_textToClassMapping.insert("FOLLOWME", vehicle_class);
                break;
            case (VEH_CLASS_ATC_EQ_MAINTENANCE):
                m_classToTextMapping.insert(VEH_CLASS_ATC_EQ_MAINTENANCE, "ATC_EQ_MAINTENANCE");
                m_textToClassMapping.insert("ATC_EQ_MAINTENANCE", vehicle_class);
                break;
            case (VEH_CLASS_AIRPORT_MAINTENANCE):
                m_classToTextMapping.insert(VEH_CLASS_AIRPORT_MAINTENANCE, "AIRPORT_MAINTENANCE");
                m_textToClassMapping.insert("AIRPORT_MAINTENANCE", vehicle_class);
                break;
            case (VEH_CLASS_FIRE):
                m_classToTextMapping.insert(VEH_CLASS_FIRE, "FIRE");
                m_textToClassMapping.insert("FIRE", vehicle_class);
                break;
            case (VEH_CLASS_BIRD_SCARER):
                m_classToTextMapping.insert(VEH_CLASS_BIRD_SCARER, "BIRD_SCARER");
                m_textToClassMapping.insert("BIRD_SCARER", vehicle_class);
                break;
            case (VEH_CLASS_SNOW_PLOUGH):
                m_classToTextMapping.insert(VEH_CLASS_SNOW_PLOUGH, "SNOW_PLOUGH");
                m_textToClassMapping.insert("SNOW_PLOUGH", vehicle_class);
                break;
            case (VEH_CLASS_RWY_SWEEPER):
                m_classToTextMapping.insert(VEH_CLASS_RWY_SWEEPER, "RWY_SWEEPER");
                m_textToClassMapping.insert("RWY_SWEEPER", vehicle_class);
                break;
            case (VEH_CLASS_EMERGENCY):
                m_classToTextMapping.insert(VEH_CLASS_EMERGENCY, "EMERGENCY");
                m_textToClassMapping.insert("EMERGENCY", vehicle_class);
                break;
            case (VEH_CLASS_POLICE):
                m_classToTextMapping.insert(VEH_CLASS_POLICE, "POLICE");
                m_textToClassMapping.insert("POLICE", vehicle_class);
                break;
            case (VEH_CLASS_BUS):
                m_classToTextMapping.insert(VEH_CLASS_BUS, "BUS");
                m_textToClassMapping.insert("BUS", vehicle_class);
                break;
            case (VEH_CLASS_TUG):
                m_classToTextMapping.insert(VEH_CLASS_TUG, "TUG");
                m_textToClassMapping.insert("TUG", vehicle_class);
                break;
            case (VEH_CLASS_GRASS_CUTTER):
                m_classToTextMapping.insert(VEH_CLASS_GRASS_CUTTER, "GRASS_CUTTER");
                m_textToClassMapping.insert("GRASS_CUTTER", vehicle_class);
                break;
            case (VEH_CLASS_FUEL):
                m_classToTextMapping.insert(VEH_CLASS_FUEL, "FUEL");
                m_textToClassMapping.insert("FUEL", vehicle_class);
                break;
            case (VEH_CLASS_BAGGAGE):
                m_classToTextMapping.insert(VEH_CLASS_BAGGAGE, "BAGGAGE");
                m_textToClassMapping.insert("BAGGAGE", vehicle_class);
                break;
            case (VEH_CLASS_CATERING):
                m_classToTextMapping.insert(VEH_CLASS_CATERING, "CATERING");
                m_textToClassMapping.insert("CATERING", vehicle_class);
                break;
            case (VEH_CLASS_ACFT_MAINTENANCE):
                m_classToTextMapping.insert(VEH_CLASS_ACFT_MAINTENANCE, "ACFT_MAINTENANCE");
                m_textToClassMapping.insert("ACFT_MAINTENANCE", vehicle_class);
                break;
            case (VEH_CLASS_UNKNOWN):
                m_classToTextMapping.insert(VEH_CLASS_UNKNOWN, "UNKNOWN");
                m_textToClassMapping.insert("UNKNOWN", vehicle_class);
                break;
            default:
                LOGGER_AVCOMMON.Write(LOG_FATAL,
                                      "AVVehicle: unknown vehicle class '%d'", vehicle_class);
                AVASSERT(false);
                break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

QString AVVehicle::textFromVehicleClass(VehicleClass vehicle_class)
{
    if (m_classToTextMapping.isEmpty()) setupMappings();

    QMap<VehicleClass, QString>::const_iterator it = m_classToTextMapping.find(vehicle_class);
    if (it != m_classToTextMapping.end()) return it.value();

    return "INVALID";
}

/////////////////////////////////////////////////////////////////////////////

AVVehicle::VehicleClass AVVehicle::vehicleClassFromText(const QString& text)
{
    if (m_textToClassMapping.isEmpty()) setupMappings();

    QMap<QString, int>::const_iterator it = m_textToClassMapping.find(text);
    if (it != m_textToClassMapping.end()) return static_cast<VehicleClass>(it.value());

    return VEH_CLASS_INVALID;
}

/////////////////////////////////////////////////////////////////////////////

// End of file
