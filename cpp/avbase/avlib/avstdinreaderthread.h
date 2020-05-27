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

#if !defined AVSTDINREADERTHREAD_H_INCLUDED
#define AVSTDINREADERTHREAD_H_INCLUDED

#include "avlib_export.h"
#include "avthread.h"

/////////////////////////////////////////////////////////////////////////////
/**
 *  This class provides a convenient way to read from blocking stdin (every input
 *  has to be confirmed with enter by the user).
 *  Input is emitted via signals in the main thread's context. Note that input is not necessarily
 *  emitted after each line (e.g. if feeding stdin from another process and not the keyboard).
 */
class AVLIB_EXPORT AVStdinReaderThread : public QObject, public AVThread
{
    Q_OBJECT
public:
    AVStdinReaderThread();
    ~AVStdinReaderThread() override;
signals:
    void signalStdin(const QByteArray& input);
    void signalStdinError();
    void signalStdinClosed();

    void signalInternalThreadStdin(const QByteArray& input);
    void signalInternalThreadStdinError();
    void signalInternalThreadStdinClosed();

protected:
    void runThread() override;
    void stopThread() override;
};


#endif

// End of file
