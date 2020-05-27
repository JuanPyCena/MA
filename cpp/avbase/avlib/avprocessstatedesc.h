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

#if !defined(AVPROCESSSTATEDESC_H_INCLUDED)
#define AVPROCESSSTATEDESC_H_INCLUDED

// Qt includes
#include <QString>
#include <QDateTime>

// local includes
#include "avlib_export.h"
#include "avprocessstate_int.h"

// forward declaration
class AVProcessState;

/////////////////////////////////////////////////////////////////////////////

/*! a process state descriptor
 */
class AVLIB_EXPORT AVProcessStateDesc
{
public:

    /**
     * Constructor for accessing "foreign" SHM.
     * This constructor is used if the foreign process is currently running, see AVProcessState::getProcessStateDesc.
     *
     * TODO: other process can come & go, this is why AVProcessStateDesc::checkPsdBlock exists and is repeatedly called.
     *       why have two constructor flavours for foreign processes? The behaviour is not consistent depending on whether the
     *       process was running at creation time or not.
     */
    AVProcessStateDesc(AVProcessState* base, proc_state_psd_block* psd);

    /**
     * Constructor for accessing "foreign" SHM.
     * This constructor is used if the foreign process is not currently running, see AVProcessState::getProcessStateDesc.
     *
     * TODO: see above.
     */
    AVProcessStateDesc(AVProcessState* base, const QString& pn);

    /**
     * This constructor is used to create the AVProcessStateDesc for the "own" process.
     *
     * @param base
     * @param pn   The process name, as used in the PSD block.
     * @param pid  The process ID; this is written into the PSD block. TBD: why?
     * @param pflags REMOVE
     */
    AVProcessStateDesc(AVProcessState* base, const QString& pn, int pid, int pflags);

    //! Destructor
    virtual ~AVProcessStateDesc();

    //! set the process name
    bool setProcessName(const QString& pn);

    //! set the process id
    bool setProcessId(int pid);

    //! set the process flags
    bool setProcessFlags(int pflags);

    //! get the process name
    bool getProcessName(QString& process_name);

    //! get the process id
    bool getProcessId(int& process_id);

    //! get the process flags
    bool getProcessFlags(int& flags);

    //! set a user defined parameter, identified by a key
    /*! \note if "#define SHM_USE_CHAINED_BLOCKS" is used: if a
              parameter needs more than MAX_PARAM_LENGTH bytes, it is
              automatically split up into the necessary number of
              param_block's. The \sa getParameter() function will
              automatically join these param_block's together.
              It is not possible to mix up processes using and not using
              chained blocks!
     */
    bool setParameter(const QString& key, const AVCVariant &val);

    //! get a user defined parameter, identified by a key
    bool getParameter(const QString& key, AVCVariant& val);

    //! get and set a user defined parameter, identified by a key (both operations are performed within one lock of the SHM)
    /*!
     * \param key name of parameter to get/set
     * \param get_val value before setting the new value set_val
     * \param set_val value to set after getting actual value
     * \return true if successful, false if one or both (get and set) operations failed
     */
    bool getAndSetParameterAtomic(const QString& key, AVCVariant& get_val, const AVCVariant& set_val);

    //! return a list of all available parameter names
    /*! \param list the place to store the parameter names
     */
    bool getParameterList(QStringList& list);

    //! return if the given parameter name key is available
    bool hasParameter(const QString& key);

    //! clear data structures
    void clearDataStructures();

    //! update the timestamp
    void updateTimeStamp();

    //! Get the current timestamp.
    //! If the process state descriptor is invalid (e.g. process is not running), will return an
    //! invalid QDateTime.
    QDateTime getTimeStamp();

    //! Get the process start time.
    //! If the process state descriptor is invalid (e.g. process is not running), will return an
    //! invalid QDateTime.
    QDateTime getStartTime();

    //! checks if this process state descriptor is valid
    bool isValid();

    //! Returns the current date and time, using time(time_t *timer). This
    //! timestamp can be compared to the timestamps SHM.
    static QDateTime currentDateTime();

protected:
    void checkPsdBlock();
    void writePsdBlock();
    void readPsdBlock();

    //! set a user defined paramter, identified by a key
    /*! This function does not call checkPsdBlock() and does not lock m_base!
        \note if "#define SHM_USE_CHAINED_BLOCKS" is used: if a
              parameter needs more than MAX_PARAM_LENGTH bytes, it is
              automatically split up into the necessary number of
              param_block's. The \sa getParameter() function will
              automatically join these param_block's together.
              It is not possible to mix up processes using and not using
              chained blocks!
     */
    bool setParameterInternal(const QString& key, const AVCVariant& val);

    //! get a user defined paramter, identified by a key
    /*! This function does not call checkPsdBlock() and does not lock m_base!
     */
    bool getParameterInternal(const QString& key, AVCVariant& val);

    proc_state_param_block* findParameter(const QString& key);

private:
    //! Copy constructor
    /*! Declared private as copying is not possible */
    AVProcessStateDesc(const AVProcessStateDesc&);

    //! Copy constructor
    /*! Declared private as copying is not possible */
    inline AVProcessStateDesc& operator=(const AVProcessStateDesc&);

private:

    QString         m_process_name;  //!< the process name
    int             m_process_id;    //!< the process id
    int             m_flags;         //!< the process flags. TODO CM document. Where is this used?

    AVProcessState*        m_base;    //!< pointer to process state object
    //! Pointer to psd block in shm segment. This will be 0 if the corresponding process is not
    //! running!
    proc_state_psd_block* m_psd;
    bool                      m_created; //!< if the psd block has been created within this object
};

#endif

// End of file
