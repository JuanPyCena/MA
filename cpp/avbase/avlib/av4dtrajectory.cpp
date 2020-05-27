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
    \author  Stefan Kunz, s.kunz@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief
*/




// system includes

// qt includes
#include <QStringList>

// avlib includes
#include "avlog.h"
#include "avmisc.h"
#include "avdatastream.h"

// local includes
#include "av4dtrajectory.h"

/////////////////////////////////////////////////////////////////////////////
//                          static members
/////////////////////////////////////////////////////////////////////////////

// ***********************
// ATTENTION: Do *NOT* change these assignments because it would brake the
// trajectory conversion of AVMsgFlightplan::readEntry() !!
// ***********************

const double AV4DTrajectoryNodeDepricated::LAT_INVALID        = -10000.0;
const double AV4DTrajectoryNodeDepricated::LON_INVALID        = -10000.0;
const double AV4DTrajectoryNodeDepricated::ALT_INVALID        = -10000.0;
const double AV4DTrajectoryNodeDepricated::TURNRADIUS_INVALID = -1.0;
const double AV4DTrajectoryNodeDepricated::SPOTSIZE_INVALID   = -1.0;

/////////////////////////////////////////////////////////////////////////////
//                              operators
/////////////////////////////////////////////////////////////////////////////

QDataStream& operator>>(QDataStream &s, AV4DTrajectoryNodeDepricated& node)
{
    qint8 type = 0, turndir = 0;
    double turnrad = 0.0, lat = 0.0, lon = 0.0, alt = 0.0, spotsize = 0.0;
    AVDateTime eta;

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // ATTENTION: When changing this, you *MUST* also advance the version
    // of the AVMsgFlightplan to stay compatible!!
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    s >> type >> turndir >> turnrad >> lat >> lon >> alt >> eta >> spotsize;

    eta.setTimeSpec(Qt::UTC);

    node.setType(static_cast<AV4DTrajectoryNodeDepricated::Type>(type));
    node.setTurnDirection(static_cast<AV4DTrajectoryNodeDepricated::TurnDirection>(turndir));
    node.setTurnRadius(turnrad);
    node.setLatitude(lat);
    node.setLongitude(lon);
    node.setAltitude(alt);
    node.setEta(eta);
    node.setSpotsize(spotsize);

    return s;
}

/////////////////////////////////////////////////////////////////////////////

QDataStream& operator<<(QDataStream &s, const AV4DTrajectoryNodeDepricated& node)
{
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // ATTENTION: When changing this, you *MUST* also advance the version
    // of the AVMsgFlightplan to stay compatible!!
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    s << static_cast<qint8>(node.type())
      << static_cast<qint8>(node.turnDirection())
      << node.turnRadius()
      << node.latitude()
      << node.longitude()
      << node.altitude()
      << node.eta()
      << node.spotsize();

    return s;
}

///////////////////////////////////////////////////////////////////////////////
//                          AV4DTrajectoryNodeDepricated
///////////////////////////////////////////////////////////////////////////////

bool AV4DTrajectoryNodeDepricated::decode(const QString& nodeStr, const AVDateTime& msgDt,
                                const QString& delim)
{
    static const double SCALING_TURN_RADIUS = 0.01;
    static const double SCALING_DEGREES     = 1.0;
    static const double SCALING_MINUTES     = 0.1;
    static const double SCALING_ALTITUDE    = 10.0;

    // invalidate all fields
    invalidate();

    QStringList elements = nodeStr.split(delim, QString::KeepEmptyParts);
    if (elements.count() < static_cast<int>(PointPositionPos+1)) return false;

    bool ok = false;
    AVASSERT(elements.count() > PointTypePos);
    int type = elements[PointTypePos].toInt(&ok, 16);
    if (!ok || type > TransitionAlt)
    {
        AVLogger->Write(LOG_WARNING, "Failed to decode IDL Point type from '%s'",
                        qPrintable(elements[PointTypePos]));
        m_type = UnknownType;
    }
    else
        m_type = static_cast<Type>(type);

    AVASSERT(elements.count() > TurnDirectionPos);
    QString turnDirection = elements[TurnDirectionPos];
    if (turnDirection.isEmpty())
        m_turnDirection = NoTurn;
    else if (turnDirection == "L")
        m_turnDirection = LeftTurn;
    else if (turnDirection == "R")
        m_turnDirection = RightTurn;
    else
    {
        AVLogger->Write(LOG_WARNING, "Failed to decode IDL Turn Direction from "
                        "'%s'", qPrintable(turnDirection));

        m_turnDirection = UnknownTurn;
    }

    AVASSERT(elements.count() > TurnRadiusPos);
    if (! elements[TurnRadiusPos].isEmpty())
    {
        int turnRadius = elements[TurnRadiusPos].toInt(&ok);
        if (!ok)
        {
            AVLogger->Write(LOG_WARNING, "Failed to decode IDL Turn Radius from "
                            "'%s'", qPrintable(elements[TurnRadiusPos]));
            m_turnRadius = TURNRADIUS_INVALID;
        }
        else
            m_turnRadius = turnRadius * SCALING_TURN_RADIUS;
    }
    else
        m_turnRadius = TURNRADIUS_INVALID;

    AVASSERT(elements.count() > PointPositionPos);
    QString position = elements[PointPositionPos];
    if (position.length() != 13)
    {
        AVLogger->Write(LOG_ERROR, "Failed to decode IDL Point Position from "
                        "'%s' (length %d != 13)",
                        qPrintable(position), position.length());
        return false;
    }
    else
    {
        bool nok = false;
        double lat = 0.0, lon = 0.0;
        QString direction = position.mid(0,1);
        int sign = 0;
        if (direction == "N") sign = 1;
        else if (direction == "S") sign = -1;
        else
        {
            nok = true;
            AVLogger->Write(LOG_ERROR, "No valid N/S identifier for latitude");
        }

        int deg = position.mid(1,2).toInt(&ok);
        if (!ok || deg < 0 || deg > 90)
        {
            nok = true;
            AVLogger->Write(LOG_ERROR, "Invalid latitude degrees range: '%s'",
                            qPrintable(position.mid(1,2)));
        }
        int min = position.mid(3,3).toInt(&ok);
        if (!ok || min < 0 || min > 600)
        {
            nok = true;
            AVLogger->Write(LOG_ERROR, "Invalid latitude minutes range: '%s'",
                            qPrintable(position.mid(3,3)));
        }
        lat = sign * (deg * SCALING_DEGREES   +
                      (min * SCALING_MINUTES) / 60.0);

        direction = position.mid(6,1);
        sign = 0;
        if (direction == "E") sign = 1;
        else if (direction == "W") sign = -1;
        else
        {
            nok = true;
            AVLogger->Write(LOG_ERROR, "No valid E/W identifier for longitude");
        }
        deg = position.mid(7,3).toInt(&ok);
        if (!ok || deg < 0 || deg > 180)
        {
            nok = true;
            AVLogger->Write(LOG_ERROR, "Invalid longitude degrees range: '%s'",
                            qPrintable(position.mid(7,3)));
        }
        min = position.mid(10,3).toInt(&ok);
        if (!ok || min < 0 || min > 600)
        {
            nok = true;
            AVLogger->Write(LOG_ERROR, "Invalid longitude minutes range: '%s'",
                            qPrintable(position.mid(10,3)));
        }
        lon = sign * (deg * SCALING_DEGREES   +
                      (min * SCALING_MINUTES) / 60.0);

        if (nok)
        {
            AVLogger->Write(LOG_ERROR, "Failed to decode IDL Point Position from "
                            "'%s'", qPrintable(position));
            m_lat = LAT_INVALID;
            m_lon = LON_INVALID;
            return false;
        }

        m_lat = lat;
        m_lon = lon;
    }

    // optional elements from here on

    if (elements.count() > PointAltitudePos)
    {
        int alt = elements[PointAltitudePos].toInt(&ok);
        if (!ok)
        {
            AVLogger->Write(LOG_WARNING, "Failed to decode IDL Altitude from '%s'",
                            qPrintable(elements[PointAltitudePos]));
            m_alt = ALT_INVALID;
        }
        else
            m_alt = alt * SCALING_ALTITUDE;
    }

    if (elements.count() > PointEtaPos)
    {
        bool nok = false;
        QString eta = elements[PointEtaPos];
        if (eta.length() != 6)
        {
            AVLogger->Write(LOG_ERROR, "Failed to decode IDL ETA from "
                            "'%s'", qPrintable(eta));
            return false;
        }

        int h = eta.mid(0,2).toInt(&ok);
        if (!ok || h < 0 || h > 23) nok = true;
        int m = eta.mid(2,2).toInt(&ok);
        if (!ok || m < 0 || m > 59) nok = true;
        int s = eta.mid(4,2).toInt(&ok);
        if (!ok || m < 0 || m > 59) nok = true;

        if (nok)
        {
            AVLogger->Write(LOG_WARNING, "Failed to decode IDL Eta from '%s'",
                            qPrintable(eta));
            return false;
        }

        static const int SECS_PER_HALF_DAY = 43200;
        // assume date for today
        m_eta = AVDateTime(msgDt.date(), QTime(h, m, s));
        // correct date if midnight detected
        if (m_eta < msgDt)
        {
            if (m_eta.secsTo(msgDt) > SECS_PER_HALF_DAY)
            {
                m_eta = m_eta.addDays(1);
            }
        }
        else // m_eta >= msgDt
        {
            if (msgDt.secsTo(m_eta) > SECS_PER_HALF_DAY)
            {
                m_eta = m_eta.addDays(-1);
            }
        }
    }

    if (hasZeroPosition())
        invalidatePosition();

    return true;
}

///////////////////////////////////////////////////////////////////////////////

QString AV4DTrajectoryNodeDepricated::toString() const
{
    QString ret;
    AVsprintf(ret, "dt=%s type=%d dir=%d rad=%.02f lat=%.04f lon=%.04f alt=%.02f",
              qPrintable(AVPrintDateTime(m_eta)), m_type, m_turnDirection,
              m_turnRadius, m_lat, m_lon, m_alt);
    return ret;
}

///////////////////////////////////////////////////////////////////////////////
//                          AV4DTrajectory
///////////////////////////////////////////////////////////////////////////////

int AV4DTrajectoryDepricated::decode(const QString& trajectoryStr,
                           const AVDateTime& msgDt,
                           const QString& nodeDelim,
                           const QString& nodeEleDelim)
{
    int ret = 0;

    QStringList trajectory = trajectoryStr.split(nodeDelim);
    if (trajectory.count() < 1) return -1;

    QStringList::const_iterator iter = trajectory.begin();
    while (iter != trajectory.end())
    {
        AV4DTrajectoryNodeDepricated node;
        if (! node.decode(*iter, msgDt, nodeEleDelim))
        {
            AVLogger->Write(LOG_ERROR, "Decoding of 4D Trajectory (node %d) "
                            "failed!", ret);
            return -1;
        }
        push_back(node);
        ++ret;
        ++iter;
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////

QString AV4DTrajectoryDepricated::toString() const
{
    QString ret = QString( "Trajectory has %1 nodes:\n" ).arg(count());

    const_iterator iter = begin();
    for (; iter != end(); ++iter)
    {
        ret += (*iter).toString();
        ret += "\n";
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////

void AV4DTrajectoryDepricated::addSecs(int secs)
{
    iterator iter = begin();
    for (; iter != end(); ++iter) (*iter).addSecs(secs);
}

///////////////////////////////////////////////////////////////////////////////

void AV4DTrajectoryDepricated::addAltitudeOffset(double alt_offset)
{
    iterator iter = begin();
    for (; iter != end(); ++iter) (*iter).addAltitudeOffset(alt_offset);
}

///////////////////////////////////////////////////////////////////////////////

uint qHash(const AV4DTrajectoryNodeDepricated& key, uint seed)
{
    Q_UNUSED(seed);
    return qHash(key.toString());	// TODO FLO check
}

// End of file
