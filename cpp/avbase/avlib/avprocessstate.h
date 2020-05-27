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
    \brief   A Process state class
*/

#if !defined(AVPROCESSSTATE_H_INCLUDED)
#define AVPROCESSSTATE_H_INCLUDED

// avlib includes
#include "avmisc.h"
#include "avlog.h"

// local includes
#include "avlib_export.h"
#include "avprocessstatedesc.h"
#include "avsharedmemory.h"
#include "avprocessstate_int.h"
#include "avsingleton.h"

// forward declaration
class AVCVariant;
class QString;
class QStringList;

///////////////////////////////////////////////////////////////////////////////
//! class to provide a process state with arbitrary variables and make it
//! available via shared memory to a supervisor process.
class AVLIB_EXPORT AVProcessState
{
    friend class AVProcessStateDesc;

public:

    static const QString defaultShmName;

    //! Create a process state
    explicit AVProcessState(const QString& process_name,
                               const QString& shm_name = defaultShmName,
                               bool read_only = false,
                               const QString& application_name = QString::null);

    //! Destructor
    ~AVProcessState();

    //! return a list of all process names currently stored in the shm segment.
    /*! \param list the place to store the process names
     */
    bool getProcessList(QStringList& list);

    /*!
     *  \return A pointer to the process state descriptor for the current process, or 0.
     *          ATTENTION: The returned process state descriptor must not be deleted!
     *          This is 0 if no process name was set.
     *          \sa AVDaemonInit
     *          \sa AVProcessStateFactory::getNewProcessState
     */
    AVProcessStateDesc* getProcessStateDesc() { return m_process_state_desc; }

    //! return the process state descriptor for a given process.
    //! Note that this function never will return 0!
    /*! ATTENTION: The returned process state descriptor needs to be deleted by the caller!
        \param process_name the name of the process
     */
    AVProcessStateDesc* getProcessStateDesc(const QString& process_name);

    //! update the hearbeat timer for the current process
    void updateHeartbeat()
    { if (m_process_state_desc != 0) m_process_state_desc->updateTimeStamp(); }

    /**
     * @brief paramBlockInfoAsString returns a summary of current SHM usage of all processes.
     *
     * @param short_summary If true, only the number of processes and total number of variables are returned. Otherwise,
     *                      a list of processes and their details is also provided.
     *
     * Note: this method locks the SHM, do not call from places where this is already locked.
     *
     * @return A summary string.
     */
    QString paramBlockInfoAsString(bool short_summary=false);

protected:

    //! returns a pointer to a free psd block in the shm segment
    proc_state_psd_block* findFreePsdBlock();

    //! returns a pointer to a free param block in the shm segment
    proc_state_param_block* findFreeParamBlock();

    //! find the the psd block of a given process
    proc_state_psd_block* findPsdBlock(const QString& process_name);

    /**
     * Obtain a non-recursive lock on the SHM. \see AVSharedMemory::lock()
     */
    bool lock();
    bool unlock();

    //! check if psd block has timed out
    bool checkTimeout(const proc_state_psd_block* psd);

    //! clear data structures, e.g. mark them unused
    void clearDataStructures(proc_state_psd_block* psd);

    //! return a pointer to the shm object
    const AVSharedMemory *getShm();

    //! return a pointer to a PSD block specified by an offset
    inline proc_state_psd_block* getPSDBlock(int offset)
    {
        AVASSERT(m_shm != 0);
        proc_state_psd_block *pPSDBlock = reinterpret_cast<proc_state_psd_block*>(m_shm->pointer(offset));
        AVASSERT(pPSDBlock != 0);
        return pPSDBlock;
    }

    //! return a pointer to a PARAM block specified by an offset
    inline proc_state_param_block* getPARAMBlock(int offset)
    {
        AVASSERT(m_shm != 0);
        return reinterpret_cast<proc_state_param_block*>(m_shm->pointer(offset));
    }

protected:

    AVSharedMemory*           m_shm;                //!< shm segment
    proc_state_control_block* m_control;            //!< control block in shm
    //! Information about current process, or 0 if no process name was given.
    AVProcessStateDesc*       m_process_state_desc;
    bool                      m_read_only;          //!< true for readonly access

private:

    //! Copy constructor
    /*! Declared private as copying is not possible */
    AVProcessState(const AVProcessState&);

    //! Copy constructor
    /*! Declared private as copying is not possible */
    inline AVProcessState& operator=(const AVProcessState&);
};

/////////////////////////////////////////////////////////////////////////////

// Class which can only be used as a singleton
class AVLIB_EXPORT AVProcessStateFactory : public AVSingleton<AVProcessStateFactory>
{
    friend class AVSingleton<AVProcessStateFactory>;

public:

    //! returns a reference to the process state configuration
    const AVProcessStateConfig& config() { return m_process_state_config; }

    //! Returns a new process state instance.
    //! ATTENTION: The caller is responsible to delete the object!
    AVProcessState* getNewProcessState(
        const QString &psn,
        const QString &shm = AVProcessState::defaultShmName,
        bool read_only = false,
        const QString& application_name = QString::null) const
    {
        return new (LOG_HERE) AVProcessState(psn, shm, read_only, application_name);
    }

protected:

    AVProcessStateFactory() {};

    ~AVProcessStateFactory() override{};

protected:

    //!< configuration data
    AVProcessStateConfig m_process_state_config;

private:
    AVProcessStateFactory(const AVProcessStateFactory&);
    const AVProcessStateFactory& operator = (const AVProcessStateFactory&);
};

///////////////////////////////////////////////////////////////////////////////
//                             GLOBAL STUFF
///////////////////////////////////////////////////////////////////////////////

AVLIB_EXPORT extern AVProcessState *AVProcState;

///////////////////////////////////////////////////////////////////////////////

//! global process state pointer initialization routine with shared memory
//! name specification.
/*! \param psn the desired process name
    \param shm the desired shared memory name
*/
inline void AVInitProcessState(const QString &psn, const QString &shm)
{
    if (AVProcState != 0) delete AVProcState;
    AVProcState = AVProcessStateFactory::getInstance()->getNewProcessState(psn, shm);
    AVASSERT(AVProcState != 0);
};

///////////////////////////////////////////////////////////////////////////////

//! global process state pointer initialization routine
/*! \param psn the desired process name
*/
inline void AVInitProcessState(const QString &psn)
{
    if (AVProcState != 0) delete AVProcState;
    AVProcState = AVProcessStateFactory::getInstance()->getNewProcessState(psn);
    AVASSERT(AVProcState != 0);
};

#endif

// End of file
