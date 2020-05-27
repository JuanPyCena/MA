//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2016
//
// Module:    AVCALCULATIONLIB - AviBit Calculation Library
//
//-----------------------------------------------------------------------------

/*! \file
    \brief   AVTransitionLevel implementation
 */

#include "avtransitionlevel.h"
#include "avsciconst.h"

#include <algorithm>

//-----------------------------------------------------------------------------

const QString AVTransitionLevelCalculator::CONFIG_NAMESPACE = ".transition_level_computation";

// The values below are the most extreme recorded values plus some safety margin
const uint AVTransitionLevelCalculatorBarometric::MIN_VALID_QNH =  850u;
const uint AVTransitionLevelCalculatorBarometric::MAX_VALID_QNH = 1100u;

//-----------------------------------------------------------------------------

AVTransitionLevelCalculator *AVTransitionLevelCalculator::createCalculator(TransitionLevelCalculationType type, const QString &config_prefix)
{
    switch (type)
    {
    case TransitionLevelCalculationType::None:
        return 0;
    case TransitionLevelCalculationType::Barometric:
        return new AVTransitionLevelCalculatorBarometricConfig(config_prefix);
    case TransitionLevelCalculationType::Table:
        return new AVTransitionLevelCalculatorTableConfig(config_prefix);
    case TransitionLevelCalculationType::Constant:
        return new AVTransitionLevelCalculatorConstantConfig(config_prefix);
    default:
        AVASSERT(false);
        return 0;
    }
}

//-----------------------------------------------------------------------------

int AVTransitionLevelCalculator::computeTransitionLevel(uint qnh) const
{
    return doComputeLevel(qnh);
}

//-----------------------------------------------------------------------------

AVTransitionLevelCalculatorBarometric::AVTransitionLevelCalculatorBarometric() :
    m_transition_altitude(0),
    m_minimum_transition_layer(0),
    m_granularity(0)
{

}

//-----------------------------------------------------------------------------

int AVTransitionLevelCalculatorBarometric::doComputeLevel(uint qnh) const
{
    if ((qnh < MIN_VALID_QNH) || (qnh > MAX_VALID_QNH))
    {
        AVLogError << "AVTransitionLevelCalculatorBarometric::doComputeLevel: Invalid QNH " << qnh;
        return -1;
    }

    if (m_granularity == 0)
    {
        AVLogError << "AVTransitionLevelCalculatorBarometric::doComputeLevel: granularity cannot be 0";
        return -1;
    }

    // 27672,29626 [ft]
    static const double K = AVSciConst::mToft / AVSciConst::g * AVSciConst::Rair * AVSciConst::T0ISA;
    // 1013,25 [hPa]
    static const double P0_HECTOPASCAL = AVSciConst::P0ISA / 100;

    double tmp_level = -K*log(qnh / P0_HECTOPASCAL);
    tmp_level += m_transition_altitude + m_minimum_transition_layer;
    tmp_level /= 100;

    if (tmp_level <= 0)
    {
        // this can happen for extremely high QNH values and low transition altitudes.
        AVLogInfo << "AVTransitionLevelCalculatorBarometric::doComputeLevel: non-positive result: " << tmp_level;
        return -1;
    }

    int ret = static_cast<int>(tmp_level);
    ret = ret - (ret % m_granularity) + m_granularity;

    return ret;
}

//-----------------------------------------------------------------------------

AVTransitionLevelCalculatorBarometricConfig::AVTransitionLevelCalculatorBarometricConfig(const QString &prefix) :
    AVConfig2(prefix + CONFIG_NAMESPACE)
{
    registerParameter("transition_altitude", &m_transition_altitude,
                      "The transition altitude in feet (=altitude to change altimeter from QNH to standard pressure when ascending).");
    registerParameter("minimum_transition_layer", &m_minimum_transition_layer,
                      "The minimum separation between transition altitude and transition level in feet (=minimum height of the transition layer).");
    registerParameter("granularity", &m_granularity,
                      "Transition level granularity. The transition level is complemented to be divisible by this number.");
}

//-----------------------------------------------------------------------------

AVTransitionLevelCalculatorConstantConfig::AVTransitionLevelCalculatorConstantConfig(const QString &prefix) :
    AVConfig2(prefix+ CONFIG_NAMESPACE)
{
    registerParameter("transition_level", &m_transition_level, "The constant transition level.");
}

//-----------------------------------------------------------------------------

AVTransitionLevelCalculatorTable::AVTransitionLevelCalculatorTable()
    : AVTransitionLevelCalculator(),
      m_table()
{
}

//-----------------------------------------------------------------------------

int AVTransitionLevelCalculatorTable::doComputeLevel(uint qnh) const
{
    // Get transition value from the table that corresponds the input QNH value
    int transition_level = -1;

    for (auto key : m_table.keys())
    {
        const uint qnh_min = key.first;
        const uint qnh_max = key.second;

        if ((qnh_min <= qnh) && (qnh <= qnh_max))
        {
            transition_level = m_table[key];
            break;
        }
    }

    // Check if transition level is valid (positive)
    if (transition_level <= 0)
    {
        AVLogInfo << "AVTransitionLevelCalculatorTable::doComputeLevel: Invalid QNH " << qnh;
        transition_level = -1;
    }

    return transition_level;
}

//-----------------------------------------------------------------------------

AVTransitionLevelCalculatorTableConfig::AVTransitionLevelCalculatorTableConfig(
        const QString &prefix)
    : AVConfig2(prefix + CONFIG_NAMESPACE),
      AVTransitionLevelCalculatorTable()
{
    registerParameter("table", &m_table,
                      "This table holds the precalculated flight level for specified QNH ranges. "
                      "The key is a pair of min and max QNH values (in hPa) that define the range "
                      "of QNH and the value is the flight level that corresponds to that QNH "
                      "range.");
}

//-----------------------------------------------------------------------------

QString AVTransitionLevelCalculatorTableConfig::postRefresh()
{
    QString error_text;

    if (!m_table.isEmpty())
    {
        // First make sure that keys are sorted by min values
        auto sorted_keys = m_table.keys();

        std::sort(sorted_keys.begin(),
                  sorted_keys.end(),
                  [](QPair<uint, uint> key1, QPair<uint, uint> key2)
        {
            return (key1.first < key2.first);
        });

        // Then check if the QNH range and the transition level are valid
        for (int i = 0; i < sorted_keys.size(); i++)
        {
            auto key = sorted_keys[i];

            if (key.first > key.second)
            {
                error_text = QString("Invalid QNH range: min = %1, max = %2");
                error_text = error_text.arg(key.first).arg(key.second);
                break;
            }

            const int transition_level = m_table[key];

            if (transition_level <= 0)
            {
                error_text = QString("Invalid transition level value: %1").arg(transition_level);
                break;
            }
        }

        // Finally check if there are any gaps between the items
        if (error_text.isEmpty())
        {
            for (int i = 1; i < sorted_keys.size(); i++)
            {
                auto key1 = sorted_keys[i - 1];
                auto key2 = sorted_keys[i];

                if (key1.second > key2.first)
                {
                    error_text = QString("Gap between QNH ranges: "
                                         "[min = %1, max = %2] and [min = %3, max = %4]");
                    error_text = error_text.arg(key1.first).arg(key1.second);
                    error_text = error_text.arg(key2.first).arg(key2.second);
                    break;
                }
            }
        }
    }

    return error_text;
}

//-----------------------------------------------------------------------------

int AVTransitionLevelCalculatorConstantConfig::doComputeLevel(uint qnh) const
{
    Q_UNUSED(qnh);
    return m_transition_level;
}


// End of file
