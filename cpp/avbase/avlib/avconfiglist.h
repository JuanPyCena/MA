///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - AviBit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author    Dr. Thomas Leitner, t.leitner@avibit.com
    \author    QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief     A list of different configurations.

    This template class maintains a list of configurations parameter sets
    which can be held in memory simultaneously and read/written
    from/to the config file.
*/

#if !defined(AVCONFIGLIST_H_INCLUDED)
#define AVCONFIGLIST_H_INCLUDED

// Qt includes
#include <QList>
#include <QMap>
#include <QStringList>

// local includes
#include "avconfig.h" // TODO CM QTP-10

///////////////////////////////////////////////////////////////////////////////
/*!
 * Template class for holding multiple configurations in memory.
 *   This class serves represents the interface for all config list classes. Derived implementations
 *   using old or new config technology exist.
 * \sa AVConfigList
 */
template<typename T>
class AVConfigListBase
{
public:
    //! read all parameters sets from the config file
    /*! If a username is specified, the configuration for this particular
        user is read. Otherwise the default username is used.
        \param un username to read the config data for.
        \param append true to append to the current list of parameter sets.
        \return true if successful, false if not.
    */
    virtual bool readAll(const QString &un = QString::null, bool append = false) = 0;

    //! writes all parameters sets to the config file
    /*! If a username is specified, the configuration for this particular
        user is written. Otherwise the default username is used.
        \param un username to read the config data for.
        \return true if successful, false if not.
    */
    virtual bool writeAll(const QString &un = QString::null) = 0;

    //! sets filename 'fname' of all parameters sets to the new configfilename
    /*! \param fname filename where to save the new config file
      \return true if successful, false if not.
    */
    virtual void setConfigFileName(const QString &fname) = 0;

    //! gets the configuration filename
    /*! \return path and filename of config file (after readAll())
     */
    virtual const QString& getConfigFileName() = 0;

    /*!
     * creates a new parameter set.
     *   the single parameters in the parameter-set are initialized with the parameters from the
     *   specified set or with defaults if no set is specified.
     * \param name the name of the new parameter set.
     * \param ref the index to the initialization parameter set or -1 if defaults should be used.
     * \return true if successful, false if not
     */
    virtual bool newParaSet(const QString &name, int ref = -1) = 0;

    //! set the default user name for readAll and writeAll
    /*! \param un the default m_username
     */
    virtual void setUsername(const QString& name) = 0;

    //! set the default app name for readAll and writeAll
    /*! \param an the default m_appname
     */
    virtual void setAppname(const QString &an) = 0;

    //! return the number of available parameter sets in the list.
    virtual uint count() const = 0;

    //! get ParaSet name for a specified parameter-set index
    /*!
     * \param index index of the parameter set to return the name for.
     * \return parameter set name of null if not found.
     */
    virtual QString getParaSet(int index) = 0;

    /*!
     * switches to a new para set by index.
     * \param i index to switch to
     * \return parameter-set pointer or null if not found
     */
    virtual T *switchToParaSet(int i) = 0;

    /*!
     * switches to a new para set by name.
     * \param ps parameter set name to switch to.
     * \return parameter-set pointer or null if not found
      */
    virtual T *switchToParaSet(const QString &ps) = 0;

    //! clears the complete list
    virtual void clear() = 0;
};

///////////////////////////////////////////////////////////////////////////////
//! Template class for holding multiple configurations in memory (using "old" config technology)
/*! It basically maintains a list of config base classes with different
    parameter sets (or even user names) in memory.

    Make sure that the config class T does not call any method like standardInit() or
    readConfig() in the constructor.

    A typical usage example:

    \code
    AVConfigList<AVMyConfig>* cfgs;
    cfgs.setConfigFileName("filename");
    cfgs.readAll();
    QStringList cfgs_names = cfgs.getParaSetList();

    if ( cfgs.count() > 0 )
        const AVMyConfig* cfg = cfgs->switchToParaSet(cfg_names[0]);
    \endcode

*/

template<class T> class AVConfigList : public AVConfigListBase<T>
{
public:

    typedef AVConfigBase::UnusedParamsPolicy UnusedParamsPolicy; //!< declare UnusedParamsPolicy

    //! create an empty list of configurations
    AVConfigList() {
        m_unusedParamsPolicy = AVConfigBase::LogAndAddUnusedParams;
    }

    //! create an empty list of configurations and set the default m_username.
    /*! \param un the default m_username for all readAll and writeAll calls.
     */
    explicit AVConfigList(const QString &un) {
        m_username = un;
        m_unusedParamsPolicy = AVConfigBase::LogAndAddUnusedParams;
    }

    //! destroy all configurations
    virtual ~AVConfigList() {
        while (!m_cfg_list.isEmpty())
            delete m_cfg_list.takeFirst();
    }

    //! Defines the handling of unused parameters
    //! The policy is set to AVConfigBase::AddUnusedParams | AVConfigBase::LogUnusedParams
    //! as default.
    //! \sa AVConfigBase::UnusedParamsPolicy
    void setUnusedParameterPolicy(UnusedParamsPolicy policy)
    {
        m_unusedParamsPolicy = policy;
    }

    //! Returns current unused parameters policy
    UnusedParamsPolicy getUnusedParameterPolicy() const
    {
        return m_unusedParamsPolicy;
    }

    void disableUnusedParameterWarning()
    {
        switch(m_unusedParamsPolicy)
        {
            case AVConfigBase::LogAndDiscardUnusedParams:
                m_unusedParamsPolicy = AVConfigBase::DiscardUnusedParams;
            return;
            case AVConfigBase::LogAndAddUnusedParams:
               m_unusedParamsPolicy = AVConfigBase::AddUnusedParams;
            return;
            case AVConfigBase::DiscardUnusedParams:
            case AVConfigBase::AddUnusedParams: return;
        }
    }

    void enableUnusedParameterWarning()
    {
        switch(m_unusedParamsPolicy)
        {
            case AVConfigBase::DiscardUnusedParams:
                m_unusedParamsPolicy = AVConfigBase::LogAndDiscardUnusedParams;
            return;
            case AVConfigBase::AddUnusedParams:
               m_unusedParamsPolicy = AVConfigBase::LogAndAddUnusedParams;
            return;
            case AVConfigBase::LogAndDiscardUnusedParams:
            case AVConfigBase::LogAndAddUnusedParams: return;
        }
    }

    //! \copydoc AVConfigListBase<T>::readAll()
    bool readAll(const QString &un = QString::null, bool append = false) override
    {
        // prepare m_username
        if (un != QString::null) m_username = un;

        // get list of parameter sets from the file
        m_base.setUserName(m_username);
        m_base.setUnusedParameterPolicy(m_unusedParamsPolicy);
        if (m_appname != QString::null)   m_base.setAppName(m_appname);
        if (m_cfg_fname != QString::null) m_base.setConfigFileName(m_cfg_fname);

        QStringList sl = m_base.getParaSetList();
        if (sl.isEmpty()) return false;

        // if append is not set -> clear the current list
        if (!append) clear();

        // now read all parameter sets
        for (int i = 0; i < sl.count(); i++)
        {
            QString ps = sl[i];

            AVLogger->Write(LOG_DEBUG," AVConfigList:readAll: "
                            "reading set (" + ps + ")");

            T *cfg = new T;
            AVASSERT(cfg != 0);

            cfg->setUnusedParameterPolicy(m_unusedParamsPolicy);

            if (m_appname != QString::null)   cfg->setAppName(m_appname);
            if (m_cfg_fname != QString::null) cfg->setConfigFileName(m_cfg_fname);

            if (!cfg->readConfig(ps, m_username, false))
            {
                AVLogger->Write(LOG_ERROR, "AVConfigList:readAll: "
                                "could not read config set (" + ps +
                                ") from file (" + cfg->getConfigFileName() + ")");

                if (i == 0)
                {
                    AVLogger->Write(LOG_ERROR, "AVConfigList:readAll: "
                                    "removing config set (" + ps + ")");
                    delete cfg;
                    cfg = 0;
                    return false;
                }
            }

            m_cfg_list.append(cfg);
        }
        if (AVConfigBase::option_save) writeAll();
        processAfterReading();
        return true;
    }

    //! \copydoc AVConfigListBase<T>::writeAll()
    bool writeAll(const QString &un = QString::null) override {
        // prepare m_username

        if (un != QString::null) m_username = un;

        // now write all parameter sets

        for (int i = 0; i < m_cfg_list.count(); i++) {
            T *cfg = m_cfg_list.at(i);

            if (m_appname != QString::null)
                cfg->setAppName(m_appname);
            if (m_cfg_fname != QString::null)
                cfg->setConfigFileName(m_cfg_fname);

            if (cfg != 0) {
                AVConfigBase::writeConfigMode mode = AVConfigBase::Append;
                if (i == 0) mode = AVConfigBase::Overwrite;
                cfg->writeConfig(cfg->getParaSetName(), m_username, mode);
            }
        }
        return true;
    }

    //! \copydoc AVConfigListBase<T>::setConfigFileName()
    void setConfigFileName(const QString &fname) override {
        m_cfg_fname = fname;
        // now change the configfilenames
        for (int i = 0; i < m_cfg_list.count(); i++) {
            T *cfg = m_cfg_list.at(i);
            if (cfg != 0) {
                cfg->setConfigFileName(fname);
            }
        }
    }

    //! \copydoc AVConfigListBase<T>::getConfigFileName()
    const QString& getConfigFileName() override {
        T *cfg = m_cfg_list.at(0);
        if (cfg != 0) {
            return (cfg->getConfigFileName());
        }
        if (m_cfg_fname.right(4) != ".cfg") m_cfg_fname += ".cfg";
        return m_cfg_fname;
    }

    //! get index for a specified parameter-set
    /*! \param ps parameter set to look for
      \return index of para-set or -1 if not found.
    */
    int getIndex(const QString &ps) {
        for (int i = 0; i < m_cfg_list.count(); i++) {
            T *cfg = m_cfg_list.at(i);
            if (cfg != 0) {
                if (cfg->getParaSetName() == ps) return i;
            }
        }
        return -1;
    }

    //! \copydoc AVConfigListBase<T>::getParaSet()
    QString getParaSet(int index) override {
        T *cfg = m_cfg_list.at(index);
        if (cfg != 0) return cfg->getParaSetName();
        return QString::null;
    }

    //! \copydoc AVConfigListBase<T>::count()
    uint count() const override { return m_cfg_list.count(); }

    //! \copydoc AVConfigListBase<T>::setUsername()
    void setUsername(const QString &un) override { m_username = un; }

    //! return current default user name
    const QString& getUsername() const { return m_username; }

    //! \copydoc AVConfigListBase<T>::setAppname()
    void setAppname(const QString &an) override { m_appname = an; }

    //! return current default app name
    const QString& getAppname() const { return m_appname; }

    T *switchToParaSet(int i) override {
        T *cfg = m_cfg_list.at(i);
        return cfg;
    }

    T *getBaseConfig() { return &m_base; }

    T *switchToParaSet(const QString &ps) override {
        int i = getIndex(ps);
        if (i < 0) return 0;
        T *cfg = m_cfg_list.at(i);
        return cfg;
    }

    //! \copydoc AVConfigListBase<T>::newParaSet()
    bool newParaSet(const QString &name, int ref = -1) override {
        T *cfg = new T;
        AVASSERT(cfg != 0);
        m_cfg_list.append(cfg);
        if (ref >= 0) {
            T *cfgref = m_cfg_list.at(ref);
            if (cfgref != 0) *cfg = *cfgref;
        }
        cfg->setParaSetName(name);
        cfg->setUserName(m_username);

        if (m_appname != QString::null)
            cfg->setAppName(m_appname);

        return true;
    }

    //! creates a new parameter set.
    /*! the single parameters in the parameter-set are initialized with
      the parameters from the specified set or with defaults if no set
      is specified.
      \param name the name of the new parameter set.
      \param psref the name of the initialization parameter set.
      \return true if successful, false if not
    */
    bool newParaSet(const QString &name, const QString &psref) {
        int ref = getIndex(psref);
        return newParaSet(name, ref);
    }

    //! delete a parameter-set by index.
    /*! \param i index of the parameter set to delete
      \return true if successful, false if not
    */
    bool deleteParaSet(int i)
    {
        AVASSERT(i >= 0 && i < m_cfg_list.size());
        delete m_cfg_list.takeAt(i);
        return true;
    }

    //! delete a parameter-set by name.
    /*! \param ps name of the parameter-set to delete.
      \return true if successful, false if not
    */
    bool deleteParaSet(const QString &ps) {
        int ref = getIndex(ps);
        return deleteParaSet(ref);
    }

    //! copies parameter set 2 to parameter set 1 by index
    /*! \param ps1 target parameter set
      \param ps2 source parameter set
      \return true if successful, false if not
    */
    bool copyParaSet(int ps1, int ps2) {
        T *cfg1 = m_cfg_list.at(ps1);
        T *cfg2 = m_cfg_list.at(ps2);
        if (cfg1 != 0 && cfg2 != 0) {
            *cfg1 = *cfg2;
        }
        return false;
    }

    //! copies parameter set 2 to parameter set 1 by name
    /*! \param ps1 target parameter set
      \param ps2 source parameter set
      \return true if successful, false if not
    */
    bool copyParaSet(const QString &ps1, const QString &ps2) {
        int r1 = getIndex(ps1);
        int r2 = getIndex(ps2);
        return copyParaSet(r1, r2);
    }

    //! returns a string list containing all available parameter-sets.
    /*! \return a QStringList containing the names of all available
      parameter sets.
    */
    QStringList getParaSetList() {
        QStringList sl;
        for (int i = 0; i < m_cfg_list.count(); i++) {
            T *cfg = m_cfg_list.at(i);
            if (cfg != 0) sl.append(cfg->getParaSetName());
        }
        return sl;
    }

    //! returns a dictionary map all available parameters set names and parameter sets.
    QMap<QString, T*> getParaSetMapByName() {
        QMap<QString, T*> map;
        for (int i = 0; i < m_cfg_list.count(); i++) {
            T *cfg = m_cfg_list.at(i);
            if (cfg != 0)
                map[cfg->getParaSetName()] = cfg;
        }
        return map;
    }

    //! \copydoc AVConfigListBase<T>::clear()
    void clear() override {
        m_cfg_list.clear();
    }

    // Returns an iterator to the config list
    QListIterator<T*> getListIterator() const
    {
        return QListIterator<T*>(m_cfg_list);
    }

protected:
    //! This method is useful when some post processing must be done after reading the config
    /*! It will be called at the end of readAll() if reading was successful. */
    virtual void processAfterReading() { }

private:
    QList<T*>          m_cfg_list;           //!< the config class list
    QString            m_username;           //!< the default m_username
    QString            m_appname;            //!< the application name
    QString            m_cfg_fname;          //!< the configruation filename
    UnusedParamsPolicy m_unusedParamsPolicy; //!< policy accord. to AVConfigBase:UnusedParamsPolicy
    T                  m_base;               //!< base config (default config w/o file read)
};


#endif

// End of file
