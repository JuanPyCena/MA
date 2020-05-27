///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// QT-Version: QT5
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Kevin Krammer, k.krammer@avibit.com
    \author  QT4-PORT: Robert Koenighofer, r.koenighofer@avibit.com
    \brief   Abstract base class for Archive Players
*/


// local includes
#include "avarchiveplayer.h"


///////////////////////////////////////////////////////////////////////////////

AVArchivePlayer::AVArchivePlayer(QObject* parent, const char* name)
    : QObject(parent), m_speedFactor(1.0)
{
    setObjectName(name);
}

///////////////////////////////////////////////////////////////////////////////

AVArchivePlayer::~AVArchivePlayer()
{
}

///////////////////////////////////////////////////////////////////////////////

void AVArchivePlayer::setMinReplayTime(const QDateTime& minTime)
{
    if (!minTime.isValid()) return;
    if (archive().isNull()) return;

    QDateTime min = minArchiveTime();
    QDateTime max = maxArchiveTime();

    if (!min.isValid() || !max.isValid()) return;

    if (minTime < min)
    {
        m_minReplayTime = min;
    }
    else if (minTime > max)
    {
        m_minReplayTime = max;
    }
    else
    {
        m_minReplayTime = minTime;
    }

    // if max replay time is set, ensure it is bigger or equal
    if (m_maxReplayTime.isValid() && m_maxReplayTime < m_minReplayTime)
    {
        m_maxReplayTime = m_minReplayTime;
    }
}
///////////////////////////////////////////////////////////////////////////////

void AVArchivePlayer::setMaxReplayTime(const QDateTime& maxTime)
{
    if (!maxTime.isValid()) return;
    if (archive().isNull()) return;

    QDateTime min = minArchiveTime();
    QDateTime max = maxArchiveTime();

    if (!min.isValid() || !max.isValid()) return;

    if (maxTime < min)
    {
        m_maxReplayTime = min;
    }
    else if (maxTime > max)
    {
        m_maxReplayTime = max;
    }
    else
    {
        m_maxReplayTime = maxTime;
    }

    // if min replay time is set, ensure it is bigger or equal
    if (m_minReplayTime.isValid() && m_minReplayTime > m_maxReplayTime)
    {
        m_minReplayTime = m_maxReplayTime;
    }
}

// End of file
