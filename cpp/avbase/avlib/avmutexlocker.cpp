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
    \author   Wolfgang Eder, w.eder@avibit.com
    \author   QT4-PORT: Karlheinz Wohlmuth, k.wohlmuth@avibit.com
    \brief    TODO
*/

#include "avlog.h"
#include "avmutexlocker.h"
#include "avsysprof.h"

bool AVMutexLocker::m_verbose = false;

///////////////////////////////////////////////////////////////////////////////

AVMutexLocker::AVMutexLocker(QMutex *mutex, const QString& method)
        : m_mutex(mutex), m_method(method)
{
    if (mutex != 0) {
        static const QString lock = " Lock";
        AVSysProfScope activity(method + lock);
        if (m_verbose) {
            AVLogDebug << "AVMutexLocker::AVMutexLocker: obtaining lock in " << method;
        }
        mutex->lock();
        if (m_verbose) {
            AVLogDebug << "AVMutexLocker::AVMutexLocker: obtained lock in " << method;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

AVMutexLocker::~AVMutexLocker()
{
    if (m_mutex != 0) {
        static const QString unlock = " Unlock";
        AVSysProfScope activity(m_method + unlock);
        if (m_verbose) {
            AVLogDebug << "AVMutexLocker::AVMutexLocker: releasing lock in " << m_method;
        }
        m_mutex->unlock();
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVMutexLocker::setVerbose(bool verbose)
{
    m_verbose = verbose;
}

// End of file
