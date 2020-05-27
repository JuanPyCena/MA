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
    \author  DI Alexander Wemmer, a.wemmer@avibit.com
    \author  QT4-PORT: Karlheinz Wohlmuth, k.wohlmuth@avibit.com
    \brief   
*/

#ifndef __AVBYTEARRAYHANDLER_H__
#define __AVBYTEARRAYHANDLER_H__

// Qt includes
#include <QString> 

// AVLib includes
#include "avmacros.h"
#include "avlib_export.h"
/////////////////////////////////////////////////////////////////////////////

//! Provides some functions to handle QByteArrays more efficient
class AVLIB_EXPORT AVByteArrayHandler
{
public:

    //! Append the source array to the dest array
    static void appendArray(QByteArray& dest, const QByteArray& source);
    
    //! Copies a segment of the source to dest array
    /*! \param dest The destination array
     *  \param dest_pos The position inside the dest to start the paste
     *  \param source The source array
     *  \param source_pos Position inside the source array to start the copy
     *  \param length The count of bytes to copy, beginning at source_pos
     */
    static void copyArray(QByteArray& dest, unsigned int dest_pos,
                          const QByteArray& source, unsigned int source_pos,
                          unsigned int length);

    //! Clips parts from the head and from the tail part of the array
    /*! \param dest The array to clip
     *  \param pre_clip Number of bytes to remove from the head of the array
     *  \param post_clip Number of bytes to remove from the tail of the array
     */
    static void clipArray(QByteArray& dest, int pre_clip,
                          int post_clip);


    //! Converst a QString to a QByteArray and strip the trailing \0
    static void convertQStringToArray(QByteArray& dest, const QString& source);

private:
    AVDISABLECOPY(AVByteArrayHandler);

    //! private ctor, only static member functions available
    AVByteArrayHandler();
};

#endif /* __AVBYTEARRAYHANDLER_H__ */

// End of file

