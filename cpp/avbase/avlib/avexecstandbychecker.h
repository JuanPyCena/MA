///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2013
// QT-Version: QT5
//
// Module:    AVLIB - AviBit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Ralf Gamillscheg, r.gamillscheg@avibit.com
    \author  QT5-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   AVExecStandbyChecker header
 */

#ifndef AVEXECSTANDBYCHECKER_INCLUDED
#define AVEXECSTANDBYCHECKER_INCLUDED

// Qt includes
#include <QObject>

// avlib includes
#include "avlib_export.h"
#include "avmacros.h"

// local includes
#include "avexplicitsingleton.h"
#include "avprocessstatepoller.h"

// forward declarations
class AVExecStandbyCheckerUnitTestDriver;

///////////////////////////////////////////////////////////////////////////////
//! This singleton class allows to get information about the current exec standby state
/*!
 *  It is initialized by the AviBit process initialization, if the according process parameter
 *  is set. In this case a configuration needs to be present.
 *  If the the configuration does not contain the polling process name,
 *  the class behaves as EXEC.
 *
 *  ATTENTION: This should be instantiated only after the connection to the Time Reference Server is established,
 *  in order not to miss the signal. In AVApplicationInit this behavior is made sure.
 *
 *  NOTE: This singleton class supports unit testing by providing a static singleton initializer
 *        for coupling the singleton to a test driver instead of a process poller that periodically
 *  queries the state of a configured process. The static initializer for testing is
 *  initializeSingleton(const QObject *).
*/
class AVLIB_EXPORT AVExecStandbyChecker : public QObject, public AVExplicitSingleton<AVExecStandbyChecker>
{
    Q_OBJECT
    AVDISABLECOPY(AVExecStandbyChecker);

    //! friend declaration for function level test case
    friend class TstAVExecStandbyChecker;

public:

#define SERVER_STATE_TYPE(enum_value) \
    enum_value(StandBy     , =  0) \
    enum_value(Exec        , =  1) \
    enum_value(Negotiate   , =  2) \
    enum_value(PendingExec , =  3) \
    enum_value(Unknown     , = -1)

    DEFINE_ENUM(ServerState, SERVER_STATE_TYPE)

public:
    AVExecStandbyChecker();
    ~AVExecStandbyChecker() override;

    static AVExecStandbyChecker& initializeSingleton();

    //! Constructor to be used in unit tests
    explicit AVExecStandbyChecker(const QObject& driver);

    /**
     * @brief Initializes the AVExecStandyChecker with a test driver in state Unknown.
     *
     * The test driver implements the signalShmVarChanged signal. The AVExecStandyChecker is bound
     * to the signalShmVarChanged signal. The test driver can then emit this signal to change the
     * state of this singleton. As the AVExecStandbyChecker starts in state Unknown, the test
     * driver needs to manually emit a signal that brings it into state exec or standby.
     *
     * NOTE: This initializer is for use with unit tests only. Do not use it in a production
     * setting.
     *
     * WARNING: Consider singleton ownership before using this initializer function! For example,
     * when using this initializer with a DAQ, the AVDaqBase class owns the AVExecStandbyChecker.
     * As a consequence the AVDaqBase class initializes and destroys the AVExecStandbyChecker
     * singleton in its constructor and destructor. The AVDaqBase class constructor checks if
     * AVExecStandbyChecker is already initialized, and only initializes the singleton if it is not
     * initialized. The problem is the destructor. The upshot is, if a unit test initializes the
     * AVExecStandbyChecker and calls a DAQ contructor, the DAQ destructor will destroy the
     * singleton.
     *
     * @return the singleton instance of the AVExecStandyChecker
     */
    static AVExecStandbyChecker& initializeSingleton(const AVExecStandbyCheckerUnitTestDriver& driver);

    //! Deprecated, use AVExecStandbyCheckerUnitTestDriver instead.
    //! @param driver the test driver implementing the signalShmVarChanged signal
    static AVExecStandbyChecker& initializeSingleton(const QObject *driver);

    bool isExec() const;
    bool isStandby() const;

    virtual const char *className() const { return "AVExecStandbyChecker"; }

signals:
    void signalSwitchedToExec();

    //! For all practical purposes, this is emitted when switching to standby mode (either from exec, or via initial
    //! negotiation).
    /**
     *  Changes to transient states are suppressed, except if we go away from exec state.
     *
     *  TODO CM: Nomenclature of this signal is correct but not intuitive.
     *  Clearly define possible state transitions (is it possible to go into states Negotiate or PendingExec
     *  after we were in Exec or StandBy? Probably not, but there is no guarantee at the moment).
     *  Clients of this class probably are interested in states Exec/StandBy only, and not in transitional states.
     */
    void signalSwitchedToNotExec();

    /*! This signal is emitted before signalSwitchedToExec()
     *  ATTENTION: it should only be used by AVTwinConnection2!
     *  It makes sure that AVTwinConnection2 is updated before other parts of the program
     */
    void signalAboutToSwitchToExec();

    //! This signal is emitted before signalSwitchedToNotExec(), it is used by the AVTwinConnection2
    void signalAboutToSwitchToNotExec();


private slots:
    void slotInternalShmVarChanged(AVProcessStatePoller::PollingEvent type,
                                   const AVCVariant& new_value);


private:
    //! non-owning; nullptr in operational setups
    const QObject *m_unit_test_driver;

    //! nullptr if unit test driver is used
    std::unique_ptr<AVProcessStatePoller> m_process_state_poller;

    ServerState m_current_state;
};

#endif // AVEXECSTANDBYCHECKER_INCLUDED

// End of file
