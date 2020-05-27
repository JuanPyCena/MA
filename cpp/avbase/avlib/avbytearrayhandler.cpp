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



#include "stdlib.h"
#include "string.h"

#include "avlog.h"

#include "avbytearrayhandler.h"
#include <QString>

///////////////////////////////////////////////////////////////////////////////
void AVByteArrayHandler::appendArray(QByteArray& dest, const QByteArray& source)
{
    dest += source;
}

///////////////////////////////////////////////////////////////////////////////
void AVByteArrayHandler::copyArray(QByteArray& dest, unsigned int dest_pos, const QByteArray& source,
                                       unsigned int source_pos, unsigned int length)
{    
    unsigned int bytes_to_copy = dest_pos+length;

    if (bytes_to_copy > 0) {
        QByteArray result;
        result.resize(bytes_to_copy);
        AVASSERT(memcpy(result.data(), dest.data(), dest_pos));
        AVASSERT(memcpy(result.data()+dest_pos, source.data()+source_pos, length));
        dest = result;
    } else {
        dest.resize(0);
    }
}

///////////////////////////////////////////////////////////////////////////////
void AVByteArrayHandler::clipArray(QByteArray& dest, int pre_clip, int post_clip)
{
    unsigned int bytes_to_copy = 0;
    if(dest.count() >= pre_clip + post_clip)
        bytes_to_copy = dest.count() - pre_clip - post_clip;
    else
        AVLogError << "AVByteArrayHandler::clipArray: pre_clip (" << pre_clip << ") and "
                      "post_clip (" << post_clip << ") greater than target size (" << dest.count() << ")!";

    if (bytes_to_copy > 0) {
        QByteArray result;
        result.resize(bytes_to_copy);
        AVASSERT(memcpy(result.data(), dest.data()+pre_clip, bytes_to_copy));
        dest = result;
    } else {
        dest.resize(0);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVByteArrayHandler::convertQStringToArray(QByteArray& dest, const QString& source)
{
    dest = source.toLatin1();
}

// End of file
