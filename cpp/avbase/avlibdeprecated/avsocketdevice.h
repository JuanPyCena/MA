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
    \author  Dr. Thomas Leitner, t.leitner@avibit.com
    \author  QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief   Defines a QSocketDevice derived class for full block I/O
*/

#if !defined(AVSOCKETDEVICE_H_INCLUDED)
#define AVSOCKETDEVICE_H_INCLUDED

#include <QTcpSocket>
#include <QHostAddress>

// local includes
#include "avsocketdevicebase.h"
#include "avlibdeprecated_export.h"

//! max. I/O block size.
//! TODO where does this value come from? why is it used here, and not only in avmsgsbase as buffer
//! size? Also see AVSocketDevice::readBlock.
const int AVSocketMaxIO = 16384;

///////////////////////////////////////////////////////////////////////////////
//! TODO
class AVLIBDEPRECATED_EXPORT AVSocketDevice : public QTcpSocket, public AVSocketDeviceBase
{
    Q_OBJECT
public:
    AVSocketDevice();
    explicit AVSocketDevice(int socket);
    ~AVSocketDevice() override;

    // Porting note: void disableReadWriteLoopHack() is now setBlocking(false)

    /*!
     * Emulate what the 2-args connect did in the Qt3 code: basic call to Qt,
     * none of the logging done by the 3args connect.
     */
    bool connect(const QHostAddress & addr, quint16 port);

    /*! Connects to the IP address and port specified by addr and port with
        the give timeout connect_timeout_ms.
        Returns TRUE if it establishes a connection; otherwise returns FALSE.
        If it returns FALSE, error() explains why.
        When the connection fails the socket is closed and the AVSocketDevice object
        CANNOT be used for another connect attempt.
        See SWE-2698 for how a connection is established on different operating systems (linux, windows).
        \param addr server address where to connect to
        \param port server port where to connect to
        \param connect_timeout_ms timeout in msecs. It may be zero, causing the function to
                                  return  immediately.  (This  is  useful  for polling.)
                                  If timeout is <0 (no timeout) the function can block
                                  indefinitely.
        \return Returns true if it establishes a connection; otherwise returns
                false. If it returns false, error() explains why.
    */
    bool connect(const QHostAddress & addr, quint16 port, int connect_timeout_ms);

    //! return true if the socket is still connected to the peer
    bool isConnected();

    /*! Reads the specified block of data from the socket (same as QTcpSocket::read).

        Note that many processes use QDataStream together with AVSocketDevice (e.g. daq_smru,
        client handler threads) and rely on the fact that all requested data is read.
        To make sure this works, make sure you're doing something like

        while (socket->bytesAvailable() < numBytes)
        {
            if (!socket->waitForReadyRead(30000))
            {
                return false;
            }
        }
        socket->readBlock(...) or socket->read(...)

        \param  data a pointer to the data buffer
        \param  maxlen the number of bytes to be read
        \return The number of actually read bytes, or -1 on error.
    */
    qint64 readBlock(char *data, qint64 maxlen) override;

    //! not allowed for a TCP socket
    qint64 writeBlock(const char* data, qint64 len, const QHostAddress& addr,
                              quint16 port) override;

    /*! Writes the specified block of data to the socket.

        By default this is blocking, i.e. the routine always writes the complete
        block unless a socket error occurs.

        setBlocking(false) turns this off, i.e. the writing will happen when the
        thread goes back to the event loop. Do not use setBlocking(false) in a
        thread without an event loop.

        \param data a pointer to the data buffer
        \param len the number of bytes to write
        \return len if successful, -1 if not
    */
    qint64 writeBlock(const char *data, qint64 len) override;

    // QTcpSocket reimplementations
    qint64 writeData(const char *data, qint64 len) override;

    //! return the status of the class. if an error occured in the last
    //! I/O call, the error number is returned here.
    bool hasError() const;

    //! returns the last socket error
    QAbstractSocket::SocketError error() const override;

    //! returns description of the QSocketDevice::Error code
    QString errorString() const;

    //! not available for TCP sockets
    bool bind(const QHostAddress& addr, quint16 port) override
    {
        Q_UNUSED(addr);
        Q_UNUSED(port);
        Q_ASSERT(false);
        return false;
    }

    //! always true
    bool addressReusable() const override
    {
        return true;
    }

    //! useless call, since QTcpSocket always sets this to true
    void setAddressReusable(bool) override
    {
    }

    //! only calls QSocketDevice::bytesAvailable()
    qint64 bytesAvailable() const override
    {
        return QTcpSocket::bytesAvailable();
    }

    //! only calls QSocketDevice::setBlocking(enable)
    void setBlocking(bool enable) override
    {
        m_blocking = enable;
    }

    //! only calls QSocketDevice::blocking()
    bool blocking() const override
    {
        return m_blocking;
    }

    //! only calls QTcpSocket::socketDescriptor()
    int socket() const override
    {
        return QTcpSocket::socketDescriptor();
    }

    QAbstractSocket::SocketType type() const override
    {
        return QAbstractSocket::TcpSocket;
    }

    //! calls QTcpSocket::localPort()
    quint16 port() const override
    {
        return QTcpSocket::localPort();
    }

    //! only calls QTcpSocket::peerPort()
    quint16 peerPort() const override
    {
        return QTcpSocket::peerPort();
    }

    //! calls QTcpSocket::localAddress()
    QHostAddress address() const override
    {
        return QTcpSocket::localAddress();
    }

    //! only calls QTcpSocket::peerAddress()
    QHostAddress peerAddress() const override
    {
        return QTcpSocket::peerAddress();
    }

    //! only calls QTcpSocket::isValid()
    bool isValid() const override
    {
        return QTcpSocket::isValid();
    }

    //! Set the send timeout. Change the timeout only if you know what you are
    //! doing. In most cases it is not necessary to change the timeout. In
    //! 99.9% this is only a workaround for a bug elsewhere!
    /*! Specify the sending timeouts until reporting an error using setsockopt.
        \param timeout the desired timeout in milliseconds
        \return true if successful, false if not
     */
    bool setSendTimeOut(uint timeout);

    //! Set the receive timeout. Change the timeout only if you know what you are
    //! doing. In most cases it is not necessary to change the timeout. In
    //! 99.9% this is only a workaround for a bug elsewhere!
    /*! Specify the reading timeouts until reporting an error using setsockopt.
        \param timeout the desired timeout in milliseconds
        \return true if successful, false if not
     */
    bool setReceiveTimeOut(uint timeout);

    //! Gets the send timeout.
    /*! Get the sending timeouts until reporting an error using getsockopt.
        \param timeout the timeout in milliseconds
        \return true if successful, false if not
     */
    bool getSendTimeOut(uint &timeout);

    //! Gets the receive timeout.
    /*! Get the reading timeouts until reporting an error using getsockopt.
        \param timeout the timeout in milliseconds
        \return true if successful, false if not
     */
    bool getReceiveTimeOut(uint &timeout);

    void setMulticastTtl(uchar ttl) override;

protected:
    //! only calls QTcpSocket::setSocketError(err)
    void setError(QAbstractSocket::SocketError err) override
    {
        QTcpSocket::setSocketError(err);
    }

private:
    bool m_has_io_error;           //!< last I/O status. true=error, false=success
    bool m_blocking;
};

#endif

// End of file
