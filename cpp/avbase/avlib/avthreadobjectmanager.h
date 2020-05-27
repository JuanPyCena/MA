//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2016
//
// Module:    AVLIB - AviBit Library
//
//-----------------------------------------------------------------------------

/*! \file
    \brief   AVThreadObjectManager header
 */

#ifndef AVTHREADOBJECTMANAGER_INCLUDED
#define AVTHREADOBJECTMANAGER_INCLUDED

// local includes

// Qt includes
#include <QtGlobal>
#include <QHash>
#include <QMutex>

// avlib includes
#include <avlib_export.h>

// forward declarations
class QObject;
class QThread;

//-----------------------------------------------------------------------------
//! Class for handling QObjects in different threads.
/*! This class allows to move QObjects to a thread out of a pool of threads.
 *  The QObject can than work in the event loop of the thread.
 *
 *  The threads are created dynamically when a new QObject is added,
 *  up until a maximal number of threads.
 *  If the maximal number is reached, objects are moved to an existing thread of the pool,
 *  which has the least number of objects already in there.
 *
 *  Objects can be removed from the threads, and threads will be ended, if they are not longer needed.
 *
 *
*/
class AVLIB_EXPORT AVThreadObjectManager
{
    //! friend declaration for function level test case
    friend class TstAVThreadObjectManager;

public:
    //! ATTENTION: maximal_number must be larger than 0, otherwise assert
    AVThreadObjectManager(uint maximal_number);
    virtual ~AVThreadObjectManager();

    //! Moves the object to one of the available threads or creates a new one for it
    //! Warning: takes ownership. Object will be deleted on destruction
    //! This function is thread-safe
    void moveToAvailableThread(QObject* object);

    //! Calls deleteLater on the object, and removes the thread if it is not longer used
    //! This function is thread-safe
    void deleteObjectLater(QObject* object);

private:
    Q_DISABLE_COPY(AVThreadObjectManager);

    QMutex m_mutex;

    uint m_maximal_number_of_threads;

    QHash<QThread*, uint> m_active_threads;   //!< value: number of objects in each thread
    QHash<QObject*, QThread*> m_thread_for_object;

};

#endif // AVTHREADOBJECTMANAGER_INCLUDED

// End of file
