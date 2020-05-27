///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - AviBit Library
//
/////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Dietmar Gösseringer, d.goesseringer@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   provides functions to convert QRect, QPoint, etc to QString
*/



// local includes
#include "avuniqueidgenerator.h"


///////////////////////////////////////////////////////////////////////////////

QString AVUniqueIdGenerator::acquireId(const QString& info, bool create_datetime_id)
{
    // Create trackId datetime segment
    QDateTime now = quantize(AVTimeReference::isSingletonInitialized() ? AVTimeReference::currentDateTime() :
            AVDateTime::currentDateTimeUtc());
    if (now == m_last_id_creation)
    {
        m_counter++;
    }
    else
    {
        m_counter = 1;
        m_last_id_creation = now;
    }

    // Create/Acquire trackNum
    bool unique_number_ok = false;
    QString unique_number = m_unique_number_generator.acquireNumberAsFixedWidthString(
        m_unique_number_postfix_width, unique_number_ok);

    if (!unique_number_ok)
    {
        AVLogger->Write(LOG_ERROR,
            "AVUniqueIdGenerator(%p):acquireId: "
            "FAILED: Could not get a unique number "
            "(%d acquired, %d free)",
            this,
            m_unique_number_generator.getNAcquiredNumbers(),
            m_unique_number_generator.getNFreeNumbers());
        return QString::null;
    }

    // Compose trackId
    QString id, tmp;
    AVsprintf(id, "%s%s", qPrintable(m_prefix), qPrintable(m_seperator));

    if (! info.isEmpty() )
    {
        AVsprintf(tmp, "%s%s", qPrintable(info), qPrintable(m_seperator));
        id += tmp;
    }

    if (create_datetime_id)
    {
        AVsprintf(tmp, "%s%s%03d%s",
            qPrintable(now.toString(QString("yyyyMMddhhmmss"))), qPrintable(m_seperator),
            m_counter, qPrintable(m_seperator));
        id += tmp;
    }

    id += unique_number;

    // Check trackId
    if (!(containsId(id)))
    {
        addId(id);
        return id;
    }
    else
    {
        AVLogger->Write(LOG_ERROR,
            "AVUniqueIdGenerator(%p):acquireId: "
            "FAILED: ID %s already exists "
            "(%d IDs issued, %d free / %d unique numbers issued, %d free)",
            this,
            qPrintable(id),
            getNAcquiredIds(),
            getNFreeIds(),
            m_unique_number_generator.getNAcquiredNumbers(),
            m_unique_number_generator.getNFreeNumbers());
        return QString::null;
    }
}

// End of file
