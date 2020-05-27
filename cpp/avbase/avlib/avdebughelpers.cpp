///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT5
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - AviBit Library
//
/////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Markus Dauschan, m.dauschan@avibit.com
    \brief   Several helper functions for debugging.
*/

#include <iostream>
using std::cout;
using std::cerr;

// include this first for OS preprocessor defines...
#include <QtGlobal>

#if defined Q_OS_WIN32
#include <windows.h>
#include <DbgHelp.h>
#endif

#ifdef Q_OS_LINUX
#include <unistd.h>
#include <sys/wait.h>
#include <cstdio>
#include <iostream>
#endif

#include "avdebughelpers.h"

#include "avlog.h"

//-----------------------------------------------------------------------------

void AVPrintStackTraceUsingGdb()
{
#ifndef Q_OS_LINUX
    AVLogError << "printStackTrace: Currently only implemented for Linux!";
#else

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
    
    char pid_buffer[30];
    sprintf(pid_buffer, "--pid=%d", getpid());
    char name_buffer[512];
    name_buffer[readlink("/proc/self/exe", name_buffer, 511)]=0;

    int read_pipe[2];
    pipe(read_pipe);

    int child_pid = fork();

    if(child_pid == -1)
    {
        AVLogFatal << "Fork failed!";
    }

    if (!child_pid)
    {
        close(read_pipe[0]);
        dup2(read_pipe[1], STDOUT_FILENO);
        close(read_pipe[1]);

        cout << "Starting GDB for " << name_buffer << " pid=" << pid_buffer;
        execlp("gdb", "gdb", "--batch", "-n", "-ex", "thread", "-ex", "bt", name_buffer, pid_buffer, NULL);
        cout << "printStackTrace: GDB failed to start!";
        cout.flush();
        exit(0);
    }
    else
    {
        close(read_pipe[1]);

        char buff[512];

        struct timeval tv;
        fd_set readfds;
        tv.tv_sec = 10;
        tv.tv_usec = 0;
        FD_ZERO(&readfds);
        FD_SET(read_pipe[0], &readfds);
        select(read_pipe[0] + 1, &readfds, NULL, NULL, &tv);

        if(FD_ISSET(read_pipe[0], &readfds))
        {
            waitpid(child_pid,NULL,0);

            ssize_t read_count;
            QString output;

            while((read_count = read(read_pipe[0], buff, 511)) > 0)
            {
                buff[read_count] = '\0';
                output += buff;
            }

            AVLogInfo << "--- STACK TRACE ---\n" << output;

            close(read_pipe[0]);
        }
    }
#pragma GCC diagnostic pop
#endif
}

//-----------------------------------------------------------------------------

#if defined Q_OS_WIN32
void AVWriteMiniDump(_EXCEPTION_POINTERS *exception_pointers)
{
    // This will be called after a crash, so make as little assumption about state as possible.
    // In particular, don't use any AviBit stuff such as logger etc.

    const uint BUF_SIZE=256;

    // First check whether minidump is allowed
    char enabled[BUF_SIZE];
    if (0 != GetEnvironmentVariable("AVBASE_ENABLE_MINIDUMP", enabled, BUF_SIZE))
    {
        if (!strcmp(enabled, "0") || !strcmp(enabled, "false"))
        {
            cerr <<  "Core dump inhibited...\n";
            return;
        }
    }

    // Write to cerr because it is auto-flushed.
    cerr <<  "Dumping core...\n";

    // Create "cores" subdir if not present. Don't bother to check for existence before creation because
    // of race condition.
    CreateDirectory("cores", NULL);
    DWORD attributes = GetFileAttributes("cores");
    if (attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_DIRECTORY))
    {
        cerr << "Failed to create 'cores' directory. Last error is " << GetLastError() << "\n";
        return;
    }
    
    // name the dump like our executable, and write it to the current directory.
    TCHAR exe_full_path[MAX_PATH + 1];
    GetModuleFileName(NULL, exe_full_path, MAX_PATH + 1);
    TCHAR* process_name = exe_full_path;
    for (TCHAR* cur_pos = process_name; *cur_pos != 0; ++cur_pos)
    {
        // file name starts after last backslash (or slash? probably always backslash)
        if (*cur_pos == '\\' || *cur_pos == '/') process_name = cur_pos+1;
    }
    // remove exe suffix
    TCHAR* exe_str = strstr(process_name, ".exe");
    if (exe_str) *exe_str = 0;

    TCHAR temp_filename[BUF_SIZE];
    TCHAR final_filename[BUF_SIZE];
    snprintf(temp_filename, BUF_SIZE, "cores\\%s.%u.dmp.tmp", process_name, GetCurrentProcessId());
    snprintf(final_filename, BUF_SIZE, "cores\\%s.%u.dmp", process_name, GetCurrentProcessId());
    
    HANDLE hFile = CreateFile( temp_filename, GENERIC_WRITE,
                               0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
    if (hFile == INVALID_HANDLE_VALUE)
    {
        cerr << "failed to open dump file " << temp_filename << " for writing.\n";
        return;
    }
    
    MINIDUMP_EXCEPTION_INFORMATION mdei;
    mdei.ThreadId           = GetCurrentThreadId();
    mdei.ExceptionPointers  = exception_pointers;
    mdei.ClientPointers     = FALSE;

    // Select minidump content. This might require some tweaking, current settings
    // based on SAM experiences (patch from SWE-3868).
    MINIDUMP_TYPE dump_type = static_cast<MINIDUMP_TYPE>(MiniDumpWithDataSegs               |
                                                         MiniDumpWithHandleData             |
                                                         MiniDumpWithFullMemoryInfo         |
                                                         MiniDumpWithThreadInfo             |
                                                         MiniDumpWithUnloadedModules        |
                                                         MiniDumpWithPrivateReadWriteMemory);

    if (FALSE == MiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(),
                                    hFile, dump_type,
                                    (exception_pointers != nullptr) ? &mdei : NULL,
                                    NULL, /* no user stream param */
                                    NULL /* no callback */ ))
    {
        cerr << "Failed to write the minidump. Last error is " << GetLastError() << "\n";
    }

    if (0 == CloseHandle( hFile ))
    {
        cerr << "Failed to close minidump file. Last error is " << GetLastError() << "\n";
    }

    if (0 == MoveFile(temp_filename, final_filename))
    {
        cerr << "Failed to move " << temp_filename << " to its final location.\n";
    }
}
#endif // #if defined Q_OS_WIN32

//-----------------------------------------------------------------------------


// End of file
