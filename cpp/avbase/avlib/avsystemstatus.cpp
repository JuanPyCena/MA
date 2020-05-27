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
    \author   Alexander Randeu, a.randeu@avibit.com,
              Wolfgang Eder, w.eder@avibit.com
    \author   QT4-PORT: Karlheinz Wohlmuth, k.wohlmuth@avibit.com
    \brief    This file declares the base class AVSystemStatus, which provides
              an API to query the status of SHM fields.
*/


// Standard includes
#include <time.h>

// local includes
#include "avprocessstate.h"
#include "avsystemstatus.h"


///////////////////////////////////////////////////////////////////////////////

int AVSystemStatus::checkUptime(const QString& processName, bool *ok) const
{
    if (ok) *ok = false;
    AVProcessStateDesc *desc = getProcessStateDesc(processName);
    if (desc == 0) return 0;
    QDateTime dt = desc->getStartTime();
    if (!dt.isValid()) {
        AVLogger->Write(LOG_ERROR,  "AVSystemStatus::checkUptime: "
                        "%s start time is invalid", qPrintable(processName));
        delete desc;
        return 0;
    }

    // Do not use AVTimeReference::currentDateTime() here.

    QDateTime now = AVProcessStateDesc::currentDateTime();
    if (!now.isValid()) {
        AVLogger->Write(LOG_ERROR,  "AVSystemStatus::checkUptime: "
                        "%s current time is invalid",
                        qPrintable(processName));
        delete desc;
        return 0;
    }
    int secs = dt.secsTo(now);
    if (secs < 0) {
        AVLogger->Write(LOG_ERROR,  "AVSystemStatus::checkUptime: "
                        "%s start time is in the future",
                        qPrintable(processName));
        delete desc;
        return 0;
    }
    delete desc;
    if (ok) *ok = true;
    return secs;
}

///////////////////////////////////////////////////////////////////////////////

AVProcessStateDesc*
AVSystemStatus::getProcessStateDesc(const QString& processName) const
{
    AVLogger->Write(LOG_DEBUG, "AVSystemStatus::getProcessStateDesc: %s",
                    qPrintable(processName));
    //AVProcessState process("", AVProcessState::defaultShmName, true);
    AVProcessState *process = AVProcState;
    if (process == 0) {
        AVLogger->Write(LOG_ERROR, "AVSystemStatus::getProcessStateDesc: "
                        "process state AVProcState is (null)");
        return 0;
    }

    QStringList list;
    process->getProcessList(list);
    if (!list.contains(processName)) {
        AVLogger->Write(LOG_DEBUG, "AVSystemStatus::getProcessStateDesc: "
                        "process %s not in list %s", qPrintable(processName),
                        list.join(",").toLatin1().constData());
        return 0;
    }
    AVProcessStateDesc *desc = process->getProcessStateDesc(processName);
    if (desc == 0) {
        AVLogger->Write(LOG_ERROR, "AVSystemStatus::getProcessStateDesc: "
                        "%s process state descriptor is (null)",
                        qPrintable(processName));
        return 0;
    }
    if (!desc->isValid()) {
        AVLogger->Write(LOG_ERROR, "AVSystemStatus::getProcessStateDesc: "
                        "%s process state descriptor is invalid",
                        qPrintable(processName));
        delete desc;
        return 0;
    }
    return desc;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSystemStatus::getParameter(AVCVariant& result,
                                  AVCVariant::Type type,
                                  const QString& processName,
                                  const QString& paramName,
                                  int heartbeatRate) const
{
    AVProcessStateDesc *desc = getProcessStateDesc(processName);
    if (desc == 0) {
        // don't log error here, it's done in getProcessStateDesc...
        return false;
    }

    // check whether process still is alive.
    if (!isProcessAlive(desc, processName, heartbeatRate)) {
        delete desc;
        return false;
    }

    if (!desc->getParameter(paramName, result)) {
        AVLogger->Write(LOG_ERROR, "AVSystemStatus::getParameter: "
                        "%s:%s not found", qPrintable(processName),
                        qPrintable(paramName));
        delete desc;
        return false;
    }

    if (result.type() != type) {
        AVLogger->Write(LOG_ERROR, "AVSystemStatus::getParameter: "
                        "%s:%s wrong type %d", qPrintable(processName),
                        qPrintable(paramName), result.type());
        delete desc;
        return false;
    }

    delete desc;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSystemStatus::isProcessAlive(const QString& processName,
                                    int heartbeatRate) const
{
    AVProcessStateDesc *desc = getProcessStateDesc(processName);
    if (desc == 0) return false;

    bool ret = isProcessAlive(desc, processName, heartbeatRate);

    delete desc;
    return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSystemStatus::isProcessAlive(AVProcessStateDesc *desc,
                                    const QString& processName,
                                    int heartbeatRate) const
{
    // The currently running process always is alive.
    if (AVProcState != 0) {
        AVProcessStateDesc *ownProcessDesc = AVProcState->getProcessStateDesc();
        if (ownProcessDesc != 0) {
            QString ownProcessName;
            if (ownProcessDesc->getProcessName(ownProcessName))
                if (ownProcessName == processName) return true;
        }
    }

    QDateTime dt = desc->getTimeStamp();
    if (!dt.isValid()) {
        AVLogger->Write(LOG_ERROR, "AVSystemStatus::isProcessAlive: "
                        "%s timestamp is invalid", qPrintable(processName));
        return false;
    }

    // Do not use AVTimeReference::currentDateTime() here.

    QDateTime now = AVProcessStateDesc::currentDateTime();
    if (!now.isValid()) {
        AVLogger->Write(LOG_ERROR, "AVSystemStatus::isProcessAlive: "
                        "%s current time is invalid",
                        qPrintable(processName));
        return false;
    }
    int secs = dt.secsTo(now);
    // we allow the timestamp to be 1 sec in the future because that is what happens sometimes
    if (secs < -1) {
        AVLogger->Write(LOG_ERROR, "AVSystemStatus::isProcessAlive: "
                        "%s timestamp is %d secs in the future",
                        qPrintable(processName), -secs);
        return false;
    }
    if (secs > heartbeatRate + 5) {
        AVLogger->Write(LOG_DEBUG, "AVSystemStatus::isProcessAlive: "
                        "%s last timestamp was %d secs ago",
                        qPrintable(processName), secs);
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

int AVSystemStatus::getIntParameter(const QString& processName,
                                    const QString& paramName, int defaultValue,
                                    bool* ok) const
{
    AVProcessStateDesc *desc = getProcessStateDesc(processName);
    if (desc == 0) {
        if (ok != 0) *ok = false;
        return defaultValue;
    }

    int result = getIntParameter(desc, paramName, defaultValue, ok);
    delete desc;
    return result;
}

///////////////////////////////////////////////////////////////////////////////

int AVSystemStatus::getIntParameter(AVProcessStateDesc *desc,
                                    const QString& paramName,
                                    int defaultValue, bool* ok) const
{
    if (desc == 0) {
        if (ok != 0) *ok = false;
        return defaultValue;
    }

    AVCVariant param;
    if (!desc->getParameter(paramName, param)) {
        QString processName;
        desc->getProcessName(processName);
        AVLogger->Write(LOG_ERROR, "AVSystemStatus::getIntParameter: "
                        "%s:%s not found", qPrintable(processName),
                        qPrintable(paramName));
        if (ok != 0) *ok = false;
        return defaultValue;
    }

    if (param.type() != AVCVariant::Int) {
        QString processName;
        desc->getProcessName(processName);
        AVLogger->Write(LOG_ERROR, "AVSystemStatus::getIntParameter: "
                        "%s:%s wrong type %d", qPrintable(processName),
                        qPrintable(paramName), param.type());
        if (ok != 0) *ok = false;
        return defaultValue;
    }

    if (ok != 0) *ok = true;
    return param.asInt();
}

///////////////////////////////////////////////////////////////////////////////

uint AVSystemStatus::getUIntParameter(const QString& processName,
                                      const QString& paramName,
                                      uint defaultValue,
                                      bool* ok) const
{
    AVProcessStateDesc *desc = getProcessStateDesc(processName);
    if (desc == 0) {
        if (ok != 0) *ok = false;
        return defaultValue;
    }
    AVCVariant param;
    if (!desc->getParameter(paramName, param)) {
        AVLogger->Write(LOG_ERROR, "AVSystemStatus::getUIntParameter: "
                        "%s:%s not found", qPrintable(processName),
                        qPrintable(paramName));
        delete desc;
        if (ok != 0) *ok = false;
        return defaultValue;
    }
    if (param.type() != AVCVariant::UInt) {
        AVLogger->Write(LOG_ERROR, "AVSystemStatus::getUIntParameter: "
                        "%s:%s wrong type %d", qPrintable(processName),
                        qPrintable(paramName), param.type());
        delete desc;
        if (ok != 0) *ok = false;
        return defaultValue;
    }
    delete desc;
    if (ok != 0) *ok = true;
    return param.asUInt();
}

///////////////////////////////////////////////////////////////////////////////

double AVSystemStatus::getDoubleParameter(const QString& processName,
                                          const QString& paramName,
                                          double defaultValue,
                                          bool* ok) const
{
    AVProcessStateDesc *desc = getProcessStateDesc(processName);
    if (desc == 0) {
        if (ok != 0) *ok = false;
        return defaultValue;
    }
    AVCVariant param;
    if (!desc->getParameter(paramName, param)) {
        AVLogger->Write(LOG_ERROR, "AVSystemStatus::getDoubleParameter: "
                        "%s:%s not found", qPrintable(processName),
                        qPrintable(paramName));
        delete desc;
        if (ok != 0) *ok = false;
        return defaultValue;
    }
    if (param.type() != AVCVariant::Double) {
        AVLogger->Write(LOG_ERROR, "AVSystemStatus::getDoubleParameter: "
                        "%s:%s wrong type %d", qPrintable(processName),
                        qPrintable(paramName), param.type());
        delete desc;
        if (ok != 0) *ok = false;
        return defaultValue;
    }
    delete desc;
    if (ok != 0) *ok = true;
    return param.asDouble();
}

///////////////////////////////////////////////////////////////////////////////

bool AVSystemStatus::getBoolParameter(const QString& processName,
                                      const QString& paramName,
                                      bool defaultValue,
                                      bool* ok) const
{
    AVProcessStateDesc *desc = getProcessStateDesc(processName);
    if (desc == 0) {
        if (ok != 0) *ok = false;
        return defaultValue;
    }
    AVCVariant param;
    if (!desc->getParameter(paramName, param)) {
        AVLogger->Write(LOG_ERROR, "AVSystemStatus::getBoolParameter: "
                        "%s:%s not found", qPrintable(processName),
                        qPrintable(paramName));
        delete desc;
        if (ok != 0) *ok = false;
        return defaultValue;
    }
    if (param.type() != AVCVariant::Bool) {
        AVLogger->Write(LOG_ERROR, "AVSystemStatus::getBoolParameter: "
                        "%s:%s wrong type %d", qPrintable(processName),
                        qPrintable(paramName), param.type());
        delete desc;
        if (ok != 0) *ok = false;
        return defaultValue;
    }
    delete desc;
    if (ok != 0) *ok = true;
    return param.asBool();
}

///////////////////////////////////////////////////////////////////////////////

QString AVSystemStatus::getStringParameter(const QString& processName,
                                           const QString& paramName,
                                           const QString& defaultValue,
                                           bool* ok) const
{
    AVProcessStateDesc *desc = getProcessStateDesc(processName);
    if (desc == 0) {
        if (ok != 0) *ok = false;
        return defaultValue;
    }
    AVCVariant param;
    if (!desc->getParameter(paramName, param)) {
        AVLogger->Write(LOG_ERROR, "AVSystemStatus::getStringParameter: "
                        "%s:%s not found", qPrintable(processName),
                        qPrintable(paramName));
        delete desc;
        if (ok != 0) *ok = false;
        return defaultValue;
    }
    if (param.type() != AVCVariant::String) {
        AVLogger->Write(LOG_ERROR, "AVSystemStatus::getStringParameter: "
                        "%s:%s wrong type %d", qPrintable(processName),
                        qPrintable(paramName), param.type());
        delete desc;
        if (ok != 0) *ok = false;
        return defaultValue;
    }
    delete desc;
    if (ok != 0) *ok = true;
    return param.asString();
}

///////////////////////////////////////////////////////////////////////////////

QStringList AVSystemStatus::getStringListParameter(const QString& processName,
                                                   const QString& paramName,
                                                   bool* ok) const
{
    AVProcessStateDesc *desc = getProcessStateDesc(processName);
    if (desc == 0) {
        if (ok != 0) *ok = false;
        return QStringList();
    }
    AVCVariant param;
    if (!desc->getParameter(paramName, param)) {
        AVLogger->Write(LOG_ERROR, "AVSystemStatus::getStringListParameter: "
                        "%s:%s not found", qPrintable(processName),
                        qPrintable(paramName));
        delete desc;
        if (ok != 0) *ok = false;
        return QStringList();
    }
    if (param.type() != AVCVariant::StringList) {
        AVLogger->Write(LOG_ERROR, "AVSystemStatus::getStringListParameter: "
                        "%s:%s wrong type %d", qPrintable(processName),
                        qPrintable(paramName), param.type());
        delete desc;
        if (ok != 0) *ok = false;
        return QStringList();
    }
    delete desc;
    if (ok != 0) *ok = true;
    return param.asStringList();
}

///////////////////////////////////////////////////////////////////////////////

AVIntList AVSystemStatus::getIntListParameter(const QString& processName,
                                              const QString& paramName,
                                              bool* ok) const
{
    AVProcessStateDesc *desc = getProcessStateDesc(processName);
    if (desc == 0) {
        if (ok != 0) *ok = false;
        return AVIntList();
    }
    AVCVariant param;
    if (!desc->getParameter(paramName, param)) {
        AVLogger->Write(LOG_ERROR, "AVSystemStatus::getIntListParameter: "
                        "%s:%s not found", qPrintable(processName),
                        qPrintable(paramName));
        delete desc;
        if (ok != 0) *ok = false;
        return AVIntList();
    }
    if (param.type() != AVCVariant::IntList) {
        AVLogger->Write(LOG_ERROR, "AVSystemStatus::getIntListParameter: "
                        "%s:%s wrong type %d", qPrintable(processName),
                        qPrintable(paramName), param.type());
        delete desc;
        if (ok != 0) *ok = false;
        return AVIntList();
    }
    delete desc;
    if (ok != 0) *ok = true;
    return param.asIntList();
}

///////////////////////////////////////////////////////////////////////////////

int AVSystemStatus::getIntParameterCheckAlive(const QString& processName, const QString& paramName,
                                              int defaultValue, bool* ok,
                                              int heartbeatRate) const
{
    AVCVariant result;
    bool success = getParameter(result, AVCVariant::Int, processName, paramName, heartbeatRate);
    if (ok != 0) *ok = success;

    if (success) return result.asInt();
    else return defaultValue;
}

///////////////////////////////////////////////////////////////////////////////

uint AVSystemStatus::getUIntParameterCheckAlive(const QString& processName,
                                                const QString& paramName,
                                                uint defaultValue, bool* ok,
                                                int heartbeatRate) const
{
    AVCVariant result;
    bool success = getParameter(result, AVCVariant::UInt, processName, paramName, heartbeatRate);
    if (ok != 0) *ok = success;

    if (success) return result.asUInt();
    else return defaultValue;
}

///////////////////////////////////////////////////////////////////////////////

double AVSystemStatus::getDoubleParameterCheckAlive(const QString& processName,
                                                    const QString& paramName,
                                                    double defaultValue,
                                                    bool* ok, int heartbeatRate) const
{
    AVCVariant result;
    bool success = getParameter(result, AVCVariant::Double, processName, paramName, heartbeatRate);
    if (ok != 0) *ok = success;

    if (success) return result.asDouble();
    else return defaultValue;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSystemStatus::getBoolParameterCheckAlive(const QString& processName,
                                                const QString& paramName,
                                                bool defaultValue, bool* ok,
                                                int heartbeatRate) const
{
    AVCVariant result;
    bool success = getParameter(result, AVCVariant::Bool, processName, paramName, heartbeatRate);
    if (ok != 0) *ok = success;

    if (success) return result.asBool();
    else return defaultValue;
}

///////////////////////////////////////////////////////////////////////////////

QString AVSystemStatus::getStringParameterCheckAlive(const QString& processName,
                                                     const QString& paramName,
                                                     const QString& defaultValue,
                                                     bool* ok,
                                                     int heartbeatRate) const
{
    AVCVariant result;
    bool success = getParameter(result, AVCVariant::String, processName, paramName, heartbeatRate);
    if (ok != 0) *ok = success;

    if (success) return result.asString();
    else return defaultValue;
}

///////////////////////////////////////////////////////////////////////////////

QStringList AVSystemStatus::getStringListParameterCheckAlive(const QString& processName,
                                                             const QString& paramName,
                                                             bool* ok,
                                                             int heartbeatRate) const
{
    AVCVariant result;
    bool success = getParameter(result, AVCVariant::StringList,
                                processName, paramName, heartbeatRate);
    if (ok != 0) *ok = success;

    if (success) return result.asStringList();
    else return QStringList();
}

///////////////////////////////////////////////////////////////////////////////

AVIntList AVSystemStatus::getIntListParameterCheckAlive(const QString& processName,
                                                        const QString& paramName,
                                                        bool* ok,
                                                        int heartbeatRate) const
{
    AVCVariant result;
    bool success = getParameter(result, AVCVariant::IntList, processName, paramName, heartbeatRate);
    if (ok != 0) *ok = success;

    if (success) return result.asIntList();
    else return AVIntList();
}

///////////////////////////////////////////////////////////////////////////////

bool AVSystemStatus::setIntParameter(const QString& processName,
                                     const QString& paramName, int value) const
{
    AVProcessStateDesc *desc = getProcessStateDesc(processName);
    if (desc == 0) return false;
    AVCVariant param;
    param.set(value);
    bool result = desc->setParameter(paramName, param);
    if (!result) {
        AVLogger->Write(LOG_ERROR, "AVSystemStatus::setIntParameter: "
                        "error setting %s:%s to %d", qPrintable(processName),
                        qPrintable(paramName), value);
    }
    delete desc;
    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSystemStatus::setUIntParameter(const QString& processName,
                                      const QString& paramName, uint value) const
{
    AVProcessStateDesc *desc = getProcessStateDesc(processName);
    if (desc == 0) return false;
    AVCVariant param;
    param.set(value);
    bool result = desc->setParameter(paramName, param);
    if (!result) {
        AVLogger->Write(LOG_ERROR, "AVSystemStatus::setUIntParameter: "
                        "error setting %s:%s to %d", qPrintable(processName),
                        qPrintable(paramName), value);
    }
    delete desc;
    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSystemStatus::setDoubleParameter(const QString& processName,
                                        const QString& paramName, double value) const
{
    AVProcessStateDesc *desc = getProcessStateDesc(processName);
    if (desc == 0) return false;
    AVCVariant param;
    param.set(value);
    bool result = desc->setParameter(paramName, param);
    if (!result) {
        AVLogger->Write(LOG_ERROR, "AVSystemStatus::setDoubleParameter: "
                        "error setting %s:%s to %f", qPrintable(processName),
                        qPrintable(paramName), value);
    }
    delete desc;
    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSystemStatus::setBoolParameter(const QString& processName,
                                      const QString& paramName, bool value) const
{
    AVProcessStateDesc *desc = getProcessStateDesc(processName);
    if (desc == 0) return false;
    AVCVariant param;
    param.set(value);
    bool result = desc->setParameter(paramName, param);
    if (!result) {
        AVLogger->Write(LOG_ERROR, "AVSystemStatus::setBoolParameter: "
                        "error setting %s:%s to %d", qPrintable(processName),
                        qPrintable(paramName), value);
    }
    delete desc;
    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSystemStatus::setStringParameter(const QString& processName,
                                        const QString& paramName,
                                        const QString& value) const
{
    AVProcessStateDesc *desc = getProcessStateDesc(processName);
    if (desc == 0) return false;
    AVCVariant param;
    param.set(value);
    bool result = desc->setParameter(paramName, param);
    if (!result) {
        AVLogger->Write(LOG_ERROR, "AVSystemStatus::setStringParameter: "
                        "error setting %s:%s to %s", qPrintable(processName),
                        qPrintable(paramName), qPrintable(value));
    }
    delete desc;
    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSystemStatus::setStringListParameter(const QString& processName,
                                            const QString& paramName,
                                            const QStringList& value) const
{
    AVProcessStateDesc *desc = getProcessStateDesc(processName);
    if (desc == 0) return false;
    AVCVariant param;
    param.set(value);
    bool result = desc->setParameter(paramName, param);
    if (!result) {
        AVLogger->Write(LOG_ERROR, "AVSystemStatus::setStringListParameter: "
                        "error setting %s:%s to %s", qPrintable(processName),
                        qPrintable(paramName), qPrintable(value.join(";")));
    }
    delete desc;
    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSystemStatus::setIntListParameter(const QString& processName,
                                         const QString& paramName,
                                         const AVIntList& value) const
{
    AVProcessStateDesc *desc = getProcessStateDesc(processName);
    if (desc == 0) return false;
    AVCVariant param;
    param.set(value);
    bool result = desc->setParameter(paramName, param);
    if (!result) {
        AVLogger->Write(LOG_ERROR, "AVSystemStatus::setIntListParameter: "
                        "error setting %s:%s to list with %d entries",
                        qPrintable(processName),
                        qPrintable(paramName), value.count());
    }
    delete desc;
    return result;
}

// End of file
