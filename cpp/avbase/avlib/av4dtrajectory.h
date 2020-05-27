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
    \brief   Encapsulates a 4D trajectory
*/

#ifndef __AV4DTRAJECTORY_H__
#define __AV4DTRAJECTORY_H__

// qt includes
#include <QList>
#include <QTextStream>


// avlib includes
#include "avlib_export.h"
#include "avdatetime.h"
#include "avmisc.h"

class AVDataStream;

//! Encapsulates a single node in a 4D trajectory
//! ATTENTION: This class is depricated.
//! \sa AVMsgTrajectoryNode \sa AVMsgListTrajectoryNode \sa AVMsgDictTrajectory
class AVLIB_EXPORT AV4DTrajectoryNodeDepricated
{
public:

    //! ***********************
    //! ATTENTION: Do *NOT* change this enum because it would brake the
    //! trajectory conversion of AVMsgFlightplan::readEntry() !!
    enum EncodedPosition
    {
        PointTypePos       = 0,
        TurnDirectionPos,
        TurnRadiusPos,
        PointPositionPos,
        PointAltitudePos,
        PointEtaPos
    };

    //! Type of Trajectory Node
    //! ***********************
    //! ATTENTION: Do *NOT* change this enum because it would brake the
    //! trajectory conversion of AVMsgFlightplan::readEntry() !!
    enum Type
    {
        FlyByWaypoint     = 0,
        FlyOverWaypoint,
        HoldPattern,
        ProcedureHold,
        ProcedureTurn,
        RfAfLeg,
        TopOfClimb        = 8,
        TopOfDescent,
        StartOfLevel,
        CrossoverAlt,
        TransitionAlt,
        UnknownType       = 0xf1
    };

    //! Turn direction
    //! ***********************
    //! ATTENTION: Do *NOT* change this enum because it would brake the
    //! trajectory conversion of AVMsgFlightplan::readEntry() !!
    enum TurnDirection
    {
        NoTurn,
        LeftTurn,
        RightTurn,
        UnknownTurn
    };

    static const double LAT_INVALID;          //!< invalid latitude
    static const double LON_INVALID;          //!< invalid longitude
    static const double ALT_INVALID;          //!< invalid altitude
    static const double TURNRADIUS_INVALID;   //!< invalid turn radius
    static const double SPOTSIZE_INVALID;     //!< invalid spotsize

    //! Standard Constructor
    AV4DTrajectoryNodeDepricated()
    : m_type(UnknownType),
      m_turnDirection(UnknownTurn),
      m_turnRadius(TURNRADIUS_INVALID),
      m_lat(LAT_INVALID),
      m_lon(LON_INVALID),
      m_alt(ALT_INVALID),
      m_spotsize(SPOTSIZE_INVALID)
    { }

    //! Alternate Constructor
    AV4DTrajectoryNodeDepricated(Type type, TurnDirection turnDirection, double turnRadius,
                       double lat, double lon, double alt, const AVDateTime& eta,
                       double spotsize)
    : m_type(type),
      m_turnDirection(turnDirection),
      m_turnRadius(turnRadius),
      m_lat(lat),
      m_lon(lon),
      m_alt(alt),
      m_eta(eta),
      m_spotsize(spotsize)
    { }

    //! Destructor
    virtual ~AV4DTrajectoryNodeDepricated() { }

    //! equal operator
    inline bool operator==(const AV4DTrajectoryNodeDepricated& other) const
    {
        return (m_type == other.m_type &&
                m_turnDirection == other.m_turnDirection &&
                m_turnRadius == other.m_turnRadius &&
                m_lat == other.m_lat &&
                m_lon == other.m_lon &&
                m_alt == other.m_alt &&
                m_eta == other.m_eta &&
                m_spotsize == other.m_spotsize);
    }

    //! unequal operator
    inline bool operator!=(const AV4DTrajectoryNodeDepricated& other) const
    { return !(*this == other); }

    //! invalidates all fields
    inline void invalidate() {
        m_type          = UnknownType;
        m_turnDirection = UnknownTurn;
        m_turnRadius    = TURNRADIUS_INVALID;
        m_lat           = LAT_INVALID;
        m_lon           = LON_INVALID;
        m_alt           = ALT_INVALID;
        m_spotsize      = SPOTSIZE_INVALID;
        m_eta           = AVDateTime();
    }

    //! Decodes the internal data from the string.
    /*! \param nodeStr String holding the data to be decoded
        \param msgTime The message timestamp to derive the date for the
               time contained in nodeString
        The format is shown in AAG_ACG_ICD_V1.0.pdf and described in more detail
        in B737_FMCS_Update_10.6_Program_SRD_for_the_Intent_Bus_Feature_Rev12.doc.
        For deriving the date the following formula is used: If the time
        specified in the string is smaller than the message time and the
        difference is more than 12 hours it is assumed to be from the next day.
        If the time specified in the string is bigger than the message time and
        the difference is more than 12 hours it is assumed to belong to the
        previous day.
        \return true on success, false otherwise
    */
    virtual bool decode(const QString& nodeStr, const AVDateTime& msgTime,
                        const QString& delim = ",");

    //! Sets the type of node
    inline void setType(Type type) { m_type = type; }

    //! Returns the type of node
    inline Type type() const { return m_type; }

    //! Sets the turn direction
    inline void setTurnDirection(TurnDirection turnDirection) { m_turnDirection = turnDirection; }

    //! Returns the turn direction
    inline TurnDirection turnDirection() const { return m_turnDirection; }

    //! Sets the turn radius in NM
    inline void setTurnRadius(double turnRadius) { m_turnRadius = turnRadius; }

    //! Returns the turn radius in NM
    inline double turnRadius() const { return m_turnRadius; }

    //! Returns if the turn radius is valid
    inline bool hasValidTurnRadius() const { return (m_turnRadius > TURNRADIUS_INVALID); }

    //! Invalidates the turn radius
    inline void invalidateTurnRadius() { m_turnRadius = TURNRADIUS_INVALID; }

    //! Sets the nodes latitude
    inline void setLatitude(double lat) { m_lat = lat; }

    //! Returns the nodes latitude
    inline double latitude() const { return m_lat; }

    //! Returns if the latitude is valid
    inline bool hasValidLatitude() const { return (m_lat > LAT_INVALID); }

    //! Invalidates the latitude
    inline void invalidateLatitude() { m_lat = LAT_INVALID; }

    //! Sets the nodes longitude
    inline void setLongitude(double lon) { m_lon = lon; }

    //! Returns the nodes longitude
    inline double longitude() const { return m_lon; }

    //! Returns if the longitude is valid
    inline bool hasValidLongitude() const { return (m_lon > LON_INVALID); }

    //! Invalidates the longitude
    inline void invalidateLongitude() { m_lon = LON_INVALID; }

    //! Invalidates the latitude and longitude
    inline void invalidatePosition() { m_lon = LON_INVALID; m_lat = LAT_INVALID; }

    //! Returns true if lat or lon is unequal 0.0
    inline bool hasZeroPosition() const
    {
        return (AVFLOATEQ(0.0, m_lat, 1e-10) && AVFLOATEQ(0.0, m_lon, 1e-10));
    }

    //! Returns true if lat and lon are greater than LAT_INVALID and LON_INVALID
    inline bool hasValidPosition() const
    {
        return (m_lat > LAT_INVALID && m_lon > LON_INVALID);
    }

    //! Sets the nodes altitude in feet
    inline void setAltitude(double alt) { m_alt = alt; }

    //! Returns the nodes altitude in feet
    inline double altitude() const { return m_alt; }

    //! Returns if the altitude is valid
    inline bool hasValidAltitude() const { return (m_alt > ALT_INVALID); }

    //! Invalidates the altitude
    inline void invalidateAltitude() { m_alt = ALT_INVALID; }

    //! Sets the estimated time of arrival at the node
    inline void setEta(const AVDateTime& eta) { m_eta = eta; }

    //! Returns the estimated time of arrival at the node
    inline const AVDateTime& eta() const { return m_eta; }

    //! Sets the spotsize of the node
    inline void setSpotsize(const double& spotsize) { m_spotsize = spotsize; }

    //! Returns the spotsize of the node
    inline const double& spotsize() const { return m_spotsize; }

    //! Returns if the spot size is valid
    inline bool hasValidSpotsize() const { return (m_spotsize > SPOTSIZE_INVALID); }

    //! Invalidates the spot size
    inline void invalidateSpotsize() { m_spotsize = SPOTSIZE_INVALID; }

    //! Returns a string represenation of the node
    QString toString() const;

    //! add the given number of seconds to the point if it has a valid time
    inline void addSecs(int secs) { if (m_eta.isValid()) m_eta = m_eta.addSecs(secs); }

    //! add the given offset <ft> to the altitude of the point if it has a valid altitude
    inline void addAltitudeOffset(double alt_offset) {
        if (hasValidAltitude()) m_alt += alt_offset;
    }

protected: // members

    Type          m_type;             //!< node type
    TurnDirection m_turnDirection;    //!< turn direction (L/R)
    double        m_turnRadius;       //!< turn radius in NM (0="knick",>0 valid, <0 invalid)
    double        m_lat;              //!< latitude
    double        m_lon;              //!< longitude
    double        m_alt;              //!< altitude in feet
    AVDateTime    m_eta;              //!< estimated time of arrival
    double        m_spotsize;         //!< spotsize of the waypoint

    // ATTENTION: When adding field, do not forget to adapt the
    // operator== , operator>> and operator<< methods!!
};

AVLIB_EXPORT QDataStream& operator>>(QDataStream &s, AV4DTrajectoryNodeDepricated& node);
AVLIB_EXPORT QDataStream& operator<<(QDataStream &s, const AV4DTrajectoryNodeDepricated& node);

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

//! Represents an ordered list of trajectory nodes
//! ATTENTION: This class is depricated.
//! \sa AVMsgTrajectoryNode \sa AVMsgListTrajectoryNode \sa AVMsgDictTrajectory
class AVLIB_EXPORT AV4DTrajectoryDepricated : public QList<AV4DTrajectoryNodeDepricated>
{
public:

    //! Standard Constructor
    AV4DTrajectoryDepricated() { }

    //! Destructor
    virtual ~AV4DTrajectoryDepricated() { }

    //! Decodes the internal data from the string.
    /*! \param trajectoryStr String holding the data to be decoded
        \param msgTime The message timestamp to derive the date for the
               times contained in trajectoryStr
        \param nodeDelim Delimiter between IDL Nodes
        \param nodeEleDelim Delimiter between IDL Node Elements (point type,
               position, altitude, etc.)
        \return number of decoded trajectory nodes, -1 on error
    */
    virtual int decode(const QString& trajectoryStr,
                       const AVDateTime& msgDt,
                       const QString& nodeDelim =    ".",
                       const QString& nodeEleDelim = ",");

    //! Returns a string represenation of the node
    QString toString() const;

    //! add the given number of seconds to each trajectory point with a valid time
    void addSecs(int secs);

    //! add the given offset <ft> to the altitude of each point
    /*! \note there is no check if the actual altitude of the point is not set (0.0)
     */
    void addAltitudeOffset(double alt_offset);
};

uint qHash(const AV4DTrajectoryNodeDepricated& key, uint seed);


#endif /* __AV4DTRAJECTORY_H__ */

// End of file
