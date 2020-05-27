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
#include <QDataStream>

// AVlib includes
#include "avdatastream.h"
#include "avdatetime.h"
#include "avlog.h"

// local includes
#include "avprocessstate.h"
#include "avprocessstatedesc.h"

#define DO_DEBUGGING 0


///////////////////////////////////////////////////////////////////////////////

AVProcessStateDesc::AVProcessStateDesc(
    AVProcessState* base, proc_state_psd_block* psd) :
    m_process_id(-1),
    m_flags(0),
    m_base(base),
    m_psd(psd),
    m_created(false)
{
    if (m_psd != 0) readPsdBlock();
}

///////////////////////////////////////////////////////////////////////////////

AVProcessStateDesc::AVProcessStateDesc(AVProcessState* base, const QString& pn) :
        m_process_name(pn),
        m_process_id(-1),
        m_flags(0),
        m_base(base),
        m_psd(0),
        m_created(false)
{
    AVASSERT(m_base != 0);
}

///////////////////////////////////////////////////////////////////////////////

AVProcessStateDesc::AVProcessStateDesc(
    AVProcessState* base, const QString& pn, int pid, int pflags) :
    m_process_name(pn),
    m_process_id(pid),
    m_flags(pflags),
    m_base(base),
    m_psd(0),
    m_created(true)
{
    AVASSERT(m_base != 0);

    // first check if there is already a psd with the same name
    m_psd = m_base->findPsdBlock(pn);

    // if we found one, clear the data structures
    if (m_psd != 0)
    {
        AVLogInfo << "AVProcessStateDesc::AVProcessStateDesc: recycling existing PSD block -> clearing existing parameters";
        m_base->clearDataStructures(m_psd);
    }

    // now find a free psd block
    m_base->lock();
    m_psd = m_base->findFreePsdBlock();
    if (m_psd != 0)
    {
        writePsdBlock();
        time(&m_psd->start_time);
    }
    m_base->unlock();

    // summary output (must be outside locked section)
    if (m_psd != nullptr)
    {
        AVLogInfo << "AVProcessStateDesc::AVProcessStateDesc: SHM usage summary:\n" << m_base->paramBlockInfoAsString(true);
    } else
    {
        AVLogInfo << "SHM usage summary:\n" << m_base->paramBlockInfoAsString();
        // We expect a constant number of processes to access our SHM, each one identified by
        // a unique string -> make this fatal as it is expected only at startup time.
        AVLogFatal << "AVProcessStateDesc::AVProcessStateDesc: failed to find a free PSD block.";
    }
}

///////////////////////////////////////////////////////////////////////////////

AVProcessStateDesc::~AVProcessStateDesc()
{
}

///////////////////////////////////////////////////////////////////////////////

void AVProcessStateDesc::clearDataStructures()
{
    if (m_psd != 0) m_base->clearDataStructures(m_psd);
}

///////////////////////////////////////////////////////////////////////////////

bool AVProcessStateDesc::setProcessName(const QString& pn)
{
    checkPsdBlock();
    if (m_psd == 0) return false;
    m_process_name = pn;
    qstrncpy(m_psd->name, qPrintable( pn), MAX_PROCESS_NAME);
    updateTimeStamp();
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVProcessStateDesc::setProcessId(int pid)
{
    checkPsdBlock();
    if (m_psd == 0) return false;

    m_process_id = pid;
    m_psd->pid = pid;
    updateTimeStamp();
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVProcessStateDesc::setProcessFlags(int pflags)
{
    checkPsdBlock();
    if (m_psd == 0) return false;

    m_flags = pflags;
    m_psd->flags = pflags;
    updateTimeStamp();
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVProcessStateDesc::getProcessName(QString& process_name)
{
    checkPsdBlock();
    if (m_psd == 0) return false;

    m_process_name = m_psd->name;
    process_name = m_process_name;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVProcessStateDesc::getProcessId(int& process_id)
{
    checkPsdBlock();
    if (m_psd == 0) return false;

    m_process_id = m_psd->pid;
    process_id = m_process_id;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVProcessStateDesc::getProcessFlags(int& flags)
{
    checkPsdBlock();
    if (m_psd == 0) return false;

    m_flags = m_psd->flags;
    flags = m_flags;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVProcessStateDesc::setParameter(const QString& key, const AVCVariant& val)
{
    checkPsdBlock();
    if (m_psd == 0) return false;

    m_base->lock();
    bool result = setParameterInternal(key, val);
    m_base->unlock();
    return result;
}

///////////////////////////////////////////////////////////////////////////////
#ifdef SHM_USE_CHAINED_BLOCKS
bool AVProcessStateDesc::setParameterInternal(const QString& key, const AVCVariant& val)
{
    proc_state_param_block* param = findParameter(key);
    if (param == 0)
    {
#if DO_DEBUGGING
        AVLogger->Write(LOG_DEBUG2, "AVProcessStateDesc::setParameter: "
                        "Adding new parameter '%s'", qPrintable(key));
#endif

        param = m_base->findFreeParamBlock();

        if (param == 0)
        {
            AVLogger->Write(LOG_ERROR, "AVProcessStateDesc::setParameter: "
                            "could not create param block, no space left "
                            "in shm segment");
#if DO_DEBUGGING
            AVLogger->Write(LOG_DEBUG2,
                            "AVProcessStateDesc::setParameter:\n%s",
                            qPrintable(m_base->paramBlockInfoAsString()));
#endif
            return false;
        }

        // set param name
        qstrncpy(param->name, (const char*) key, MAX_PARAM_NAME);
        param->next_parameter = -1;

        // save the last parameter
        proc_state_param_block* last_param =
            (proc_state_param_block*)m_base->getShm()->pointer(m_psd->last_parameter);
        if (last_param != 0)
        {
            last_param->next_parameter = m_base->getShm()->offset(param);
            m_psd->last_parameter = m_base->getShm()->offset(param);
        }
    }
    else
    {
#if DO_DEBUGGING
        AVLogger->Write(LOG_DEBUG2, "AVProcessStateDesc::setParameter: "
                        "Updating parameter '%s'", qPrintable(key));
#endif
    }

    // update psd structure if the first parameter
    if (m_psd->first_parameter < 0)
    {
        m_psd->first_parameter = m_base->getShm()->offset(param);
        m_psd->last_parameter = m_base->getShm()->offset(param);
#if DO_DEBUGGING
        AVLogger->Write(LOG_DEBUG2, "AVProcessStateDesc::setParameter: "
                        "Adding first parameter %s", qPrintable(key));
    }
    else
    {
        AVLogger->Write(LOG_DEBUG2, "AVProcessStateDesc::setParameter: "
                        "Adding/updating not first parameter %s",
                        qPrintable(key));
#endif
    }

    // first read all data, because we do not know the size of it
    QByteArray ba;
    AVDataStream s(ba, QIODevice::WriteOnly);
    bool rc = val.write(s);
    if (rc)
    {
#if DO_DEBUGGING
        AVLogger->Write(LOG_DEBUG2, "AVProcessStateDesc::setParameter: "
                        "splitting data into blocks...");
#endif
        // now split up the data into parameter blocks
        int remain_size = ba.size();
        int act_pos     = 0;
        while (remain_size > 0)
        {
            int act_size = MAX_PARAM_LENGTH;
            if (remain_size < MAX_PARAM_LENGTH) act_size = remain_size;
            remain_size -= act_size;

            // copy the data to the param
            memcpy(param->value, ba.data() + act_pos, act_size);
            act_pos     += act_size;

#if DO_DEBUGGING
            AVLogger->Write(LOG_DEBUG2, "AVProcessStateDesc::setParameter: "
                            "copied %d bytes from %d (remaining=%d)",
                            act_size, ba.size(), remain_size);
#endif

            if (remain_size > 0)
            {
                // now check, if this is already a continued param. If not we
                // have to find an empty one.
                if (param->cont != 0)
                {
                    param = m_base->getPARAMBlock(param->next_cont);
#if DO_DEBUGGING
                    AVLogger->Write(LOG_DEBUG2,
                                    "AVProcessStateDesc::setParameter: "
                                    "Getting next param block: using "
                                    "alread given one.");
#endif
                }
                else
                {
#if DO_DEBUGGING
                    AVLogger->Write(LOG_DEBUG2,
                                    "AVProcessStateDesc::setParameter: "
                                    "Getting next param block: using "
                                    "new one.");
#endif

                    proc_state_param_block* tmp_param = param;
                    param = m_base->findFreeParamBlock();

                    if (param == 0)
                    {
                        AVLogger->Write(LOG_ERROR,
                                        "AVProcessStateDesc::setParameter: "
                                        "could not create param block, no "
                                        "space left in shm segment");

#if DO_DEBUGGING
                        AVLogger->Write(LOG_DEBUG2,
                                     "AVProcessStateDesc::setParameter:\n%s",
                                     qPrintable(m_base->paramBlockInfoAsString()));
#endif

                        return false;
                    }

                    // save new state to previous param
                    tmp_param->cont = 1;
                    tmp_param->next_cont = m_base->getShm()->offset(param);
                    // currently no next parameter
                    param->next_parameter = -1;
                    param->cont = 0;
                }
            }
        }

        // now correct the last param and free all possible remaining cont blocks
        bool first = true;
        while (param != 0 && param->cont == 1)
        {
            if (first) first = false;
            else param->used = 0;

            int next_cont = param->next_cont;
            param->cont = 0;
            param->next_cont = -1;

#if DO_DEBUGGING
            AVLogger->Write(LOG_DEBUG2, "AVProcessStateDesc::setParameter: "
                            "marking param block %d to be not continued.",
                            m_base->getShm()->offset(param));
#endif

            param = m_base->getPARAMBlock(next_cont);
            if (param != 0 && param->cont == 0) param->used = 0;
        }
    }

    updateTimeStamp();
    return rc;
}

#else

bool AVProcessStateDesc::setParameterInternal(const QString& key, const AVCVariant &val)
{
    proc_state_param_block* param = findParameter(key);
    proc_state_param_block* last_param = 0;

    if (param == 0)
    {
        param = m_base->findFreeParamBlock();

        if (param == 0)
        {
            AVLogError << "AVProcessStateDesc::setParameterInternal: no more free SHM param blocks, failed to set " << key << " to " << val.toString();
            AVLogError << AVLogStream::SuppressRepeated(60) << "SHM usage summary: "
                       << m_base->paramBlockInfoAsString();
#if DO_DEBUGGING
            AVLogger->Write(LOG_DEBUG2,
                            "AVProcessStateDesc::setParameter:\n%s",
                            qPrintable(m_base->paramBlockInfoAsString()));
#endif
            return false;
        }

        // set param name
        qstrncpy(param->name, qPrintable(key), MAX_PARAM_NAME);
        param->next_parameter = -1;

        // save the last parameter
        last_param = reinterpret_cast<struct _proc_state_param_block*>(m_base->getShm()->pointer(m_psd->last_parameter));
    }

    // Serialize the parameter value (there is room for optimization)
    QByteArray ba;
    AVDataStream s(&ba, QIODevice::WriteOnly);
    AVASSERT(val.write(s));

    if (ba.count() > MAX_PARAM_LENGTH)
    {
        AVLogFatal << "AVProcessStateDesc:setParameter: parameter value "
                      "exceeds data limit of " << MAX_PARAM_LENGTH << " bytes\n"
                   << "key: " << key  << "\n"
                   << "parameter value: " << val.toString();
    }

    memcpy(param->value, ba.data(), ba.size());

    // if we added a new parameter -> update data structures
    if (last_param != 0)
    {
        last_param->next_parameter = m_base->getShm()->offset(reinterpret_cast<char*>(param));
        m_psd->last_parameter = m_base->getShm()->offset(reinterpret_cast<char*>(param));
    }

    if (m_psd->first_parameter < 0)
    {
        m_psd->first_parameter = m_base->getShm()->offset(reinterpret_cast<char*>(param));
        m_psd->last_parameter = m_base->getShm()->offset(reinterpret_cast<char*>(param));
    }

    updateTimeStamp();
    return true;
}
#endif

///////////////////////////////////////////////////////////////////////////////

bool AVProcessStateDesc::getParameter(const QString& key, AVCVariant& val)
{
    checkPsdBlock();
    if (m_psd == 0) return false;

    m_base->lock();
    bool result = getParameterInternal(key, val);
    m_base->unlock();
    return result;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SHM_USE_CHAINED_BLOCKS
bool AVProcessStateDesc::getParameterInternal(const QString& key, AVCVariant& val)
{
    proc_state_param_block* param = findParameter(key);
    if (param == 0)
    {
        AVLogger->Write(LOG_WARNING, "AVProcessStateDesc::getParameter: "
                    "could not find parameter '%s'", qPrintable(key));
        return false;
    }

    // first we have to read the whole data, because it can be split up
    // into more than one param field
    QByteArray ba;
    AVDataStream swrite(ba, QIODevice::WriteOnly);

#if DO_DEBUGGING
    AVLogger->Write(LOG_DEBUG2, "AVProcessStateDesc::getParameter: "
                    "Getting first block of param %s", qPrintable(key));
#endif

    swrite.writeRawData(param->value, MAX_PARAM_LENGTH);
    while (param->cont == 1)
    {
        param = m_base->getPARAMBlock(param->next_cont);
        swrite.writeRawData(param->value, MAX_PARAM_LENGTH);

#if DO_DEBUGGING
        AVLogger->Write(LOG_DEBUG2, "AVProcessStateDesc::getParameter: "
                        "Getting next block (size=%d)", ba.size());
#endif
    }

    // now read it into val
    AVDataStream sread(ba, QIODevice::ReadOnly);
    bool rc = val.read(sread);

    return rc;
}

#else

bool AVProcessStateDesc::getParameterInternal(const QString& key, AVCVariant& val)
{
    proc_state_param_block* param = findParameter(key);
    if (param == 0)
    {
        AVLogger->Write(LOG_WARNING, "AVProcessStateDesc::getParameter: "
                    "could not find parameter '%s'", qPrintable(key));
        return false;
    }

    QByteArray ba;
    ba.setRawData(param->value, MAX_PARAM_LENGTH);
    AVDataStream s(&ba, QIODevice::ReadOnly);
    bool rc = val.read(s);
    ba.clear();

    return rc;
}
#endif

///////////////////////////////////////////////////////////////////////////////

bool AVProcessStateDesc::getAndSetParameterAtomic(const QString& key, AVCVariant& get_val, const AVCVariant& set_val)
{
    checkPsdBlock();
    if (m_psd == 0)
        return false;

    m_base->lock();
    bool result = getParameterInternal(key, get_val);
    result &= setParameterInternal(key, set_val);
    m_base->unlock();
    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool AVProcessStateDesc::getParameterList(QStringList& list)
{
    checkPsdBlock();
    if (m_psd == 0) return false;

    proc_state_param_block* param = m_base->getPARAMBlock(m_psd->first_parameter);
    while (param != 0)
    {
        if (param->used == 1 && QString(param->name) != "") list += param->name;
        proc_state_param_block* next_param = m_base->getPARAMBlock(param->next_parameter);
        if (param == next_param)
            AVLogger->Write(LOG_FATAL, "next_parameter %d", param->next_parameter);
        param = next_param;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVProcessStateDesc::hasParameter(const QString& key)
{
    m_base->lock();
    proc_state_param_block *tmp = findParameter(key);
    m_base->unlock();
    return (tmp != 0);
}

///////////////////////////////////////////////////////////////////////////////

bool AVProcessStateDesc::isValid()
{
    return (m_psd != 0);
}

///////////////////////////////////////////////////////////////////////////////

QDateTime AVProcessStateDesc::currentDateTime()
{
    time_t timer;
    time(&timer);
    AVDateTime result;
    result.setTime_t(timer);
    return result;
}

///////////////////////////////////////////////////////////////////////////////

void AVProcessStateDesc::checkPsdBlock()
{
    // just update psd_block for descriptors where we got the psd_block externally
    if (m_created) return;

    // update psd pointer, for example a process could have quit in the meantime
    m_psd = m_base->findPsdBlock(m_process_name);
}

///////////////////////////////////////////////////////////////////////////////

void AVProcessStateDesc::writePsdBlock()
{
    AVASSERT(m_psd != 0); // guaranteed by caller
    qstrncpy(m_psd->name, qPrintable(m_process_name), MAX_PROCESS_NAME);
    m_psd->pid = m_process_id;
    m_psd->flags = m_flags;
    m_psd->used = 1;
    ftime(&m_psd->timestamp_ms);
    m_psd->first_parameter = -1;
    m_psd->last_parameter = -1;
}

///////////////////////////////////////////////////////////////////////////////

void AVProcessStateDesc::readPsdBlock()
{
    AVASSERT(m_psd != 0); // guaranteed by caller
    m_process_name = m_psd->name;
    m_process_id = m_psd->pid;
    m_flags = m_psd->flags;
}

///////////////////////////////////////////////////////////////////////////////

void AVProcessStateDesc::updateTimeStamp()
{
    ftime(&m_psd->timestamp_ms);
}

///////////////////////////////////////////////////////////////////////////////

QDateTime AVProcessStateDesc::getTimeStamp()
{
    checkPsdBlock();
    AVDateTime t;
    if (m_psd == 0) return t;
    t.setTime_t(m_psd->timestamp_ms.time);
    t.setTime(QTime(t.time().hour(),
                    t.time().minute(),
                    t.time().second(),
                    m_psd->timestamp_ms.millitm));
    return t;
}

///////////////////////////////////////////////////////////////////////////////

QDateTime AVProcessStateDesc::getStartTime()
{
    checkPsdBlock();
    AVDateTime t;
    if (m_psd != 0) t.setTime_t(m_psd->start_time);
    return t;
}

///////////////////////////////////////////////////////////////////////////////

proc_state_param_block* AVProcessStateDesc::findParameter(const QString& key)
{
    if (m_psd == 0) return 0;

    proc_state_param_block* param = m_base->getPARAMBlock(m_psd->first_parameter);
    while (param != 0)
    {
        if (key == param->name) break;
        proc_state_param_block* next_param = m_base->getPARAMBlock(param->next_parameter);
        if (param == next_param)
            AVLogger->Write(LOG_FATAL, "next_parameter %d", param->next_parameter);
        param = next_param;
    }

#if DO_DEBUGGING
    if (param == 0)
    {
        AVLogger->Write(LOG_DEBUG2, "AVProcessStateDesc::findParameter: "
                        "first_parameter=%d, last_parameter=%d",
                        m_psd->first_parameter, m_psd->last_parameter);

        param = m_base->getPARAMBlock(m_psd->first_parameter);
        while (param != 0)
        {
            AVLogger->Write(LOG_DEBUG2, "AVProcessStateDesc::findParameter: "
                            "name=%s, next_parameter=%d",
                            param->name, param->next_parameter);
            param = m_base->getPARAMBlock(param->next_parameter);
        }
    }
#endif

    return param;
}

// end of file
