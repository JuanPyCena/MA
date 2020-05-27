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
    \brief   A Process descriptor class
*/


// Qt includes
#include <QtGlobal>
#include <QTextStream>
#include <QString>
#include <QStringList>

#if defined(_OS_WIN32_) || defined(Q_OS_WIN32)
#include <windows.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

// AVlib includes
#include "avlog.h"

// local includes
#include "avprocessstate.h"


///////////////////////////////////////////////////////////////////////////////

const QString AVProcessState::defaultShmName = "processstate";

#if defined(AVPROCESSSTATE_MAIN)

AVProcessState *AVProcState = 0;
bool AVProcessTerminated = false;      // global termination flag

#endif

/////////////////////////////////////////////////////////////////////////////

AVProcessState::AVProcessState(const QString& process_name,
                                     const QString& shm_name,
                                     bool read_only,
                                     const QString& application_name) :
    m_shm(0),
    m_control(0),
    m_process_state_desc(0),
    m_read_only(read_only)
{
    // TODO CM the code below effectively makes processstate.cfg mandatory. Unify with processconfig.cc before doing
    // that? Don't specify config file as optional in AVProcessStateConfig constructor!
    //
    // if (AVConfigBase::process_uses_avconfig2 &&
    //     !AVProcessStateFactory::getInstance()->config().refreshSuccessful())
    // {
    //     AVLogError << "AVProcessState::AVProcessState: skipping initialization because of missing configuration.";
    //     return;
    // }

    m_shm = new AVSharedMemory(shm_name, AVProcessStateFactory::getInstance()->config().SHM_SIZE, application_name);
    AVASSERT(m_shm != 0);

    m_control = reinterpret_cast<proc_state_control_block*>(m_shm->getSegment());
    AVASSERT(m_control != 0);

    // if we have just created the shm segment, and it is not already initialized
    m_shm->lock();
    if (!m_shm->isInitialized())
    {
        m_control->start_psd   = sizeof(proc_state_control_block);
        m_control->start_param =
            (sizeof(proc_state_control_block) +
             AVProcessStateFactory::getInstance()->config().MAX_PSD_BLOCKS *
             sizeof(proc_state_psd_block));
        m_control->max_psd_blocks = AVProcessStateFactory::getInstance()->config().MAX_PSD_BLOCKS;

        int param_size = m_shm->getSize() - sizeof(proc_state_control_block) -
                         AVProcessStateFactory::getInstance()->config().MAX_PSD_BLOCKS *
                         sizeof(proc_state_psd_block);
        if (param_size <= 0)
        {
            AVLogFatal << "Invalid SHM configuration";
        }

        m_control->max_param_blocks = param_size / sizeof(proc_state_param_block);

        // set use flags to false
        proc_state_psd_block* psd = getPSDBlock(m_control->start_psd);
        for (int i=0; i<m_control->max_psd_blocks; i++) psd[i].used = 0;

        proc_state_param_block* param = getPARAMBlock(m_control->start_param);
        for (int j=0; j<m_control->max_param_blocks; j++)
        {
            param[j].used = 0;
#ifdef SHM_USE_CHAINED_BLOCKS
            param[j].cont = 0;
            param[j].next_cont = -1;
#endif
        }

        m_shm->setInitialized();
    }

    m_shm->unlock();

    // register our process only if a process name is specified
    if (!process_name.isEmpty())
    {
        // now create a psd for our process
        int pid = -1;

#if defined(_OS_WIN32_) || defined(Q_OS_WIN32)
        pid = GetCurrentProcessId();
#else
        pid = getpid();
#endif

        m_process_state_desc = new AVProcessStateDesc(this, process_name, pid, 0);
        AVASSERT(m_process_state_desc != 0);
    }
}

///////////////////////////////////////////////////////////////////////////////

AVProcessState::~AVProcessState()
{
    // clear data structures of our process
    if (m_process_state_desc != 0)
    {
        m_process_state_desc->clearDataStructures();
        delete m_process_state_desc;
    }

    AVDEL(m_shm);
}

///////////////////////////////////////////////////////////////////////////////

bool AVProcessState::getProcessList(QStringList& list)
{
    if (m_control == 0)
        return false;

    proc_state_psd_block* psd = getPSDBlock(m_control->start_psd);
    for (int i=0; i<m_control->max_psd_blocks; i++) {
        if (psd[i].used == 1) {
            if (!m_read_only && checkTimeout(&psd[i])) {
                clearDataStructures(&psd[i]);
            } else {
                list += psd[i].name;
            }
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

AVProcessStateDesc* AVProcessState::getProcessStateDesc(const QString& process_name)
{
    proc_state_psd_block* psd = findPsdBlock(process_name);
    if (psd == 0) return new (LOG_HERE) AVProcessStateDesc(this, process_name);
    else          return new (LOG_HERE) AVProcessStateDesc(this, psd);
}

///////////////////////////////////////////////////////////////////////////////

proc_state_psd_block* AVProcessState::findFreePsdBlock()
{
    proc_state_psd_block* psd = getPSDBlock(m_control->start_psd);
    for (int i=0; i<m_control->max_psd_blocks; i++) {
        if (psd[i].used == 0) {
            psd[i].name[0] = '\0';
            // just mark it used now
            psd[i].used = 1;
            return (psd + i);
        }
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

proc_state_param_block* AVProcessState::findFreeParamBlock()
{
    proc_state_param_block* param = getPARAMBlock(m_control->start_param);
    for (int i=0; i<m_control->max_param_blocks; i++) {
        if (param[i].used == 0) {
            param[i].name[0] = '\0';
            // just mark it used now
            param[i].used = 1;
#ifdef SHM_USE_CHAINED_BLOCKS
            param[i].cont = 0;
            param[i].next_cont = -1;
#endif
            return (param + i);
        }
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

proc_state_psd_block* AVProcessState::findPsdBlock(const QString& process_name)
{
    proc_state_psd_block* psd = getPSDBlock(m_control->start_psd);
    for (int i=0; i<m_control->max_psd_blocks; i++)
    {
        if (psd[i].used == 1 && process_name == psd[i].name)
        {
            // check if the psd entry has timed out
            if (!m_read_only && checkTimeout(&psd[i]))
            {
                clearDataStructures(&psd[i]);
                return 0;
            }
            else
            {
                return (psd + i);
            }
        }
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

bool AVProcessState::lock()
{
    return m_shm->lock();
}

///////////////////////////////////////////////////////////////////////////////

bool AVProcessState::unlock()
{
    return m_shm->unlock();
}

///////////////////////////////////////////////////////////////////////////////

bool AVProcessState::checkTimeout(const proc_state_psd_block* psd)
{
    time_t t;
    time(&t);
    if (AVProcessStateFactory::getInstance()->config().PSD_TIMEOUT > 0 &&
        difftime(t, psd->timestamp_ms.time) >
            static_cast<double>(AVProcessStateFactory::getInstance()->config().PSD_TIMEOUT))
        return true;
    else
        return false;
}

///////////////////////////////////////////////////////////////////////////////

void AVProcessState::clearDataStructures(proc_state_psd_block* psd)
{
    if (psd == 0 || m_read_only)
        return;

    lock();
    // clear parameter list
    proc_state_param_block* param = getPARAMBlock(psd->first_parameter);
    while(param != 0) {
        param->used = 0;
#ifdef SHM_USE_CHAINED_BLOCKS
        proc_state_param_block* saved_param = param;
        while (param != 0 && param->cont != 0) {
            proc_state_param_block* tmp_param = param;
            param = getPARAMBlock(param->next_cont);
            tmp_param->used      = 0;
            tmp_param->cont      = 0;
            tmp_param->next_cont = -1;
        }
        param = saved_param;
#endif
        param = getPARAMBlock(param->next_parameter);
    }

    // clear psd block
    if (psd != 0) {
        psd->used = 0;
        psd->first_parameter = -1;
        psd->last_parameter = -1;
    }
    unlock();
}

///////////////////////////////////////////////////////////////////////////////

const AVSharedMemory *AVProcessState::getShm()
{
    return m_shm;
}

///////////////////////////////////////////////////////////////////////////////

QString AVProcessState::paramBlockInfoAsString(bool short_summary)
{
    if (!m_control)
    {
        AVLogError << "AVProcessState::paramBlockInfoAsString: m_control is null";
        return "";
    }

    struct ProcessInfo
    {
        uint m_param_count        = 0; // includes unused parameters
        uint m_unused_param_count = 0; // expected to be always 0
        bool m_timed_out          = false;
        int m_age                 = 0; // minutes
    };
    typedef QMap<QString, ProcessInfo> ProcessInfoMap;

    int total_param_count  = 0;
    int total_param_count2 = 0; // for plausi check
    time_t now; // for age computation
    time(&now);

    ProcessInfoMap process_info;

    // locked section
    lock();
    {
        // iterate over processes
        proc_state_psd_block* psd = getPSDBlock(m_control->start_psd);
        for (int i=0; i<m_control->max_psd_blocks; i++)
        {
            const proc_state_psd_block& cur_psd = psd[i];
            if (!cur_psd.used) continue;

            ProcessInfo cur_info;

            if (checkTimeout(&cur_psd))
            {
                cur_info.m_timed_out = true;
            }
            cur_info.m_age = round(difftime(now, cur_psd.start_time)/60);

            // iterate over parameters
            proc_state_param_block* cur_param = getPARAMBlock(cur_psd.first_parameter);
            while (cur_param != nullptr)
            {
                if (!cur_param->used)
                {
                    // unexpected
                    ++cur_info.m_unused_param_count;
                }

                ++cur_info.m_param_count;
                ++total_param_count;
                cur_param = getPARAMBlock(cur_param->next_parameter);
            }

            process_info[cur_psd.name] = cur_info;
        }

        // plausi check total param count by iterating over all parameters
        proc_state_param_block* param = getPARAMBlock(m_control->start_param);
        for (int i=0; i<m_control->max_param_blocks; ++i)
        {
            if (param[i].used) ++total_param_count2;
        }
    }
    unlock();

    QString str;
    QTextStream ts( &str, QIODevice::WriteOnly );

    ts << "Total SHM size: "  << (m_shm->getSize()>>10)     << " kb" << endl;
    ts << "Process count: "   << process_info.count() << "/" << m_control->max_psd_blocks << endl;
    ts << "Parameter count: " << total_param_count    << "/" << m_control->max_param_blocks << endl;

    const int MIN_SPARE_PSD = 5;
    if (process_info.count() + MIN_SPARE_PSD > m_control->max_psd_blocks)
    {
        AVLogError << AVLogStream::SuppressRepeated(60)
                   << "AVProcessState::paramBlockInfoAsString: less than " << MIN_SPARE_PSD
                   << " free PSD blocks remaining.";
    }
    const int MIN_SPARE_PARAMS = 20;
    if (total_param_count + MIN_SPARE_PARAMS > m_control->max_param_blocks)
    {
        AVLogError << AVLogStream::SuppressRepeated(60)
                   << "AVProcessState::paramBlockInfoAsString: less than " << MIN_SPARE_PARAMS
                   << " parameters remaining.";
    }

    if (short_summary)
    {
        return str;
    }

    for (ProcessInfoMap::const_iterator it = process_info.begin(); it != process_info.end(); ++it)
    {
        const ProcessInfo& cur_info = it.value();
        ts << qSetFieldWidth(20) << it.key() << qSetFieldWidth(0) << ": "
           << "params: "         << qSetFieldWidth(3) << cur_info.m_param_count << qSetFieldWidth(0) << "; "
           << "timed out: "      << cur_info.m_timed_out << "; "
           << "age: "            << cur_info.m_age << " minutes"
           << endl;

        if (cur_info.m_unused_param_count != 0)
        {
            ts << "ERROR: process has " << cur_info.m_unused_param_count << " unused params in its linked list" << endl;
        }
    }

    if (total_param_count != total_param_count2)
    {
        ts << "ERROR: param count mismatch " << total_param_count << " vs " << total_param_count2 << endl;
    }

    return str;
}

// end of file
