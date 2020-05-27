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
    \author  Stefan Kunz, s.kunz@avibit.com
    \author  QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief   Abstract base class for Socket-Device functionality
*/


#ifndef __AVSOCKETDEVICEBASE_H__
#define __AVSOCKETDEVICEBASE_H__

// only needed for enums
#include <qabstractsocket.h>

//! Abstract base class for Socket-Device functionality
class AVSocketDeviceBase
{
public:
    //! Standard Constructor
    /*! \param type The type of socket which is either TcpSocket (formerly: Stream) or
               UdpSocket (formerly: Datagram).
     */
    AVSocketDeviceBase(QAbstractSocket::SocketType type = QAbstractSocket::TcpSocket)
        : m_qsd_error(QAbstractSocket::UnknownSocketError), m_qsd_type(type) {}

    //! Destructor
    virtual ~AVSocketDeviceBase() {}

    //! Reads data from the socket
    /*! Reads maxlen bytes from the socket into data and returns the number
        of bytes read.
        \param data array where the read data is written to
        \param maxlen maximum length in bytes that is read
        \return the actual number of bytes read or -1 if an error occured
     */
    virtual qint64 readBlock(char *data, qint64 maxlen) = 0;

    //! Write data to the socket
    /*! Writes len bytes to the socket from data and returns the number of
        bytes written.
        \param data array where the data is stored
        \param len length in bytes that should be written
        \return the number of bytes written or -1 if an error occured
     */
    virtual qint64 writeBlock(const char *data, qint64 len) = 0;

    //! Write data to the socket
    /*! Writes len bytes to the socket from data and returns the number of
        bytes written.
        \param data array where the data is stored
        \param len length in bytes that should be written
        \param addr the address where to send the data
        \param port the port where to send the data
        \return the number of bytes written or -1 if an error occured
     */
    virtual qint64 writeBlock(const char* data, qint64 len,
                              const QHostAddress& addr, quint16 port) = 0;

    //! Binds the socket to the specified address/port
    /*! Assigns a name to an unnamed socket using the specified address and
        port. If the operation succeeds, bind() returns true otherwise it
        returns false without changing what port() and address() return.
        \param addr the address to bind to
        \param port the port to bind to
        \return true if successful, false otherwise
     */
    virtual bool bind(const QHostAddress& addr, quint16 port) = 0;

    //! Sets the socket blocking state
    /*! Makes the socket blocking if enable is true or nonblocking if enable is
        false. Sockets are blocking by default.
        \param enable If true socket will be made blocking otherwise it will
               be made non-blocking.
     */
    virtual void setBlocking(bool enable) = 0;

    //! Returns true if the socket is in blocking mode; otherwise returns false.
    virtual bool blocking() const = 0;

    //! Returns the socket number
    /*! \return the socket number or -1 if not valid
     */
    virtual int socket() const = 0;

    //! Checks whether the address of this socket may be used by other sockets
    /*! \return true if the address of this socket can be used by other sockets
        at the same time, and false if this socket claims exclusive ownership.
     */
    virtual bool addressReusable() const = 0;

    //! Sets whether the address of this socket to be usable by other sockets too
    /*! Sets the address of this socket to be usable by other sockets too if
        enable is true, and to be used exclusively by this socket if enable is
        false.

        When a socket is reusable, other sockets can use the same port number
        (and IP address), which is generally useful. Of course other sockets
        cannot use the same (address,port,peer-address,peer-port) 4-tuple as
        this socket, so there is no risk of confusing the two TCP connections.

        \param enable If true, socket is set to be reusable, otherwise it is set
               to be not-reusable.
     */
    virtual void setAddressReusable(bool enable) = 0;

    //! Returns true if this is a valid socket; otherwise returns false.
    virtual bool isValid() const = 0;

    //! Returns the port number of this socket device
    /*! \return port number of this socket device in native byte order
     */
    virtual quint16 port() const = 0;

    //! Returns the port number of the port this socket device is connected to
    /*! \return the port number of the port this socket device is connected to.
                For Datagram sockets, this is the source port of the last packet
                received (in native byte order).
     */
    virtual quint16 peerPort() const = 0;

    //! Returns the address of this socket device
    virtual QHostAddress address() const = 0;

    //! Returns the source address this socket device is connected to
    /*! \return the source address this socket device is connected to.
                For Datagram sockets, this is the source address of the last
                packet received.
     */
    virtual QHostAddress peerAddress() const = 0;

    //! Returns the number of bytes available for reading
    /*! \return the number of bytes available for reading, or -1 if an error
                occurred.
     */
    virtual qint64 bytesAvailable() const = 0;

    //! Returns the first error seen
    virtual QAbstractSocket::SocketError error() const
    {
        return m_qsd_error;
    }

    //! Returns the socket type
    /*! \return the socket type which is either QSocketDevice::Stream or
                QSocketDevice::Datagram.
     */
    virtual QAbstractSocket::SocketType type() const
    {
        return m_qsd_type;
    }
    //! Sets the TTL field for multicasting on the socket.
    /*! Sets teh socketoption IP_MULTICAST_TTL on the socket.
        \param ttl cout of hops before a multicast packet expires.
     */
    virtual void setMulticastTtl(uchar ttl) = 0;

protected: // methods
    //! Allows subclasses to set the error state to err.
    virtual void setError(QAbstractSocket::SocketError err)
    {
        m_qsd_error = err;
    }

private: // members
    QAbstractSocket::SocketError  m_qsd_error; //!< last error state
    QAbstractSocket::SocketType   m_qsd_type;  //!< socket-device type

private:
    //! Hidden copy-constructor
    AVSocketDeviceBase(const AVSocketDeviceBase&);
    //! Hidden assignment operator
    const AVSocketDeviceBase& operator = (const AVSocketDeviceBase&);
};


#endif /* __AVSOCKETDEVICEBASE_H__ */

// End of file
