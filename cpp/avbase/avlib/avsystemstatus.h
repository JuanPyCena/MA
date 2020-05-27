///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// QT-Version: QT5
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author   Alexander Randeu, a.randeu@avibit.com, Wolfgang Eder, w.eder@avibit.com
    \author   QT4-PORT: Karlheinz Wohlmuth, k.wohlmuth@avibit.com
    \brief    This file declares the base class AVSystemStatus, which provides an API
              to query the status of SHM fields.
*/

#ifndef AVSYSTEMSTATUS_H
#define AVSYSTEMSTATUS_H

// Qt includes
#include <qdatetime.h>
#include <qstringlist.h>

// avlib includes
#include "avlib_export.h"
#include "avconfig.h"

class AVProcessStateDesc;

const int DEFAULT_HEARTBEAT_RATE = 5;

///////////////////////////////////////////////////////////////////////////////

//! This class provides the API to query the status of variables in the shared
//! memory of processes.
/*! You should derive from this class and implement your own functions
    e.g. checkVASLink. If you need this class as a singelton, use the
    AVSingleton template.
*/

class AVLIB_EXPORT AVSystemStatus
{
public:
    //! Default constructor
    AVSystemStatus() {}

    //! Destructor
    virtual ~AVSystemStatus() {}

    //! Check and answer the up-time (in seconds) of the specified process
    int checkUptime(const QString& processName, bool* ok = 0) const;

    //! Answer a shared memory parameter for a given process and parameter name
    int getIntParameter(const QString& processName, const QString& paramName,
                        int defaultValue = -1, bool* ok = 0) const;

    //! Answer a shared memory parameter for a given process and parameter name
    int getIntParameter(AVProcessStateDesc *desc, const QString& paramName,
                        int defaultValue = -1, bool* ok = 0) const;

    //! Answer a shared memory parameter for a given process and parameter name
    uint getUIntParameter(const QString& processName, const QString& paramName,
                          uint defaultValue = 0, bool* ok = 0) const;

    //! Answer a shared memory parameter for a given process and parameter name
    double getDoubleParameter(const QString& processName,
                              const QString& paramName,
                              double defaultValue = -1.0,
                              bool* ok = 0) const;

    //! Answer a shared memory parameter for a given process and parameter name
    bool getBoolParameter(const QString& processName, const QString& paramName,
                          bool defaultValue = false, bool* ok = 0) const;

    //! Answer a shared memory parameter for a given process and parameter name
    QString getStringParameter(const QString& processName,
                               const QString& paramName,
                               const QString& defaultValue = "DEFAULT",
                               bool* ok = 0) const;

    //! Answer a shared memory parameter for a given process and parameter name
    QStringList getStringListParameter(const QString& processName,
                                       const QString& paramName,
                                       bool* ok = 0) const;

    //! Answer a shared memory parameter for a given process and parameter name
    AVIntList getIntListParameter(const QString& processName,
                                  const QString& paramName,
                                  bool* ok = 0) const;




    //! Answer a shared memory parameter for a given process and parameter name
    int getIntParameterCheckAlive(const QString& processName, const QString& paramName,
                                  int defaultValue = -1, bool* ok = 0,
                                  int heartbeatRate = DEFAULT_HEARTBEAT_RATE) const;
    //! Answer a shared memory parameter for a given process and parameter name
    uint getUIntParameterCheckAlive(const QString& processName, const QString& paramName,
                                    uint defaultValue = 0, bool* ok = 0,
                                    int heartbeatRate = DEFAULT_HEARTBEAT_RATE) const;
    //! Answer a shared memory parameter for a given process and parameter name
    double getDoubleParameterCheckAlive(const QString& processName,
                                        const QString& paramName,
                                        double defaultValue = -1.0,
                                        bool* ok = 0,
                                        int heartbeatRate = DEFAULT_HEARTBEAT_RATE) const;
    //! Answer a shared memory parameter for a given process and parameter name
    bool getBoolParameterCheckAlive(const QString& processName, const QString& paramName,
                                    bool defaultValue = false, bool* ok = 0,
                                    int heartbeatRate = DEFAULT_HEARTBEAT_RATE) const;
    //! Answer a shared memory parameter for a given process and parameter name
    QString getStringParameterCheckAlive(const QString& processName,
                                         const QString& paramName,
                                         const QString& defaultValue = "DEFAULT",
                                         bool* ok = 0,
                                         int heartbeatRate = DEFAULT_HEARTBEAT_RATE) const;
    //! Answer a shared memory parameter for a given process and parameter name
    QStringList getStringListParameterCheckAlive(const QString& processName,
                                                 const QString& paramName,
                                                 bool* ok = 0,
                                                 int heartbeatRate = DEFAULT_HEARTBEAT_RATE) const;
    //! Answer a shared memory parameter for a given process and parameter name
    AVIntList getIntListParameterCheckAlive(const QString& processName,
                                            const QString& paramName,
                                            bool* ok = 0,
                                            int heartbeatRate = DEFAULT_HEARTBEAT_RATE) const;
    

    
    //! Set a shared memory parameter for a given process and parameter name
    bool setIntParameter(const QString& processName, const QString& paramName,
                         int value) const;

    //! Set a shared memory parameter for a given process and parameter name
    bool setUIntParameter(const QString& processName, const QString& paramName,
                          uint value) const;

    //! Set a shared memory parameter for a given process and parameter name
    bool setDoubleParameter(const QString& processName,
                            const QString& paramName,
                            double value) const;

    //! Set a shared memory parameter for a given process and parameter name
    bool setBoolParameter(const QString& processName,
                          const QString& paramName,
                          bool value) const;

    //! Set a shared memory parameter for a given process and parameter name
    bool setStringParameter(const QString& processName,
                            const QString& paramName,
                            const QString& value) const;

    //! Set a shared memory parameter for a given process and parameter name
    bool setStringListParameter(const QString& processName,
                                const QString& paramName,
                                const QStringList& value) const;

    //! Set a shared memory parameter for a given process and parameter name
    bool setIntListParameter(const QString& processName,
                             const QString& paramName,
                             const AVIntList& value) const;
    
protected:
    //! Answer a process state descriptor for the specified process. The caller
    //! is responsible for deleting the result.
    AVProcessStateDesc *getProcessStateDesc(const QString& processName) const;

    //! Helper function used by the get***CheckAlive methods.
    bool getParameter(AVCVariant& result, AVCVariant::Type type,
                      const QString& processName,
                      const QString& paramName,
                      int heartbeatRate) const;

    //! Answer whether the specified process is alive
    /*! The heartbeat timestamp in shared memory is checked against the current
        time.
        \param name          the name of the process, e.g. "dswitch"
        \param heartbeatRate the regular heartbeat rate of the process, in
                             seconds, usually 5
        \return              whether the process has signalled a heartbeat
                             within (heartbeatRate + 5) seconds
    */
    bool isProcessAlive(const QString& name, int heartbeatRate = DEFAULT_HEARTBEAT_RATE) const;

    //! Used internally.
    //! \param processName Used for error messages only.
    bool isProcessAlive(AVProcessStateDesc *desc,
                        const QString& processName,
                        int heartbeatRate) const;

private:

    //! dummy copy constructor
    AVSystemStatus(const AVSystemStatus &);

    //! dummy assignment operator
    AVSystemStatus &operator=(const AVSystemStatus &);
};

#endif // AVSYSTEMSTATUS_H

// End of file
