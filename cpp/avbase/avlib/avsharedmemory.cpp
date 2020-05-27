//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2015
//
// Module:    AVLIB - Avibit Library
//
//-----------------------------------------------------------------------------

/*! \file
    \author  Ralf Gamillscheg, r.gamillscheg@avibit.com
    \author  QT5-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   AVSharedMemory implementation
 */

// local includes
#include "avsharedmemory.h"

#if defined(_OS_WIN32_) || defined(Q_OS_WIN32)
#include "avsharedmemory_private_windows.h"
#else
#include "avsharedmemory_private_unix.h"
#endif
// Qt includes


// avlib includes


//-----------------------------------------------------------------------------

AVSharedMemory::AVSharedMemory(const QString& segment_id, int create_size,
               const QString& override_application_name) :
        m_private(new AVSharedMemoryPrivate(segment_id, create_size, override_application_name))
{
}

//-----------------------------------------------------------------------------

AVSharedMemory::~AVSharedMemory()
{
}

//-----------------------------------------------------------------------------

bool AVSharedMemory::lock()
{
    return m_private->lock();
}

//-----------------------------------------------------------------------------

bool AVSharedMemory::unlock()
{
    return m_private->unlock();
}

//-----------------------------------------------------------------------------

char* AVSharedMemory::getSegment() const
{
    return m_private->getSegment();
}

//-----------------------------------------------------------------------------

int AVSharedMemory::getSize() const
{
    return m_private->getSize();
}

//-----------------------------------------------------------------------------

bool AVSharedMemory::isInitialized() const
{
    return m_private->isInitialized();
}

//-----------------------------------------------------------------------------

void AVSharedMemory::setInitialized()
{
    m_private->setInitialized();
}

//-----------------------------------------------------------------------------

void AVSharedMemory::detachWithoutCleanup()
{
    m_private->detachWithoutCleanup();
    m_private.reset(0);
}

// End of file
