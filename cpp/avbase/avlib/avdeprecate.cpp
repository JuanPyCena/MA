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
 *  \author  Christian Muschick, c.muschick@avibit.com
 *  \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
 *  \brief
 */


#include "avlog.h"


#include "avdeprecate.h"

///////////////////////////////////////////////////////////////////////////////

AVDeprecatedList& AVDeprecatedList::initializeSingleton()
{
    return setSingleton(new (LOG_HERE) AVDeprecatedList());
}

///////////////////////////////////////////////////////////////////////////////

AVDeprecatedList::~AVDeprecatedList()
{
}

///////////////////////////////////////////////////////////////////////////////

void AVDeprecatedList::addDeprecatedEntry(const char *name, uint line, const QString& file)
{
    QMutexLocker locker(&m_mutex);

    EntryContainer::iterator it = m_deprecated_entry.find(name);
    if (it != m_deprecated_entry.end()) return;
    m_deprecated_entry.insert(name, DeprecatedEntry(line, file));
}

///////////////////////////////////////////////////////////////////////////////

void AVDeprecatedList::startTimer()
{
    AVDIRECTCONNECT(&m_printout_timer, SIGNAL(timeout()),
              this, SLOT(slotPrintEntries()));
    m_printout_timer.setSingleShot(false);        
    m_printout_timer.start(PRINTOUT_INTERVAL);
}

///////////////////////////////////////////////////////////////////////////////

void AVDeprecatedList::slotPrintEntries(bool print_as_warnings) const
{
    QMutexLocker locker(&m_mutex);

    if (m_deprecated_entry.isEmpty()) return;

    QStringList log;
    log << "------------------------------------------------------------------------";
    log << "The following facilities used by this program are deprecated:";
    for (EntryContainer::const_iterator it = m_deprecated_entry.begin();
            it != m_deprecated_entry.end();
            ++it)
    {
        QString line("%1:%2:\t%3");
        log << line.arg(it.value().m_file).arg(it.value().m_line).arg(it.key());
    }
    log << "------------------------------------------------------------------------";

    if (print_as_warnings)
    {
        for (int i=0; i<log.size(); ++i) AVLogWarning << log[i];
    } else
    {
        for (int i=0; i<log.size(); ++i) AVLogInfo    << log[i];
    }
}

///////////////////////////////////////////////////////////////////////////////

AVDeprecatedList::AVDeprecatedList()
{
}

// End of file
