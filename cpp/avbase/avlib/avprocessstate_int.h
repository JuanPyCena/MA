///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Thomas Neidhart, t.neidhart@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Internal definitions for avprocessstate
*/

#if !defined(AVPROCESSSTATEINT_H_INCLUDED)
#define AVPROCESSSTATEINT_H_INCLUDED

// system includes
#include <sys/types.h>
#include <time.h>
#include <sys/timeb.h>

#include "avlib_export.h"
#include "avconfig2classic.h"

///////////////////////////////////////////////////////////////////////////////
// Configuration data

class AVLIB_EXPORT AVProcessStateConfig : public AVConfig2HybridConfig
{
public:
    AVProcessStateConfig() :
        AVConfig2HybridConfig("Process State", "processstate")
    {
        m_show_help_per_default = false;

        QString help_shm_size = "size of shm (in bytes)";
        QString help_blocks   = "maximum psd blocks in shm";
        QString help_timeout  = "timeout for psd entry (in sec)";

        if (isNewConfigUsed())
        {
            AVConfig2Global::singleton().loadConfig("processstate", true);
            registerParameter("shm_size", &SHM_SIZE, help_shm_size).
                    setOptional(2000*1024).
                    setDeprecatedName("SHMSize");
            registerParameter("max_psd_blocks", &MAX_PSD_BLOCKS, help_blocks).
                    setOptional(100).
                    setMinMax(5, 1000).
                    setDeprecatedName("PSDBlocks");
            registerParameter("psd_timeout", &PSD_TIMEOUT, help_timeout).
                    setOptional(120).
                    setMin(0).
                    setDeprecatedName("Timeout");
            refreshParams();
        } else
        {
            ADDCEP  ("SHMSize", &SHM_SIZE, 2000*1024, qPrintable(help_shm_size), "size");
            ADDCEPTT("PSDBlocks", &MAX_PSD_BLOCKS, 100, qPrintable(help_blocks), "psd", 5, 1000);
            ADDCEPT ("Timeout", &PSD_TIMEOUT, 120, qPrintable(help_timeout), "timeout", 0);
            readConfig();
        }
    };

    int SHM_SIZE;                //!< size of the shm segment
    int MAX_PSD_BLOCKS;          //!< maximum number of psd blocks in shm
    int PSD_TIMEOUT;             //!< timeout of psd entry in sec
};

///////////////////////////////////////////////////////////////////////////////

const int MAX_PROCESS_NAME = 50;
const int MAX_PARAM_NAME = 100;
const int MAX_PARAM_LENGTH = 200;

//! structure to contain one parameter value
typedef struct _proc_state_param_block
{
    uchar used;                              //!< used flag
    char name[MAX_PARAM_NAME + 1];           //!< name of the parameter
    char value[MAX_PARAM_LENGTH + 1];        //!< value of the parameter
#ifdef SHM_USE_CHAINED_BLOCKS
    uchar cont;                              //!< if this is a continued value
                                             //!< data part
    int  next_cont;                          //!< offset to the next param_block,
                                             //!< where value is continued
#endif
    int  next_parameter;                     //!< offset of the next paramter
} proc_state_param_block;

//! structure to contain one process state descriptor
typedef struct _proc_state_psd_block
{
    uchar used;                              //!< used flag
    char name[MAX_PROCESS_NAME + 1];         //!< name of the process
    int pid;                                 //!< process id
    int flags;                               //!< process flags
    time_t start_time;                       //!< process start time
    timeb  timestamp_ms;                     //!< last timestamp with milliseconds
// struct timeb
//   {
//     time_t time;                /* Seconds since epoch, as from `time'.  */
//     unsigned short int millitm; /* Additional milliseconds.  */
//     short int timezone;         /* Minutes west of GMT.  */
//     short int dstflag;          /* Nonzero if Daylight Savings Time used.  */
//   };
    int first_parameter;                     //!< offset of the first parameter
    int last_parameter;                      //!< offset of the last parameter
} proc_state_psd_block;

//! structure to contain control information
typedef struct _proc_state_control_block
{
    // note that the total SHM size itself is stored in AVSharedMemory.
    int start_psd;                           //!< offset of the psd blocks
    int start_param;                         //!< offset of the param blocks
    int max_psd_blocks;                      //!< number of psd blocks
    int max_param_blocks;                    //!< number of param blocks
} proc_state_control_block;

#endif

// End of file
