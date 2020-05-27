//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2016
//
// Module:    AVCALCULATIONLIB - AviBit Calculation Library
//
//-----------------------------------------------------------------------------

/*! \file
    \brief   AVTransitionLevel header
 */

#ifndef AVTRANSITIONLEVEL_INCLUDED
#define AVTRANSITIONLEVEL_INCLUDED

#include "avcalculationlib_export.h"

#include "avconfig2.h"

//-----------------------------------------------------------------------------
class AVCALCULATIONLIB_EXPORT AVTransitionLevelCalculator
{
public:
    virtual ~AVTransitionLevelCalculator() {}

    #define EnumValues(enum_value) \
             enum_value(Barometric, )   \
             enum_value(Table, )   \
             enum_value(None, )   \
             enum_value(Constant, )
    DEFINE_TYPESAFE_ENUM(TransitionLevelCalculationType, EnumValues)

    //! Factory method for easier usage in config classes.
    static AVTransitionLevelCalculator* createCalculator(TransitionLevelCalculationType type, const QString& config_prefix);

    /**
     * @brief Computes the transition level (=flight level to change altimeter from standard pressure to QNH when descending).
     *
     * @param qnh The current QNH in hectopascal. The valid range is configuration dependent.
     * @return    The transition level (flight level), or -1 for invalid input.
     */
    int computeTransitionLevel(uint qnh) const;

protected:
    AVTransitionLevelCalculator() {}
    static const QString CONFIG_NAMESPACE;

private:
    //! Receives the range-checked qnh
    virtual int doComputeLevel(uint qnh) const = 0;
};

//-----------------------------------------------------------------------------
//! Not a config class for easy unit testing.
class AVCALCULATIONLIB_EXPORT AVTransitionLevelCalculatorBarometric : public AVTransitionLevelCalculator
{
public:
    AVTransitionLevelCalculatorBarometric();
    ~AVTransitionLevelCalculatorBarometric() override {}

    uint m_transition_altitude;
    uint m_minimum_transition_layer;
    uint m_granularity;

private:
    /**
     * Min valid QNH value (in hPa)
     */
    static const uint MIN_VALID_QNH;

    /**
     * Max valid QNH value (in hPa)
     */
    static const uint MAX_VALID_QNH;

    /**
     * Basis for the transition level computation is the barometric formula (https://en.wikipedia.org/wiki/Barometric_formula)
     *
     * This implementation complies with the SAMOS ICD
     * (http://websvn.avibit.com/wsvn/docs/projects/LBIA/airmax/documentation/ICDs/QNH/lbia_icd_qnh_samos_r1.0.pdf)
     */
    int doComputeLevel(uint qnh) const override;
};

//-----------------------------------------------------------------------------
class AVCALCULATIONLIB_EXPORT AVTransitionLevelCalculatorBarometricConfig : public AVConfig2, public AVTransitionLevelCalculatorBarometric
{
public:
    AVTransitionLevelCalculatorBarometricConfig(const QString& prefix);
    ~AVTransitionLevelCalculatorBarometricConfig() override {}
};

//-----------------------------------------------------------------------------
//! Not a config class for easy unit testing.
class AVCALCULATIONLIB_EXPORT AVTransitionLevelCalculatorTable
        : public AVTransitionLevelCalculator
{
public:
    AVTransitionLevelCalculatorTable();
    ~AVTransitionLevelCalculatorTable() override {}

    /**
     * This table holds the precalculated flight level for specified QNH ranges. The key is a pair
     * of min and max QNH values (in hPa) that define the range of QNH and the value is the flight
     * level that corresponds to that QNH range.
     *
     * Structure: QMap<QPair<qnh_min, qnh_max>, flight_level)
     */
    QMap<QPair<uint, uint>, int> m_table;

private:
    /**
     * The transition level is calculated from the barometric table read from the configuration.
     */
    int doComputeLevel(uint qnh) const override;
};

//-----------------------------------------------------------------------------
class AVCALCULATIONLIB_EXPORT AVTransitionLevelCalculatorTableConfig
        : public AVConfig2, public AVTransitionLevelCalculatorTable
{
public:
    AVTransitionLevelCalculatorTableConfig(const QString& prefix);
    ~AVTransitionLevelCalculatorTableConfig() override {}

    QString postRefresh() override;
};

//-----------------------------------------------------------------------------
class AVCALCULATIONLIB_EXPORT AVTransitionLevelCalculatorConstantConfig : public AVConfig2, public AVTransitionLevelCalculator
{
public:
    AVTransitionLevelCalculatorConstantConfig(const QString& prefix);
    ~AVTransitionLevelCalculatorConstantConfig() override {}

private:
    int doComputeLevel(uint qnh) const override;

    uint m_transition_level;
};

#endif // AVTRANSITIONLEVEL_INCLUDED

// End of file
