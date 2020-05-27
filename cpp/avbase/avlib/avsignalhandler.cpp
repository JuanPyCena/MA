///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:     AVLIB - AviBit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Daniela Kengyel, d.kengyel@avibit.com
    \brief
 */

#include <iostream>

// qt includes
#include <qglobal.h>

// system includes

#if defined(Q_OS_LINUX)
#include <unistd.h>
#include <sys/socket.h>
#endif

#include <atomic>
#include <csignal>
#include <iostream>
#include <memory>
#include <signal.h>
#include <sys/types.h>

//local includes
#include "avsignalhandler.h"
#include "avdaemon.h"
#include "avenvironment.h"
#include "avdebughelpers.h"

/////////////////////////////////////////////////////////////////////////////

#if defined(Q_OS_LINUX)
int AVSignalHandler::m_socket_pair[2];
#endif

///////////////////////////////////////////////////////////////////////////////

// windows exception handler stuff
#if defined Q_OS_WIN32

//! Helper function to handle invalid params via unhandled exceptions
void invalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t reserved)
{
    Q_UNUSED(reserved);

    std::cerr << "invalidParameterHandler, raising Exception...\n";
    std::cerr << "expression " << expression << "\n";
    std::cerr << "function " << function << "\n";
    std::cerr << "file " << file << "\n";
    std::cerr << "line " << line << "\n";
    RaiseException(0,0,0,NULL);
}

//! Helper function to handle abort() calls via unhandled exceptions
void abortHandler(int signal)
{
    std::cerr <<  "abortHandler, raising Exception...\n";
    RaiseException(0,0,0,NULL);
}

//! Helper function for PreventSetUnhandledExceptionFilter
LPTOP_LEVEL_EXCEPTION_FILTER WINAPI DummySetUnhandledExceptionFilter(
        LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
    return NULL;
}

/**
 * @brief PreventSetUnhandledExceptionFilter works around some cases in which setting the filter for unhandled exceptions does not work correctly.
 *
 * See http://blog.kalmbachnet.de/?postid=75 or http://blog.kalmbach-software.de/2008/04/02/:
 * Many programs are setting an own Unhandled-Exception-Filter, for catching unhandled exceptions and do some reporting or logging
 * (for example creating a mini-dump ).
 * Now, starting with VC8 (VS2005), MS changed the behaviour of the CRT is some security related and special situations.
 * The CRT forces the call of the default-debugger (normally Dr.Watson) without informing the registered unhandled exception filter.
 * The situations in which this happens are the following:
 * ...
 */
BOOL PreventSetUnhandledExceptionFilter()
{
    HMODULE hKernel32 = LoadLibrary("kernel32.dll");
    if (hKernel32 == NULL) return FALSE;

    void *pOrgEntry = GetProcAddress(hKernel32, "SetUnhandledExceptionFilter");
    if(pOrgEntry == NULL) return FALSE;

    unsigned char newJump[ 100 ];
    DWORD dwOrgEntryAddr = (DWORD) pOrgEntry;
    dwOrgEntryAddr += 5; // add 5 for 5 op-codes for jmp far
    void *pNewFunc = &DummySetUnhandledExceptionFilter;
    DWORD dwNewEntryAddr = (DWORD) pNewFunc;
    DWORD dwRelativeAddr = dwNewEntryAddr - dwOrgEntryAddr;

    newJump[ 0 ] = 0xE9;  // JMP absolute
    memcpy(&newJump[ 1 ], &dwRelativeAddr, sizeof(pNewFunc));

    SIZE_T bytesWritten;
    BOOL bRet = WriteProcessMemory(GetCurrentProcess(),
                                   pOrgEntry, newJump, sizeof(pNewFunc) + 1, &bytesWritten);
    return bRet;
}

///////////////////////////////////////////////////////////////////////////////

static LONG WINAPI AVUnhandledExceptionFilter(_EXCEPTION_POINTERS *ei)
{
    // On some platforms or with some settings, endless recursion of unhandled exception filters can apparently occur
    // (see SWE-5452). Until the problem is better understood, ensure that this handler is called only once.
    // Perhaps this is also related to PreventSetUnhandledExceptionFilter, which modifies the
    // SetUnhandledExceptionFilter implementation?
    {
        static std::atomic<bool> already_executed = false;
        bool f=false, t=true;
        if (!already_executed.compare_exchange_strong(f, t))
        {
            return EXCEPTION_EXECUTE_HANDLER;
        }
    }

    char *exn = nullptr;
    switch (ei->ExceptionRecord->ExceptionCode)
    {
    case EXCEPTION_ACCESS_VIOLATION:
        exn = "EXCEPTION_ACCESS_VIOLATION";      break;
    case EXCEPTION_DATATYPE_MISALIGNMENT:
        exn = "EXCEPTION_DATATYPE_MISALIGNMENT"; break;
    case EXCEPTION_BREAKPOINT:
        exn = "EXCEPTION_BREAKPOINT";            break;
    case EXCEPTION_SINGLE_STEP:
        exn = "EXCEPTION_SINGLE_STEP";           break;
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
        exn = "EXCEPTION_ARRAY_BOUNDS_EXCEEDED"; break;
    case EXCEPTION_FLT_DENORMAL_OPERAND:
        exn = "EXCEPTION_FLT_DENORMAL_OPERAND";  break;
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
        exn = "EXCEPTION_FLT_DIVIDE_BY_ZERO";    break;
    case EXCEPTION_FLT_INEXACT_RESULT:
        exn = "EXCEPTION_FLT_INEXACT_RESULT";    break;
    case EXCEPTION_FLT_INVALID_OPERATION:
        exn = "EXCEPTION_FLT_INVALID_OPERATION"; break;
    case EXCEPTION_FLT_OVERFLOW:
        exn = "EXCEPTION_FLT_OVERFLOW";          break;
    case EXCEPTION_FLT_STACK_CHECK:
        exn = "EXCEPTION_FLT_STACK_CHECK";       break;
    case EXCEPTION_FLT_UNDERFLOW:
        exn = "EXCEPTION_FLT_UNDERFLOW";         break;
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
        exn = "EXCEPTION_INT_DIVIDE_BY_ZERO";    break;
    case EXCEPTION_INT_OVERFLOW:
        exn = "EXCEPTION_INT_OVERFLOW";          break;
    case EXCEPTION_PRIV_INSTRUCTION:
        exn = "EXCEPTION_PRIV_INSTRUCTION";      break;
    case EXCEPTION_IN_PAGE_ERROR:
        exn = "EXCEPTION_IN_PAGE_ERROR";         break;
    case EXCEPTION_ILLEGAL_INSTRUCTION:
        exn = "EXCEPTION_ILLEGAL_INSTRUCTION";   break;
    case EXCEPTION_NONCONTINUABLE_EXCEPTION:
        exn = "EXCEPTION_NONCONTINUABLE_EXCEPTION"; break;
    case EXCEPTION_STACK_OVERFLOW:
        exn = "EXCEPTION_STACK_OVERFLOW";        break;
    case EXCEPTION_INVALID_DISPOSITION:
        exn = "EXCEPTION_INVALID_DISPOSITION";   break;
    case EXCEPTION_GUARD_PAGE:
        exn = "EXCEPTION_GUARD_PAGE";            break;
    case EXCEPTION_INVALID_HANDLE:
        exn = "EXCEPTION_INVALID_HANDLE";        break;
    default:
        exn = "UNKNOWN"; break;
    }

    // Don't assume that anything is still working (logger etc).
    // Just write out the message and be done with it.
    // Also see SWE-4687, SWE-3816.
    fprintf(stderr, "AVUnhandledExceptionFilter: Fatal exception %s occurred at 0x%p\n", exn, ei->ExceptionRecord->ExceptionAddress);

    AVWriteMiniDump(ei);

    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

/////////////////////////////////////////////////////////////////////////////

AVSignalHandler::~AVSignalHandler()
{
    signal(SIGINT,  SIG_DFL);
    signal(SIGTERM, SIG_DFL);
}

/////////////////////////////////////////////////////////////////////////////

AVSignalHandler &AVSignalHandler::initializeSingleton()
{
    return setSingleton(new AVSignalHandler());
}

/////////////////////////////////////////////////////////////////////////////

void AVSignalHandler::installHandler()
{
    if (isHandlerDisabled())
    {
        // logger not available yet...
        return;
    }

#if defined(Q_OS_LINUX)
    ::socketpair(AF_UNIX, SOCK_STREAM, 0, m_socket_pair);

    struct sigaction term_struct;
    memset(&term_struct, 0, sizeof(term_struct));
    term_struct.sa_handler = AVSignalHandler::signalHandler;
    sigemptyset(&term_struct.sa_mask);
    term_struct.sa_flags |= SA_RESTART;

    sigaction(SIGTERM, &term_struct, 0);

    bool catch_int = isatty(0); // TODO CM why? this is ancient code...
    if (!catch_int)
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
        term_struct.sa_handler = SIG_IGN;
#pragma GCC diagnostic pop
    }
    sigaction(SIGINT, &term_struct, 0);
#else
    signal(SIGINT, AVSignalHandler::signalHandler);
    // As there is no TERM signal in windows this is not expected to do anything useful; register it anyway for the sake of completeness.
    // Also see avterminateprocess, which is used by runtime and test framework to terminate processes.
    signal(SIGTERM, AVSignalHandler::signalHandler);
#endif

    // setup error handling for win32, this avoids an error message box
    // to pop-up in case of severe errors, and triggers writing a minidump.
#if defined Q_OS_WIN32
    _set_invalid_parameter_handler(invalidParameterHandler);
    signal(SIGABRT, abortHandler);
    SetUnhandledExceptionFilter(AVUnhandledExceptionFilter);
    PreventSetUnhandledExceptionFilter();

    // RD-306 - avoid popup windows "xyz has stopped working"
    DWORD current_error_mode = SetErrorMode(0);
    SetErrorMode(current_error_mode | SEM_NOGPFAULTERRORBOX);
#endif
}

/////////////////////////////////////////////////////////////////////////////

AVSignalHandler::AVSignalHandler()
{
    installHandler();
    initialize();
}

/////////////////////////////////////////////////////////////////////////////

bool AVSignalHandler::isHandlerDisabled() const
{
    return !AVEnvironment::getEnv("EXCEPTION_HANDLER_OFF", false).isNull();
}

/////////////////////////////////////////////////////////////////////////////

void AVSignalHandler::signalHandler(int s)
{
    // don't require that the QApplication actually is started (e.g. command line tools) - set this flag right away.
    AVProcessTerminated = true;

#if defined(Q_OS_LINUX)
    ::write(m_socket_pair[0], &s, sizeof(s));
#else
    AVLogInfo << "Terminating due to CTRL-C";

    // This is called in a thread context in windows, which apparently causes trouble -> post to main thread
    AVASSERT(QMetaObject::invokeMethod(&AVSignalHandler::singleton(), "slotQuitInternal", Qt::QueuedConnection));
#endif
}

/////////////////////////////////////////////////////////////////////////////

void AVSignalHandler::initialize()
{
    AVASSERT(qApp != nullptr);

    if (isHandlerDisabled())
    {
        AVLogInfo << "signal and exception handling disabled via environment variable";
        return;
    }

#if defined(Q_OS_LINUX)
    m_socket_notifier.reset(new QSocketNotifier(m_socket_pair[1], QSocketNotifier::Read));
    AVASSERT(connect(m_socket_notifier.get(), SIGNAL(activated(int)), this, SLOT(slotLinuxSignal())));
#endif
}

/////////////////////////////////////////////////////////////////////////////

void AVSignalHandler::slotQuitInternal()
{
    AVASSERT(qApp != nullptr);
    AVProcessTerminated = true;
    qApp->quit();
}

/////////////////////////////////////////////////////////////////////////////

void AVSignalHandler::slotLinuxSignal()
{
#if defined(Q_OS_LINUX)
    m_socket_notifier->setEnabled(false);
    int sig = 0;
    ::read(m_socket_pair[1], &sig, sizeof(sig));
    AVLogInfo << "terminating due to signal " << sig;
    slotQuitInternal();
    m_socket_notifier->setEnabled(true);
#else
    AVASSERT(false);
#endif
}

// End of file
