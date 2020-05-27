///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - AviBit Library
//
//////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  DI Alexander Wemmer, a.wemmer@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Checks a given SHM  block variable for it's value. If it
    says block, the doBlock() methods returns true, false otherwise.
*/

#include "avshmcheckblock.h"

/////////////////////////////////////////////////////////////////////////////

AVSHMCheckBlock::AVSHMCheckBlock(const QString& shm_var_name, const QString& proc_name)
{
    AVASSERT(!shm_var_name.isEmpty());

    m_shm_var_name = shm_var_name;
    m_proc_name = proc_name;

    // init variable in SHM
    doBlock();
};

/////////////////////////////////////////////////////////////////////////////

bool AVSHMCheckBlock::doBlock()
{
    if (!AVProcState)
    {
        AVLogger->Write(LOG_WARNING, "AVSHMCheckBlock:doBlock: AVProcState was NULL");
        return false;
    }

    AVProcessStateDesc* process_state_desc = NULL;

    if (m_proc_name.isEmpty())
        process_state_desc = AVProcState->getProcessStateDesc();
    else
        process_state_desc = AVProcState->getProcessStateDesc(m_proc_name);

    AVASSERT(process_state_desc != 0);
    if (!process_state_desc->isValid())
    {
        AVLogger->Write(LOG_WARNING, "AVSHMCheckBlock:doBlock: process state was invalid");
        return false;
    }

    if (!process_state_desc->hasParameter(m_shm_var_name))
    {
        AVLogger->Write(LOG_INFO, "AVSHMCheckBlock:doBlock: creating new variable %s for "
                        "process %s with initial value = 0",
                        qPrintable(m_shm_var_name), qPrintable(m_proc_name));

        m_shm_var_value.set(static_cast<int>(0));
        process_state_desc->setParameter(m_shm_var_name, m_shm_var_value);

        AVASSERT(process_state_desc->getParameter(m_shm_var_name, m_shm_var_value));

        if (!m_proc_name.isEmpty()) delete process_state_desc;
        // TODO FIXXME: really return false here, because when we call this function the
        // first time and the param does not exist, we initialize it with 0.
        // On a second call the param already exists with an value of 0 and we return true.
        return false;
    }

    AVASSERT(process_state_desc->getParameter(m_shm_var_name, m_shm_var_value));

    if (!m_proc_name.isEmpty()) delete process_state_desc;

    if (m_shm_var_value.type() != AVCVariant::Int)
    {
        AVLogger->Write(LOG_ERROR, "AVSHMCheckBlock:doBlock: "
                        "Variable %s of process %s is not an Integer - we do not block",
                        qPrintable(m_shm_var_name), qPrintable(m_proc_name));
        return false;
    }

    return ((m_shm_var_value.asInt() % 2) != 0);
}

// End of file
