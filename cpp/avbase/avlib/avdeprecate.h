///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - AviBit Library
//
/////////////////////////////////////////////////////////////////////////////


/*!  \file
 *   \author  Christian Muschick, c.muschick@avibit.com
 *   \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
 *   \brief
 */

#if !defined AVDEPRECATE_H_INCLUDED
#define AVDEPRECATE_H_INCLUDED

#include <QMap>
#include <QTimer>

#include "avlib_export.h"
#include "avexplicitsingleton.h"

#define AVDEPRECATE(s) if(AVDeprecatedList::isSingletonInitialized()) AVDeprecatedList::singleton().addDeprecatedEntry(#s, __LINE__, __FILE__)

///////////////////////////////////////////////////////////////////////////////
/**
 *  Provides an easy way to warn about deprecated identifiers used in programs.
 *
 *  Use the macro AVDEPRECATE with the deprecated identifier.
 *  After program termination, a list of all used deprecated identifiers will be printed/logged.
 */
class AVLIB_EXPORT AVDeprecatedList : public QObject, public AVExplicitSingleton<AVDeprecatedList>
{
    Q_OBJECT;
public:
    static AVDeprecatedList& initializeSingleton();
    ~AVDeprecatedList() override;

    const char *className() const { return metaObject()->className(); }

    void addDeprecatedEntry(const char *name, uint line, const QString& file);
    
    //! We want to initialize the list as early as possible; however the timer requires the QApplication to be present -
    //! delay starting the timer.
    //!
    //! TODO CM review deprecation mechanism:
    //! - timer mechanism is flaky
    //! - distinguish between soft and hard deprecation? "Hard" has easy alternative available and should be immediately
    //!   removed, "soft" involves design changes or more elaborate work to avoid. One could be info, the other error.
    void startTimer();

public slots:

    //! \param print_as_warnings Why is this needed? Because printing a warning at program
    //!                          termination time will upset automated tests and they are reported
    //!                          as failed.
    void slotPrintEntries(bool print_as_warnings = false) const;

private:
    AVDeprecatedList();

    class DeprecatedEntry
    {
    public:
        DeprecatedEntry() : m_line(0) {}
        DeprecatedEntry(uint line, const QString& file) : m_line(line), m_file(file) {}
        uint        m_line;
        QString     m_file;
    };

    typedef QMap<const char*, DeprecatedEntry> EntryContainer;
    EntryContainer m_deprecated_entry;

    //! Used to serialize access to our list of deprecated entries.
    mutable QMutex m_mutex;

    //! Regularly print out deprecation warnings in case a process goes operational with deprecated
    //! stuff inside. Printing them only when a process is terminated might get ignored more easily.
    //! There also might be the chance that a deprecated code path is taken only after the process
    //! has been running for a while.
    QTimer m_printout_timer;
    //! Do it every day
    enum { PRINTOUT_INTERVAL = 24*60*60*1000 };
};

#endif

// End of file
