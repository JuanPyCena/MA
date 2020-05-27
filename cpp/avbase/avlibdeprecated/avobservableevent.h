///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// QT-Version: QT5
// Copyright: AviBit data processing GmbH, 2001-2008
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Andreas Schuller
    \author  QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief   Declaration of AVObservableEvents
*/

#if !defined(AVOBSERVABLEEVENT_H_INCLUDED)
#define AVOBSERVABLEEVENT_H_INCLUDED

// Qt includes
#include <qstring.h>
#include "avlibdeprecated_export.h"
// forward declarations
class AVMsgRaw;
class AVObservable;
class AVObserver;

///////////////////////////////////////////////////////////////////////////////

class AVObservableEventVisitorBase
{
public:
    virtual ~AVObservableEventVisitorBase(){}
};

///////////////////////////////////////////////////////////////////////////////

template <class T>
class AVObservableEventVisitor : public AVObservableEventVisitorBase
{
public:
    AVObservableEventVisitor(){}
    ~AVObservableEventVisitor() override {}

public:
    virtual bool visit(const T &) = 0;
};

///////////////////////////////////////////////////////////////////////////////
//! Base class for events published by an AVObservable
class AVLIBDEPRECATED_EXPORT AVObservableEvent
{
    friend class AVObserver;
    friend class AVObservable;
public:
    enum Type
    {
        Null,
        Error,
        ClientStarted,
        ClientFinished,
        HostConnected,
        HostDisconnected,
        MessageTimeout,
        MessageReceived,
        HBTimeout,
        DSWRedirectStopOnError,
        ExecStandbyChange,
        TermaControlClientConnected,
        TermaControlClientDisconnected,
        TermaControlClientConnectionEstablished,
        TermaControlClientVideoSource,
        TermaControlClientRange,
        ByteArray
    };

public:
    AVObservableEvent();

    AVObservableEvent(const AVObservableEvent& src);

    virtual ~AVObservableEvent() {}

    Type type() const { return m_type; }

    const AVObservable* sender() const { return m_sender; }

    virtual bool accept(AVObservableEventVisitorBase &) const { return false; }

protected:
    explicit AVObservableEvent(Type type);

    template <class T>
    static bool acceptVisitor(T &visited, AVObservableEventVisitorBase &visitor)
    {
        AVObservableEventVisitor<T>* p = dynamic_cast<AVObservableEventVisitor<T>*> (&visitor);
        if (p != 0)
        {
            return p->visit(visited);
        }

        return false;
    }

    #define VISITABLE() \
        virtual bool accept(AVObservableEventVisitorBase &v) const override\
            { return acceptVisitor(*this, v); }
private:
    Type                      m_type;
    mutable AVObservable*     m_sender;
};

///////////////////////////////////////////////////////////////////////////////

class AVObservableEventHostInfo : public AVObservableEvent
{
public:
    ~AVObservableEventHostInfo() override {}

    QString addr() const { return m_addr; }
    void setAddr(const QString& addr){ m_addr = addr; }

    int port() const { return m_port; }
    void setPort(int port) { m_port = port; }

protected:
    AVObservableEventHostInfo(Type type) : AVObservableEvent(type){}
protected:
    QString m_addr;
    int     m_port;
};

///////////////////////////////////////////////////////////////////////////////

class AVObservableEventClientStarted : public AVObservableEventHostInfo
{
public:
    AVObservableEventClientStarted() : AVObservableEventHostInfo(ClientStarted) {}
    ~AVObservableEventClientStarted() override {}

protected:
    VISITABLE();
};

///////////////////////////////////////////////////////////////////////////////

class AVObservableEventClientFinished : public AVObservableEventHostInfo
{
public:
    AVObservableEventClientFinished() : AVObservableEventHostInfo(ClientFinished) {}
    ~AVObservableEventClientFinished() override {}

protected:
    VISITABLE();
};

///////////////////////////////////////////////////////////////////////////////

class AVObservableEventHostConnected : public AVObservableEventHostInfo
{
public:
    AVObservableEventHostConnected() : AVObservableEventHostInfo(HostConnected){}
    ~AVObservableEventHostConnected() override {}

protected:
    VISITABLE();
};

///////////////////////////////////////////////////////////////////////////////

class AVObservableEventHostDisconnected : public AVObservableEventHostInfo
{
public:
    AVObservableEventHostDisconnected() : AVObservableEventHostInfo(HostDisconnected){}
    ~AVObservableEventHostDisconnected() override {}

protected:
    VISITABLE();
};

///////////////////////////////////////////////////////////////////////////////

class AVObservableEventHBTimeout : public AVObservableEvent
{
public:
    AVObservableEventHBTimeout() : AVObservableEvent(HBTimeout){}
    ~AVObservableEventHBTimeout() override {}

protected:
    VISITABLE();
};

///////////////////////////////////////////////////////////////////////////////

class AVObservableEventError : public AVObservableEvent
{
public:
    AVObservableEventError() : AVObservableEvent(Error){}
    ~AVObservableEventError() override {}

protected:
    VISITABLE();
};

///////////////////////////////////////////////////////////////////////////////

class AVObservableEventDSWRedirectStopOnError : public AVObservableEvent
{
public:
    AVObservableEventDSWRedirectStopOnError() : AVObservableEvent(DSWRedirectStopOnError){}
    ~AVObservableEventDSWRedirectStopOnError() override {}

protected:
    VISITABLE();
};

///////////////////////////////////////////////////////////////////////////////

class AVObservableEventMessageTimeout : public AVObservableEvent
{
public:
    AVObservableEventMessageTimeout() : AVObservableEvent(MessageTimeout){}
    ~AVObservableEventMessageTimeout() override {}

protected:
    VISITABLE();
};

///////////////////////////////////////////////////////////////////////////////

class AVLIBDEPRECATED_EXPORT AVObservableEventMessageReceived : public AVObservableEvent
{
public:
    AVObservableEventMessageReceived();

    explicit AVObservableEventMessageReceived(AVMsgRaw* msg);

    AVObservableEventMessageReceived(const AVObservableEventMessageReceived& src);

    ~AVObservableEventMessageReceived() override {}

    AVMsgRaw* rawMsg() const { return m_raw_msg; }
    void setRawMsg(AVMsgRaw* msg) { m_raw_msg = msg; }

protected:
    VISITABLE();

private:
    AVMsgRaw*  m_raw_msg;
};

///////////////////////////////////////////////////////////////////////////////

class AVLIBDEPRECATED_EXPORT AVObservableEventExecStandbyChange : public AVObservableEvent
{
public:
    enum ExecState
    {
        Exec,
        Standby
    };

    AVObservableEventExecStandbyChange();

    explicit AVObservableEventExecStandbyChange(ExecState new_state);

    ~AVObservableEventExecStandbyChange() override {}

    ExecState newState() const { return m_new_state; }
    void setNewState(ExecState state) { m_new_state = state; }

protected:
    VISITABLE();
private:
    ExecState m_new_state;
};

///////////////////////////////////////////////////////////////////////////////

//! Event emitted when the control client connected to the terma control server
class AVObservableEventTermaControlClientConnected : public AVObservableEvent
{
public:
    AVObservableEventTermaControlClientConnected()
        : AVObservableEvent(TermaControlClientConnected) {}
    ~AVObservableEventTermaControlClientConnected() override {}

protected:
    VISITABLE();
};

///////////////////////////////////////////////////////////////////////////////

//! Event emitted when the control client has disconnected from the terma control server
class AVObservableEventTermaControlClientDisconnected : public AVObservableEvent
{
public:
    AVObservableEventTermaControlClientDisconnected()
        : AVObservableEvent(TermaControlClientDisconnected) {}
    ~AVObservableEventTermaControlClientDisconnected() override {}

protected:
    VISITABLE();
};

///////////////////////////////////////////////////////////////////////////////

//! Eevnt emitted when the control client has received Version information etc. from the terma
//! control server
class AVObservableEventTermaControlClientConnectionEstablished : public AVObservableEvent
{
public:

    AVObservableEventTermaControlClientConnectionEstablished()
        : AVObservableEvent(TermaControlClientConnectionEstablished),
          m_sweeps_per_full_scan(0),
          m_cells_per_sweep(0),
          m_sector_size(0)
    {}

    ~AVObservableEventTermaControlClientConnectionEstablished() override {}

    int sweepsPerFullScan() const { return m_sweeps_per_full_scan; }
    void setSweepsPerFullScan(int sweeps_per_full_scan) { m_sweeps_per_full_scan = sweeps_per_full_scan; }

    int cellsPerSweep() const { return m_cells_per_sweep; }
    void setCellsPerSweep(int cells_per_sweep) { m_cells_per_sweep = cells_per_sweep; }

    int sectorSize() const { return m_sector_size; }
    void setSectorSize(int sector_size) { m_sector_size = sector_size; }

protected:
    VISITABLE();
protected:
    int m_sweeps_per_full_scan;
    int m_cells_per_sweep;
    int m_sector_size;
};

///////////////////////////////////////////////////////////////////////////////

//! Event emitted when the control client has received information about the video source from
//! control server
/*! \param addr UDP multicast address or localhost for UDP unicast of the video source
    \param port UDP port of the video source
 */
class AVObservableEventTermaControlClientVideoSource : public AVObservableEventHostInfo
{
public:
    AVObservableEventTermaControlClientVideoSource()
        : AVObservableEventHostInfo(TermaControlClientVideoSource) {}
    ~AVObservableEventTermaControlClientVideoSource() override {}

protected:
    VISITABLE();
};

///////////////////////////////////////////////////////////////////////////////

//! Event emitted when the control client has received range information from
//! control server
/*! \param range range in meter received from the terma control server
 */
class AVObservableEventTermaControlClientRange : public AVObservableEvent
{
public:
    AVObservableEventTermaControlClientRange()
        : AVObservableEvent(TermaControlClientRange),
          m_range(0)
    {}

    ~AVObservableEventTermaControlClientRange() override {}

    int range() const { return m_range; }
    void setRange(int range) { m_range = range; }

protected:
    VISITABLE();
protected:
    int m_range;
};

///////////////////////////////////////////////////////////////////////////////

class AVObservableEventByteArray : public AVObservableEvent
{
public:
    AVObservableEventByteArray()
        : AVObservableEvent(ByteArray)
    {}

    ~AVObservableEventByteArray() override {}

    const QByteArray& data() const { return m_data; }

    //! makes a shallow copy!
    void setData(const QByteArray &data) { m_data = data; }

protected:
    VISITABLE();
protected:
    QByteArray m_data;
};

#endif

// End of file
