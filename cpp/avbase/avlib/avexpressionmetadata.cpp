///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
// QT4-Equivalent:  avexpressions.cpp
// QT4-Approach: port
// QT4-Progress: finished
// QT4-Tested:
// QT4-Problems:
// QT4-Comment:
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author   Wolfgang Eder, w.eder@avibit.com
    \brief    TODO
*/

// AviBit common includes
#include "avexpressionmetadata.h"
#include "avlog.h"

///////////////////////////////////////////////////////////////////////////////

QStringList AVComputableMetadata::valueOfLHSStrings() const
{
    return QStringList();
}

///////////////////////////////////////////////////////////////////////////////

QStringList AVComputableMetadata::valueOfLHSFirstStrings() const
{
    return QStringList();
}

///////////////////////////////////////////////////////////////////////////////

AVComputableMetadata *AVComputableMetadata::valueOfLHSRemainingMetadata(const QString& first) const
{
    Q_UNUSED(first);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

QStringList AVComputableMetadata::assignValueStrings() const
{
    return QStringList();
}

///////////////////////////////////////////////////////////////////////////////

QStringList AVComputableMetadata::assignValueFirstStrings() const
{
    return QStringList();
}

///////////////////////////////////////////////////////////////////////////////

AVComputableMetadata *AVComputableMetadata::assignValueRemainingMetadata(const QString& first) const
{
    Q_UNUSED(first);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

bool AVComputableMetadata::isRegistered() const
{
    return (getMetadata(name()) == this);
}

///////////////////////////////////////////////////////////////////////////////

void AVComputableMetadata::logMetadataValueOfLHS(const QString& prefix) const
{
    if (prefix.isEmpty()) {
        AVLogInfo << name() << "::valueOfLHS understands";
    }
    QStringList list = valueOfLHSStrings();
    QStringListIterator it1(list);
    while (it1.hasNext()) {
        QString lhs = it1.next();
        int count = list.count(lhs);
        if (count != 1) {
            AVLogWarning << prefix + "  " << lhs << " occurs " << count << " times";
        } else {
            AVLogInfo << prefix + "  " << lhs;
        }
    }
    list = valueOfLHSFirstStrings();
    QStringListIterator it2(list);
    while (it2.hasNext()) {
        QString first = it2.next();
        int count = list.count(first);
        if (count != 1) {
            AVLogWarning << prefix + "  " << first << ". occurs " << count << " times";
        } else {
            AVLogInfo << prefix + "  " << first << ".";
        }
        AVComputableMetadata *metadata = valueOfLHSRemainingMetadata(first);
        if (metadata == 0) {
            AVLogInfo << prefix + "    ?";
        } else if (metadata->isRegistered()) {
            AVLogInfo << prefix + "    -> " << metadata->name();
        } else
            metadata->logMetadataValueOfLHS(prefix + "  ");
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVComputableMetadata::logMetadataAssignValue(const QString& prefix) const
{
    if (prefix.isEmpty()) {
        AVLogInfo << name() << "::assignValue understands";
    }
    QStringList list = assignValueStrings();
    QStringListIterator it1(list);
    while (it1.hasNext()) {
        QString lhs = it1.next();
        int count = list.count(lhs);
        if (count != 1) {
            AVLogWarning << prefix + "  " << lhs << " occurs " << count << " times";
        } else {
            AVLogInfo << prefix + "  " << lhs;
        }
    }
    list = assignValueFirstStrings();
    QStringListIterator it2(list);
    while (it2.hasNext()) {
        QString first = it2.next();
        int count = list.count(first);
        if (count != 1) {
            AVLogWarning << prefix + "  " << first << ". occurs " << count << " times";
        } else {
            AVLogInfo << prefix + "  " << first << ".";
        }
        AVComputableMetadata *metadata = assignValueRemainingMetadata(first);
        if (metadata == 0) {
            AVLogInfo << prefix + "    ?";
        } else if (metadata->isRegistered()) {
            AVLogInfo << prefix + "    -> " << metadata->name();
        } else
            metadata->logMetadataAssignValue(prefix + "  ");
    }
}

///////////////////////////////////////////////////////////////////////////////

AVComputableMetadata *AVComputableMetadata::getMetadata(const QString& name) const
{
    return AVComputableMetadataRegistry::singleton().getMetadata(name);
}

///////////////////////////////////////////////////////////////////////////////

AVComputableMetadata& AVComputableMetadata::accessMetadata(const QString& name) const
{
    return AVComputableMetadataRegistry::singleton().accessMetadata(name);
}

///////////////////////////////////////////////////////////////////////////////

AVComputableMetadataRegistry& AVComputableMetadataRegistry::initializeSingleton()
{
    return setSingleton(new (LOG_HERE) AVComputableMetadataRegistry());
}

///////////////////////////////////////////////////////////////////////////////

AVComputableMetadataRegistry::~AVComputableMetadataRegistry()
{
    qDeleteAll(m_metadataDict);
}

///////////////////////////////////////////////////////////////////////////////

void AVComputableMetadataRegistry::registerMetadata(AVComputableMetadata *metadata)
{
    AVASSERT(metadata != 0);
    QString name = metadata->name();
    AVASSERT(!name.isEmpty());
    if (getMetadata(name) != 0)
        AVLogFatal << "registerMetadata duplicate " << name;
    m_metadataDict.insert(name, metadata);
}

///////////////////////////////////////////////////////////////////////////////

QStringList AVComputableMetadataRegistry::getMetadataNames() const
{
    QStringList result = m_metadataDict.keys();
    result.sort();
    return result;
}

///////////////////////////////////////////////////////////////////////////////

AVComputableMetadata *AVComputableMetadataRegistry::getMetadata(const QString& name) const
{
    return m_metadataDict.value(name, 0);
}

///////////////////////////////////////////////////////////////////////////////

AVComputableMetadata& AVComputableMetadataRegistry::accessMetadata(const QString& name) const
{
    AVComputableMetadata *result = getMetadata(name);
    if (result == 0)
        AVLogFatal << "Missing metadata for " << name;
    return *result;
}

///////////////////////////////////////////////////////////////////////////////

void AVComputableMetadataRegistry::logAllMetadata() const
{
    QStringList names = getMetadataNames();
    QStringListIterator it(names);
    while (it.hasNext()) {
        QString name = it.next();
        AVComputableMetadata *metadata = getMetadata(name);
        AVASSERT(metadata != 0);
        metadata->logMetadataValueOfLHS("");
        metadata->logMetadataAssignValue("");
    }
}

///////////////////////////////////////////////////////////////////////////////

AVGenericComputableMetadata::AVGenericComputableMetadata(const QString& name)
    : AVComputableMetadata(name), m_successorsAdded(false)
{
}

///////////////////////////////////////////////////////////////////////////////

AVGenericComputableMetadata::~AVGenericComputableMetadata()
{
    qDeleteAll(m_valueOfLHSRemainingMetadataDict);
    qDeleteAll(m_assignValueRemainingMetadataDict);
}

///////////////////////////////////////////////////////////////////////////////

void AVGenericComputableMetadata::setValueOfLHSStrings(const QStringList& list)
{
    m_valueOfLHSStrings = list;
}

///////////////////////////////////////////////////////////////////////////////

void AVGenericComputableMetadata::setValueOfLHSFirstStrings(const QStringList& list)
{
    m_valueOfLHSFirstStrings = list;
}

///////////////////////////////////////////////////////////////////////////////

void AVGenericComputableMetadata::setAssignValueStrings(const QStringList& list)
{
    m_assignValueStrings = list;
}

///////////////////////////////////////////////////////////////////////////////

void AVGenericComputableMetadata::setAssignValueFirstStrings(const QStringList& list)
{
    m_assignValueFirstStrings = list;
}

///////////////////////////////////////////////////////////////////////////////

void AVGenericComputableMetadata::addSuccessor(AVComputableMetadata *successor) const
{
    if (successor != 0 && !m_successors.contains(successor))
        m_successors.append(successor);
}

///////////////////////////////////////////////////////////////////////////////

void AVGenericComputableMetadata::addSuccessorsOnce() const
{
    if (!m_successorsAdded) {
        addSuccessors();
        m_successorsAdded = true;
    }
}

///////////////////////////////////////////////////////////////////////////////

AVComputableMetadata *AVGenericComputableMetadata::getValueOfLHSRemainingMetadata(const QString& first) const
{
    Q_UNUSED(first);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

AVComputableMetadata *AVGenericComputableMetadata::getAssignValueRemainingMetadata(const QString& first) const
{
    Q_UNUSED(first);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

QStringList AVGenericComputableMetadata::valueOfLHSStrings() const
{
    QStringList result;
    result += m_valueOfLHSStrings;
    addSuccessorsOnce();
    QListIterator<AVComputableMetadata*> it(m_successors);
    while (it.hasNext()) {
        AVComputableMetadata *successor = it.next();
        result += successor->valueOfLHSStrings();
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////

QStringList AVGenericComputableMetadata::valueOfLHSFirstStrings() const
{
    QStringList result;
    result += m_valueOfLHSFirstStrings;
    addSuccessorsOnce();
    QListIterator<AVComputableMetadata*> it(m_successors);
    while (it.hasNext()) {
        AVComputableMetadata *successor = it.next();
        result += successor->valueOfLHSFirstStrings();
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////

AVComputableMetadata *AVGenericComputableMetadata::valueOfLHSRemainingMetadata(const QString& first) const
{
    AVGenericComputableMetadata *result = m_valueOfLHSRemainingMetadataDict.value(first, 0);
    if (result != 0)
        return result;

    QList<AVComputableMetadata*> list; // all applicable metadata

    if (m_valueOfLHSFirstStrings.contains(first)) {
        AVComputableMetadata *metadata = getValueOfLHSRemainingMetadata(first);
        if (metadata != 0 && !list.contains(metadata))
            list.append(metadata);
    }

    addSuccessorsOnce();
    QListIterator<AVComputableMetadata*> it1(m_successors);
    while (it1.hasNext()) {
        AVComputableMetadata *successor = it1.next();
        AVComputableMetadata *metadata = successor->valueOfLHSRemainingMetadata(first);
        if (metadata != 0 && !list.contains(metadata))
            list.append(metadata);
    }

    if (list.isEmpty())
        return 0;

    if (list.count() == 1)
        return list.at(0);

    result = new AVGenericComputableMetadata(name() + "." + first);
    AVASSERT(result != 0);
    QListIterator<AVComputableMetadata*> it2(list);
    while (it2.hasNext()) {
        AVComputableMetadata *successor = it2.next();
        result->addSuccessor(successor);
    }
    m_valueOfLHSRemainingMetadataDict.insert(first, result);
    return result;
}

///////////////////////////////////////////////////////////////////////////////

QStringList AVGenericComputableMetadata::assignValueStrings() const
{
    QStringList result;
    result += m_assignValueStrings;
    addSuccessorsOnce();
    QListIterator<AVComputableMetadata*> it(m_successors);
    while (it.hasNext()) {
        AVComputableMetadata *successor = it.next();
        result += successor->assignValueStrings();
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////

QStringList AVGenericComputableMetadata::assignValueFirstStrings() const
{
    QStringList result;
    result += m_assignValueFirstStrings;
    addSuccessorsOnce();
    QListIterator<AVComputableMetadata*> it(m_successors);
    while (it.hasNext()) {
        AVComputableMetadata *successor = it.next();
        result += successor->assignValueFirstStrings();
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////

AVComputableMetadata *AVGenericComputableMetadata::assignValueRemainingMetadata(const QString& first) const
{
    AVGenericComputableMetadata *result = m_assignValueRemainingMetadataDict.value(first, 0);
    if (result != 0)
        return result;

    QList<AVComputableMetadata*> list; // all applicable metadata

    if (m_assignValueFirstStrings.contains(first)) {
        AVComputableMetadata *metadata = getAssignValueRemainingMetadata(first);
        if (metadata != 0 && !list.contains(metadata))
            list.append(metadata);
    }

    addSuccessorsOnce();
    QListIterator<AVComputableMetadata*> it1(m_successors);
    while (it1.hasNext()) {
        AVComputableMetadata *successor = it1.next();
        AVComputableMetadata *metadata = successor->assignValueRemainingMetadata(first);
        if (metadata != 0 && !list.contains(metadata))
            list.append(metadata);
    }

    if (list.isEmpty())
        return 0;

    if (list.count() == 1)
        return list.at(0);

    result = new AVGenericComputableMetadata(name() + "." + first);
    AVASSERT(result != 0);
    QListIterator<AVComputableMetadata*> it2(list);
    while (it2.hasNext()) {
        AVComputableMetadata *successor = it2.next();
        result->addSuccessor(successor);
    }
    m_assignValueRemainingMetadataDict.insert(first, result);
    return result;
}

// End of file
