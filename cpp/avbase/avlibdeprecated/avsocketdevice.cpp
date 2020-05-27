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
    \brief   Implements a QSocketDevice derived class for full block I/O
*/



// system includes
#include <qglobal.h>
#if defined(Q_OS_WIN32)
#include <winsock2.h>
#else
#if defined(Q_OS_OSF)
#include <sys/time.h>
#endif
#include <sys/socket.h>
#endif
#include <errno.h>
#if defined(Q_OS_UNIX)
#include <sys/select.h>
#endif

// local includes
#include "avdeprecate.h"
#include "avlog.h"
#include "avmisc.h"
#include "avsocketdevice.h"

///////////////////////////////////////////////////////////////////////////////

AVSocketDevice::AVSocketDevice() :
        QTcpSocket(), AVSocketDeviceBase(QAbstractSocket::TcpSocket), m_has_io_error(false),
        m_blocking(true)
{
}

///////////////////////////////////////////////////////////////////////////////

AVSocketDevice::AVSocketDevice(int socket) :
        QTcpSocket(), AVSocketDeviceBase(QAbstractSocket::TcpSocket), m_has_io_error(false),
        m_blocking(true)
{
    setSocketDescriptor(socket);
}

///////////////////////////////////////////////////////////////////////////////
AVSocketDevice::~AVSocketDevice()
{
}

///////////////////////////////////////////////////////////////////////////////

bool AVSocketDevice::connect(const QHostAddress &addr, quint16 port)
{
    QTcpSocket::connectToHost(addr, port);
    return waitForConnected();
}

///////////////////////////////////////////////////////////////////////////////

bool AVSocketDevice::connect(const QHostAddress & addr, quint16 port, int connect_timeout_ms)
{
    QTime elapsed_connection_time;
    elapsed_connection_time.start();

    // connect_timeout_ms:  0: causing the function to return  immediately.  (This  is  useful  for polling.)
    //                     <0: the function can block indefinitely
    //                     >0: timeout is used
    QTcpSocket::connectToHost(addr, port);
    if (!waitForConnected(connect_timeout_ms))
    {
        if(connect_timeout_ms == 0)
            return false;

        QAbstractSocket::SocketError err = QTcpSocket::error();
        AVLogError << AVLogStream::SuppressTransientProblems
                   << "AVSocketDevice::connect: failed, error="
                   << err << " for addr=" << addr.toString() << "/port=" << port;
        close();
        return false;
    }

    // We are connected. Now wait until the socket is ready (writeable) respecting given connect_timeout_ms. But try at least once.
    int remaining_connect_time_ms = -1;
    if(connect_timeout_ms >= 0)
    {
        remaining_connect_time_ms = connect_timeout_ms - elapsed_connection_time.elapsed();
        if(remaining_connect_time_ms < 0) remaining_connect_time_ms = 0;
    }
    elapsed_connection_time.start();

    // get raw socket
    int socket_fd = socket();

    // prepare parameters for select(), only write set is needed
    fd_set wset;
    FD_ZERO(&wset);
#if defined(Q_OS_WIN32)
    FD_SET((uint) socket_fd, &wset);
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
    FD_SET(socket_fd, &wset);
#pragma GCC diagnostic pop
#endif
    int secs = remaining_connect_time_ms/1000;
    int musecs = (remaining_connect_time_ms - 1000*secs)*1000;

    struct timeval tval;
    tval.tv_sec = secs;
    tval.tv_usec = musecs;

    // Call select to wait for writability or timeout or error
    // use getsockopt to find any errors from select
    // wait for connect
    int n = select(socket_fd + 1, NULL, &wset, NULL, (remaining_connect_time_ms >= 0) ? &tval : NULL);

    // see http://cr.yp.to/docs/connect.html for testing if the connection was succesful

    // timeout
    if (n == 0)
    {
        if(connect_timeout_ms != 0)
        {
            AVLogger->Write(LOG_ERROR_ERRNO, "AVSocketDevice::connect: *** CONNECT TIMEOUT (select) for addr=%s/port=%d, errno=%d",
                            qPrintable(addr.toString()), port, errno);
            close();
        }
        setError(QAbstractSocket::ConnectionRefusedError);
        return false;
    }

    // error
#if defined(Q_OS_WIN32)
    if (n == SOCKET_ERROR)
#else
    if (n < 0)
#endif
    {
        AVLogErrorErrno << AVLogStream::SuppressTransientProblems
                        << "AVSocketDevice::connect: *** CONNECT ERROR (select) for addr=" << addr.toString()
                        << "/port=" << port << ", errno=" << errno;
        close();
        setError(QAbstractSocket::UnknownSocketError);
        return false;
    } else {
        // The descriptor should now be writable unless there was a timeout
        int sock_error = 0;
#if defined(Q_OS_WIN32)
        int len = sizeof(sock_error);
        if (getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, (char*)(&sock_error), &len) != 0)
#else
#if defined(Q_OS_OSF)
        int len = sizeof(sock_error);
#else
        socklen_t len = sizeof(sock_error);
#endif
        if (getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &sock_error, &len) != 0)
#endif
        {
            AVLogErrorErrno << AVLogStream::SuppressTransientProblems
                            << "AVSocketDevice::connect: *** CONNECT ERROR (getsockopt) for addr="
                            << addr.toString() << "/port=" << port << ", errno=" << errno;
            close();
            setError(QAbstractSocket::UnknownSocketError);
            return false;
        }

        if (sock_error != 0)
        {
            AVLogErrorErrno << AVLogStream::SuppressTransientProblems
                            << "AVSocketDevice::connect: *** CONNECT ERROR (sock_error) for addr=" << addr.toString()
                            << "/port=" << port << ", sock_error=" << sock_error;
            close();
            setError(QAbstractSocket::UnknownSocketError);
            return false;
        }

    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

qint64 AVSocketDevice::readBlock(char *data, qint64 len)
{
    // Technically possible, but probably an error...
    AVASSERT(data != 0);
    AVASSERT(len  != 0);

    return QTcpSocket::read(data, len);
}

///////////////////////////////////////////////////////////////////////////////

qint64 AVSocketDevice::writeBlock(const char*, qint64, const QHostAddress&, quint16)
{
    // Abort, this method is only for UDP sockets
    AVASSERT(type() == QAbstractSocket::UdpSocket); // never true in this class
    return -1;
}

///////////////////////////////////////////////////////////////////////////////

qint64 AVSocketDevice::writeBlock(const char *data, qint64 len)
{
    if (len == 0) return 0;
    AVASSERT(data != 0);

    if (!isValid())
    {
        // Catch this so there is no warning during startup/shutdown caused by qt message handling in qtMessageHandler
        AVLogWarning << AVLogStream::SuppressTransientProblems
                     << "AVSocketDevice::writeBlock: socket device not valid";
        m_has_io_error = true;
        setError(QAbstractSocket::NetworkError);
        return -1;
    }

    const qint64 ret = QTcpSocket::writeData(data, len);
    if (ret >= 0 && m_blocking) {
        if (!QTcpSocket::waitForBytesWritten()) {
            m_has_io_error = true;
            setError(QAbstractSocket::NetworkError);
            return -1;
        }
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////////////

qint64 AVSocketDevice::writeData(const char *data, qint64 len)
{
    return writeBlock(data, len);
}

///////////////////////////////////////////////////////////////////////////////

bool AVSocketDevice::hasError() const
{
    return m_has_io_error || (QTcpSocket::error() != UnknownSocketError && QTcpSocket::error() != SocketTimeoutError);
}

///////////////////////////////////////////////////////////////////////////////

QAbstractSocket::SocketError AVSocketDevice::error() const
{
    if (!m_has_io_error) return QTcpSocket::error();
    return QAbstractSocket::NetworkError;
}

///////////////////////////////////////////////////////////////////////////////

QString AVSocketDevice::errorString() const
{
    return QTcpSocket::errorString();
}

///////////////////////////////////////////////////////////////////////////////

bool AVSocketDevice::isConnected()
{
    return state() == ConnectedState;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSocketDevice::setSendTimeOut(uint timeout)
{
    struct timeval to;
    to.tv_sec  = timeout/1000;
    to.tv_usec = (timeout - to.tv_sec*1000)*1000;

    // Make sure we have a valid socket -- otherwise setsockopt will crash
    // This can occur e.g. if a preceding write call caused the socket to be closed, and the socket status is not re-checked before calling
    // this method.
    if (socket() == -1)
    {
        AVLogError << AVLogStream::SuppressTransientProblems << Q_FUNC_INFO << ": socket is invalid";
        return false;
    }

#if defined(Q_OS_WIN32)
    int set_error = setsockopt(socket(), SOL_SOCKET, SO_SNDTIMEO,
                               (const char*)&to, sizeof(struct timeval));
#else
    int set_error = setsockopt(socket(), SOL_SOCKET, SO_SNDTIMEO, &to,
                               sizeof(struct timeval));
#endif
    if (set_error == -1) {
        AVLogger->Write(LOG_ERROR_ERRNO, "AVSocketDevice::setSendTimeOut: "
                        "cannot set send timeout");
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSocketDevice::setReceiveTimeOut(uint timeout)
{
    struct timeval to;
    to.tv_sec  = timeout/1000;
    to.tv_usec = (timeout - to.tv_sec*1000)*1000;

#if defined(Q_OS_WIN32)
    int set_error = setsockopt(socket(), SOL_SOCKET, SO_RCVTIMEO,
                               (const char*)&to, sizeof(struct timeval));
#else
    int set_error = setsockopt(socket(), SOL_SOCKET, SO_RCVTIMEO, &to,
                               sizeof(struct timeval));
#endif
    if (set_error == -1) {
        AVLogger->Write(LOG_ERROR_ERRNO, "AVSocketDevice::setReceiveTimeOut: "
                        "cannot set receive timeout");
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSocketDevice::getReceiveTimeOut(uint &timeout)
{
    struct timeval to;

#if defined(Q_OS_WIN32)
    int FAR optlen = sizeof(struct timeval);
    int get_error = getsockopt(socket(), SOL_SOCKET, SO_RCVTIMEO,
                               (char*)&to, &optlen);
#else
#if defined(Q_OS_OSF)
    int optlen = sizeof(struct timeval);
#else
    socklen_t optlen = sizeof(struct timeval);
#endif
    int get_error = getsockopt(socket(), SOL_SOCKET, SO_RCVTIMEO, &to,
                               &optlen);
#endif
    if (get_error == -1) {
        AVLogger->Write(LOG_ERROR_ERRNO, "AVSocketDevice::getReceiveTimeOut: "
                        "cannot get receive timeout");
        return false;
    }

    timeout = to.tv_sec*1000 + to.tv_usec/1000;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void AVSocketDevice::setMulticastTtl(uchar ttl)
{
    setSocketOption(QAbstractSocket::SocketOption::MulticastTtlOption, (int)ttl);
}

///////////////////////////////////////////////////////////////////////////////

bool AVSocketDevice::getSendTimeOut(uint &timeout)
{
    struct timeval to;

#if defined(Q_OS_WIN32)
    int FAR optlen = sizeof(struct timeval);
    int get_error = getsockopt(socket(), SOL_SOCKET, SO_SNDTIMEO,
                               (char*)&to, &optlen);
#else
#if defined(Q_OS_OSF)
    int optlen = sizeof(struct timeval);
#else
    socklen_t optlen = sizeof(struct timeval);
#endif
    int get_error = getsockopt(socket(), SOL_SOCKET, SO_SNDTIMEO, &to,
                               &optlen);
#endif
    if (get_error == -1) {
        AVLogger->Write(LOG_ERROR_ERRNO, "AVSocketDevice::getSendTimeOut: "
                        "cannot get send timeout");
        return false;
    }

    timeout = to.tv_sec*1000 + to.tv_usec/1000;
    return true;
}

// End of file
