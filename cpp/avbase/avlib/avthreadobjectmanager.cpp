//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2016
//
// Module:    AVLIB - AviBit Library
//
//-----------------------------------------------------------------------------

/*! \file
    \brief   AVThreadObjectManager implementation
 */

// local includes
#include "avthreadobjectmanager.h"

// Qt includes
#include <QObject>
#include <QThread>

// avlib includes
#include "avlog.h"

//-----------------------------------------------------------------------------

AVThreadObjectManager::AVThreadObjectManager(uint maximal_number) :
    m_mutex(QMutex::Recursive), // recursion is used in destructor
    m_maximal_number_of_threads(maximal_number)
{
    AVASSERT(m_maximal_number_of_threads > 0); // CS-90
}

//-----------------------------------------------------------------------------

AVThreadObjectManager::~AVThreadObjectManager()
{
    QMutexLocker locker(&m_mutex);

    for(auto iter = m_thread_for_object.begin(); iter != m_thread_for_object.end();)
    {
        QObject* object = iter.key();
        ++iter;
        deleteObjectLater(object);
    }
    AVASSERT(m_active_threads.size() == 0);   //CS-93
    AVASSERT(m_thread_for_object.size() == 0);//CS-93
}

//-----------------------------------------------------------------------------

void AVThreadObjectManager::moveToAvailableThread(QObject* object)
{
    QMutexLocker locker(&m_mutex);

    if(object == nullptr)
    {
        AVLogError << "AVThreadObjectManager::moveToAvailableThread: given nullptr - bailing out";
        return;
    }

    if(m_thread_for_object.contains(object))
    {
        AVLogError << "AVThreadObjectManager::moveToAvailableThread: object already managed - bailing out";
        return;
    }

    QThread* thread = 0;

    if(m_active_threads.size() < static_cast<int>(m_maximal_number_of_threads))
    {
        AVLogInfo << "AVThreadObjectManager::moveToAvailableThread: creating new thread";
        thread = new QThread();
        thread->setObjectName(QString("AVThreadObjectManager 0x%1").arg((quintptr)thread, QT_POINTER_SIZE*2, 16, QChar('0')));
        thread->start();
        m_active_threads[thread] = 0;
    }
    else
    {
        uint smallest_number = 0;
        //find thread with smallest number of objects
        for(auto iter = m_active_threads.begin(); iter != m_active_threads.end(); ++iter)
        {
            if(iter == m_active_threads.begin() || iter.value() < smallest_number)
            {
                thread = iter.key();
                smallest_number = iter.value();
            }
        }
    }
    AVASSERT(thread != 0); // CS-93

    object->moveToThread(thread);

    ++m_active_threads[thread]; // increase number of objects in thread
    m_thread_for_object[object] = thread;
}

//-----------------------------------------------------------------------------

void AVThreadObjectManager::deleteObjectLater(QObject* object)
{
    QMutexLocker locker(&m_mutex);

    if(object == nullptr)
    {
        if(AVLogger != 0) // this function may be called after logger has been destructed
        {
            AVLogError << "AVThreadObjectManager::deleteObjectLater: given nullptr - bailing out";
        }
        return;
    }

    if(!m_thread_for_object.contains(object))
    {
        if(AVLogger != 0) // this function may be called after logger has been destructed
        {
            AVLogError << "AVThreadObjectManager::deleteObjectLater: object not managed - bailing out";
        }
        return;
    }

    QThread* thread = m_thread_for_object[object];

    object->deleteLater(); // manually tested: destructor will be called in thread

    AVASSERT(m_active_threads[thread] > 0); // CS-93
    --m_active_threads[thread]; // decrease number of objects in thread

    m_thread_for_object.remove(object);

    if(m_active_threads[thread] == 0)
    {
        // remove thread
        // Note: according to Qt docu, it is guaranteed,
        // that objects for which deleteLater has been called, are deleted before end
        thread->quit();
        if(!thread->wait(5000))
        {
            if(AVLogger != 0) // this function may be called after logger has been destructed
            {
                AVLogError << "AVThreadObjectManager::deleteObjectLater: "<<
                          " could not quit thread "<<thread;
            }
        }
        m_active_threads.remove(thread);
        delete thread;
        if(AVLogger != 0) // this function may be called after logger has been destructed
        {
            AVLogInfo << "AVThreadObjectManager::moveToAvailableThread: deleted unused thread";
        }
    }
}

//-----------------------------------------------------------------------------



// End of file
