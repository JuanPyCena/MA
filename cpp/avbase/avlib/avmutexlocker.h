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
    \brief    This file declares the class AVMutexLocker
*/

#ifndef AVMUTEXLOCKER_H
#define AVMUTEXLOCKER_H

// Local includes
#include "avlib_export.h"

// Qt includes
#include <qmutex.h>
#include <qstring.h>

///////////////////////////////////////////////////////////////////////////////

//! AVMutexLocker works very similar to QMutexLocker. It adds optional debug
//! output to trace mutex usage.

class AVLIB_EXPORT AVMutexLocker
{
public:
    //! Constructor
    /*! \param mutex  the mutex to lock until the instance is destroyed
        \param method the name of the method where the instance is created
    */
    AVMutexLocker(QMutex *mutex, const QString& method);
    //! Destructor
    virtual ~AVMutexLocker();
    static void setVerbose(bool verbose);
private:
    //! Copy-constructor: defined but not implemented
    AVMutexLocker(const AVMutexLocker& rhs);
    //! Assignment operator: defined but not implemented
    AVMutexLocker& operator=(AVMutexLocker& rhs);
private:
    static bool m_verbose;  //!< indicates whether to output debug information
private:
    QMutex *m_mutex;    //!< the mutex that is locked by this instance
    QString m_method;   //!< the name of the method that created this instance
};

#endif // AVMUTEXLOCKER_H

// End of file
