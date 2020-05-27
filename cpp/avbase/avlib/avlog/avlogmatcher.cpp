///////////////////////////////////////////////////////////////////////////////
//
// Package:   ADMAX - Avibit AMAN/DMAN Libraries
// QT-Version: QT4
// Copyright: AviBit data processing GmbH, 2001-2011
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Matthias Fuchs, m.fuchs@avibit.com
    \author  QT4-PORT: Matthias Fuchs, m.fuchs@avibit.com
    \brief   Classes for matching logging output to defined patterns
*/


// QT includes

// local includes
#include "avlog/avlogmatcher.h"


///////////////////////////////////////////////////////////////////////////////

AVLogMatcherBase::AVLogMatcherBase(int type)
  : m_matchCount(0),
    m_minMatches(1),
    m_maxMatches(1)
{
    int lev = type & ~LOG_ERRNO_FLAG;
    if (lev < 0 || lev >= AVLog::LOG__OFF)
        m_type = -1;
    else
        m_type = lev;
}

///////////////////////////////////////////////////////////////////////////////

AVLogMatcherBase::~AVLogMatcherBase()
{
}

///////////////////////////////////////////////////////////////////////////////

void AVLogMatcherBase::setMatchConstraints(int minMatches, int maxMatches)
{
    m_minMatches = minMatches;
    m_maxMatches = maxMatches;
}

///////////////////////////////////////////////////////////////////////////////

bool AVLogMatcherBase::match(const AVLogMatcherLogLine &line)
{
    if (m_type == -1 || m_type == line.type)
    {
        if (doMatch(line))
        {
            m_matchedLines.append("Matched: Type \"" + AVLog::levelToString(line.type) + "\" "
                                  "Message \"" + line.message + "\"\n");
            return true;
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

QString AVLogMatcherBase::matchedLines() const
{
    return infoString() + "\n" + m_matchedLines;
}

///////////////////////////////////////////////////////////////////////////////

bool AVLogMatcherBase::fullfilledConstraints() const
{
    // check if number of matches fulfills the minimum matches constraint
    // a m_minMatches <= 0 means that there is no constraint
    if (m_minMatches > 0 && m_matchCount < m_minMatches)
        return false;

    // check if number of matches fulfills the maximum matches constraint
    // a m_maxMatches < 0 means that there is no constraint
    if (m_maxMatches >= 0 && m_matchCount > m_maxMatches)
        return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

QString AVLogMatcherBase::errorString() const
{
    // check if there actually was an error
    if (!fullfilledConstraints())
        return createErrorString();
    else
        return QString();
}


///////////////////////////////////////////////////////////////////////////////

QString AVLogMatcherBase::createErrorString() const
{
    return infoString() + " " + getConstraintsMessage() + "\n";
}

///////////////////////////////////////////////////////////////////////////////

void AVLogMatcherBase::resetCount()
{
    m_matchCount = 0;
}

///////////////////////////////////////////////////////////////////////////////

QString AVLogMatcherBase::getConstraintsMessage() const
{
    return QString("Matched " + QString::number(m_matchCount) + " times. Expected: min=" +
                   QString::number(m_minMatches) + ", max=" + QString::number(m_maxMatches));
}

///////////////////////////////////////////////////////////////////////////////

AVLogStringMatcher::AVLogStringMatcher(const QString &pattern, MatchingType type, int logType)
  : AVLogMatcherBase(logType),
    m_matchType(type),
    m_pattern(pattern)
{
}

///////////////////////////////////////////////////////////////////////////////

AVLogStringMatcher::~AVLogStringMatcher()
{
}

///////////////////////////////////////////////////////////////////////////////

QString AVLogStringMatcher::infoString() const
{
    return QString("AVStringMatcher: level " + AVLog::levelToString(m_type) + " log " +
                   matchTypeToString(m_matchType) + " \"" + m_pattern + "\"");
}

///////////////////////////////////////////////////////////////////////////////

QString AVLogStringMatcher::createErrorString() const
{
    return infoString() + " " + getConstraintsMessage() + "\n";
}

///////////////////////////////////////////////////////////////////////////////

bool AVLogStringMatcher::doMatch(const AVLogMatcherLogLine &line)
{
    const QString &log = line.message;
    bool matched = false;

    switch(m_matchType) {
        case STARTS_WITH:
            matched = log.startsWith(m_pattern, Qt::CaseSensitive );
            break;
        case ENDS_WITH:
            matched = log.endsWith(m_pattern, Qt::CaseSensitive );
            break;
        case CONTAINS:
            matched = log.contains(m_pattern);
            break;
        case EQUALS:
            matched = (log == m_pattern);
            break;
    }

    if (matched)
        ++m_matchCount;

    return matched;
}

///////////////////////////////////////////////////////////////////////////////

QString AVLogStringMatcher::matchTypeToString(MatchingType matchingType)
{
    switch (matchingType)
    {
        case STARTS_WITH:   return "STARTS_WITH";
        case ENDS_WITH:     return "ENDS_WITH";
        case CONTAINS:      return "CONTAINS";
        case EQUALS:        return "EQUALS";
    }

    return QString();
}

///////////////////////////////////////////////////////////////////////////////

AVLogRegExpMatcher::AVLogRegExpMatcher(const QRegExp &pattern, int type)
  : AVLogMatcherBase(type),
    m_pattern(pattern)
{
}

///////////////////////////////////////////////////////////////////////////////

AVLogRegExpMatcher::~AVLogRegExpMatcher()
{
}

///////////////////////////////////////////////////////////////////////////////

QString AVLogRegExpMatcher::infoString() const
{
    return QString("AVRegExpMatcher: level " + AVLog::levelToString(m_type) + " \"" +
                   m_pattern.pattern() + "\"");
}

///////////////////////////////////////////////////////////////////////////////

bool AVLogRegExpMatcher::doMatch(const AVLogMatcherLogLine &line)
{
    const QString &log = line.message;
    const bool matched = log.contains(m_pattern);
    if (matched)
        ++m_matchCount;

    return matched;
}

///////////////////////////////////////////////////////////////////////////////

AVLogTypeMatcher::AVLogTypeMatcher(int type)
  : AVLogMatcherBase(type)
{
}

///////////////////////////////////////////////////////////////////////////////

AVLogTypeMatcher::~AVLogTypeMatcher()
{
}

///////////////////////////////////////////////////////////////////////////////

QString AVLogTypeMatcher::infoString() const
{
    return "AVLogTypeMatcher: level " + AVLog::levelToString(m_type);
}

///////////////////////////////////////////////////////////////////////////////

QString AVLogTypeMatcher::createErrorString() const
{
    return infoString() + " " + getConstraintsMessage() + " Unexpected:\n" + m_error;
}

///////////////////////////////////////////////////////////////////////////////

bool AVLogTypeMatcher::doMatch(const AVLogMatcherLogLine &line)
{
    // type already checked in match
    ++m_matchCount;

    // add the line to the error message if it was unexpected
    if (m_maxMatches != -1 && m_matchCount > m_maxMatches)
        m_error.append("\"" +line.message + "\"\n");

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void AVLogTypeMatcher::resetCount()
{
    m_matchCount = 0;
    m_error.clear();
}

///////////////////////////////////////////////////////////////////////////////

AVLogNoProblemsMatcher::AVLogNoProblemsMatcher(int minType)
  : AVLogMatcherBase(-1)
{
    // set the minimum logging level of log entries not allowed to appear
    int lev = minType & ~LOG_ERRNO_FLAG;
    if (lev < 0 || lev >= AVLog::LOG__OFF)
        m_minType = AVLog::LOG__WARNING;
    else
        m_minType = lev;

    for (int i = m_minType; i < AVLog::LOG__OFF; ++i) {
        AVLogTypeMatcher *noType = new AVLogTypeMatcher(i);
        AVASSERT(noType);
        noType->setMatchConstraints(0, 0);
        m_matchers.append(noType);
    }
}

///////////////////////////////////////////////////////////////////////////////

AVLogNoProblemsMatcher::~AVLogNoProblemsMatcher()
{
    QVector<AVLogTypeMatcher*>::iterator it;
    QVector<AVLogTypeMatcher*>::iterator itEnd = m_matchers.end();
    for (it = m_matchers.begin(); it != itEnd; ++it)
        delete *it;
}

///////////////////////////////////////////////////////////////////////////////

QString AVLogNoProblemsMatcher::infoString() const
{
    return "AVLogNoProblemsMatcher: mintype " + AVLog::levelToString(m_minType);
}

///////////////////////////////////////////////////////////////////////////////

QString AVLogNoProblemsMatcher::createErrorString() const
{
    QString error(infoString() + " types result:\n===TYPES BEGIN===\n");

    QVector<AVLogTypeMatcher*>::const_iterator it;
    QVector<AVLogTypeMatcher*>::const_iterator itEnd = m_matchers.constEnd();
    for (it = m_matchers.constBegin(); it != itEnd; ++it)
        error.append((*it)->errorString());
    error.append("===TYPES END===\n");

    return error;
}

///////////////////////////////////////////////////////////////////////////////

bool AVLogNoProblemsMatcher::fullfilledConstraints() const
{
    QVector<AVLogTypeMatcher*>::const_iterator it;
    QVector<AVLogTypeMatcher*>::const_iterator itEnd = m_matchers.constEnd();
    for (it = m_matchers.constBegin(); it != itEnd; ++it) {
        if (!(*it)->fullfilledConstraints())
            return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void AVLogNoProblemsMatcher::resetCount()
{
    m_matchCount = 0;
    QVector<AVLogTypeMatcher*>::iterator it;
    QVector<AVLogTypeMatcher*>::iterator itEnd = m_matchers.end();
    for (it = m_matchers.begin(); it != itEnd; ++it)
        (*it)->resetCount();
}

///////////////////////////////////////////////////////////////////////////////

bool AVLogNoProblemsMatcher::doMatch(const AVLogMatcherLogLine &line)
{
    QVector<AVLogTypeMatcher*>::iterator it;
    QVector<AVLogTypeMatcher*>::iterator itEnd = m_matchers.end();
    for (it = m_matchers.begin(); it != itEnd; ++it) {
        if ((*it)->match(line))
            return true;
    }

    return false;
}

// End of file
