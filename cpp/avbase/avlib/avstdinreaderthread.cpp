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
    \author  Christian Muschick, c.muschick@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief
*/



// system includes
#include <iostream>
#include <cerrno>

// QT includes
#include <QtGlobal>
#include <QVector>

// local includes
#include "avlog.h"
#include "avmisc.h"

#include "avstdinreaderthread.h"

// include unix headers for low level read
#if defined (Q_OS_UNIX)
#include <unistd.h>
#include <sys/time.h>
#endif

#if defined (Q_OS_WIN32)
#include <conio.h>
#endif

///////////////////////////////////////////////////////////////////////////////

AVStdinReaderThread::AVStdinReaderThread() : AVThread("AVStdinReaderThread")
{
    AVCONNECT(this, SIGNAL(signalInternalThreadStdin(const QByteArray&)),
              this, SIGNAL(signalStdin(const QByteArray&)), Qt::QueuedConnection);
    AVCONNECT(this, SIGNAL(signalInternalThreadStdinError()),
              this, SIGNAL(signalStdinError()), Qt::QueuedConnection);
    AVCONNECT(this, SIGNAL(signalInternalThreadStdinClosed()),
              this, SIGNAL(signalStdinClosed()), Qt::QueuedConnection);
}

///////////////////////////////////////////////////////////////////////////////

AVStdinReaderThread::~AVStdinReaderThread()
{
}

///////////////////////////////////////////////////////////////////////////////

void AVStdinReaderThread::runThread()
{
    while (!checkForStop())
    {
#if defined(Q_OS_WIN32)
        // Windows console input handling currently is most rudimentary, special handling for "windows raw mode" in avconsole?
        // Note: Using WaitForSingleObject here does not work as expected, the subsequent read call can block. See e.g.
        // http://www.coldestgame.com/site/blog/cybertron/non-blocking-reading-stdin-c
        std::string input_string;
        std::getline(std::cin, input_string);

        if(!input_string.empty())
        {
            QByteArray input_buffer = QByteArray::fromStdString(input_string) + '\n';
            emit signalInternalThreadStdin(input_buffer);
        }
        Sleep(10);

#endif // #if defined(Q_OS_WIN32)
#if defined(Q_OS_UNIX)
        const uint BUF_SIZE = 256;
        static char buf[BUF_SIZE];

        fd_set stdinfd;
        FD_ZERO( &stdinfd );
        // Avoid platform issues and simply define this here...
        static const uint STDIN = 0;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
        FD_SET( STDIN, &stdinfd );
#pragma GCC diagnostic pop
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 10000;
        int ready = select( 1, &stdinfd, NULL, NULL, &tv );
        if( ready > 0 )
        {
            int read_bytes = read(STDIN, buf, BUF_SIZE);
            if (read_bytes == -1)
            {
                AVLogger->Write(LOG_ERROR, "AVStdinReaderThread::runThread: "
                                "read returned -1");
                emit signalInternalThreadStdinError();
                break;
            }
            if (read_bytes == 0)
            {
                AVLogger->Write(LOG_INFO, "AVStdinReaderThread::runThread: stdin closed.");
                emit signalInternalThreadStdinClosed();
                break;
            }

            emit signalInternalThreadStdin(QByteArray(buf,read_bytes));

        } else if (ready == -1)
        {
            // This will happen if suspended.
            if (errno == EINTR) continue;

            AVLogger->Write(LOG_ERROR, "StdinReaderThread::runThread: "
                            "select returned an error: %i", errno);
            emit signalInternalThreadStdinError();
            break;
        }
#endif // #if defined(Q_OS_UNIX)
    }
    AVLogger->Write(LOG_INFO, "AVStdinReaderThread::runThread finished");
}


///////////////////////////////////////////////////////////////////////////////

void AVStdinReaderThread::stopThread()
{
#if defined(Q_OS_WIN32)
    // Nonblocking IO on Windows/Cygwin is not possible, so terminate after a short wait
    if (!wait(50))
    {
        AVLogDebug << "AVStdinReaderThread::stopThread: Still running, will terminate";
        terminate();
    }
#endif // #if defined(Q_OS_WIN32)
}


// End of file
