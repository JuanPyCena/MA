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

// Qt includes
#include <QMap>
#include <QString>

// local includes
#include "avlib_export.h"
#include "avmacros.h"

#ifndef __AVVEHICLE_H__
#define __AVVEHICLE_H__

/////////////////////////////////////////////////////////////////////////////

//! Vehicle Data Class
class AVLIB_EXPORT AVVehicle
{
    AVDISABLECOPY(AVVehicle);

    //! friend declaration for function level test case
    friend class TstAVVehicle;

public:

    enum VehicleClass { VEH_CLASS_INVALID = 0,
                        VEH_CLASS_FOLLOWME,
                        VEH_CLASS_ATC_EQ_MAINTENANCE,
                        VEH_CLASS_AIRPORT_MAINTENANCE,
                        VEH_CLASS_FIRE,
                        VEH_CLASS_BIRD_SCARER,
                        VEH_CLASS_SNOW_PLOUGH,
                        VEH_CLASS_RWY_SWEEPER,
                        VEH_CLASS_EMERGENCY,
                        VEH_CLASS_POLICE,
                        VEH_CLASS_BUS,
                        VEH_CLASS_TUG,
                        VEH_CLASS_GRASS_CUTTER,
                        VEH_CLASS_FUEL,
                        VEH_CLASS_BAGGAGE,
                        VEH_CLASS_CATERING,
                        VEH_CLASS_ACFT_MAINTENANCE,
                        VEH_CLASS_UNKNOWN
    };

    //! Standard Constructor
    AVVehicle();

    //! Destructor
    virtual ~AVVehicle();

    //------

    static QString textFromVehicleClass(VehicleClass vehicle_class);
    static VehicleClass vehicleClassFromText(const QString& text);

private:
    static void setupMappings();

private:
    static QMap<VehicleClass, QString> m_classToTextMapping;
    static QMap<QString, int> m_textToClassMapping;
};

#endif /* __AVVEHICLE_H__ */

// End of file

