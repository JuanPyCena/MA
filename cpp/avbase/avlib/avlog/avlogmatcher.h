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

#if !defined AVLOGMATCHER_H_INCLUDED
#define AVLOGMATCHER_H_INCLUDED

// QT includes
#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QVector>

// avlib includes
#include "avlib_export.h"
#include "avlog.h"
#include "avmacros.h"
#include "avlib_export.h"

//! Struct for a log line
struct AVLIB_EXPORT AVLogMatcherLogLine
{
    int     type;       //!< type of the log message
    QString message;    //!< log message not containing the time or any other header
};


//! Base class for matching log output lines
/*! AVLogMatcherBase is the abstract base class for matching log lines. It should be used
    in combination with AVLogMonitor to filter log output of messages you expected and only keep
    those that were not expected.

    A log line is a line of the log output. It contains the
    \li message which itself has no header, thus no timestamp and no "WARNING" for example
    \li type like AVLog::LOG_DEBUG

    There are two main parts of AVLogMatcherBase. One is the match method where you should return
    true if a specific line did match.
    Though a match does not mean that the matcher actually worked. For that there is
    fullfilledConstraints where you can return false if some constraints were not fulfilled.

    \remark Subclassing:
    When subclassing take care of the following steps.
    \li You have to subclass doMatch. Here you should return true if the line matches your pattern
        and increase the match count. You can also act differently if your subclass is a container
        for example. You do not need to check for the log message type, as doMatch is already
        called with the types that were specified
    \li You have to subclass infoString. Here you should return information of your matcher.
        Like which pattern it matches. You can reuse this in createErrorString
    \li Subclass createErrorString if you want to provide additional information not present in
        the default implementation, see the createErrorString documentation.
    \li subclass fullfilledConstraints if it should take care of more than the boundaries
        defined by setMatchConstraints, i.e. return true/false also in other cases
    \li subclassing setMatchConstraints is optional if you need special handling there
    \li \note do NOT call AVLog from subclasses as this could lead to an infinite loop

    \remark
    Extensions possibilities:
    There are multiple ways subclasses could extend the functionality, like adding classes acting
    as containers of different sort.
      \li Any Order.
          Here fullfilledConstraints would return true if all children's
          fullfilledConstraints return true.
          You only need to have a iterator pointing at the current
          child and a state if it matched before. This is to ensures that
          children work no matter what they look like.
          Then if a child does not match you would simply go to the next
          child.
      \li Sequence Order.
          One container could take care that fullfilledConstraints
          only returns true if all its children matched in the order
          they were added and their fullfilledConstraints returns true.
          For this to work one would only need to add an iterator which
          points at the current child matcher item and a state to know if
          the child matched before.
          If the child matched before but not now there would be two cases:
          The child fulfills the constrains --> one could go on to the next
          child and if that one does not match the container should return
          false on fullfilledConstraints. Or the child does not fulfill
          the constraints in that case fullfilledConstraints of the
          container should return false.
 */
class AVLIB_EXPORT AVLogMatcherBase
{
    AVDISABLECOPY(AVLogMatcherBase);
public:
    virtual ~AVLogMatcherBase();

    //! Set the constraints the numbers of matches have to fulfill
    /*! Here you set the boundaries the number of matches has to fulfill
        so that fullfilledConstraints returns true
        \param minMatches there need to be at least minMatches, a minMatches of <= 0
               means that there is no minimum constraint, default on construction is 1
        \param maxMatches matchcount needs to be smaller than maxMatches to fulfill the constraint,
                          a maxMatches of < 0 means that there is no maximum constraint,
                          default on construction is 1
        \note minMatches = 0, maxMatches = 0 means that the pattern should never match
        \see fullfilledConstraints
     */
    virtual void setMatchConstraints(int minMatches = 1, int maxMatches = 1);

    //! return the match count
    int matchCount() const { return m_matchCount; }

    //! returns the minimum matches constraint
    /* returns the minimum matches constraint
       \see setMatchConstraints
     */
    int minMatches() const { return m_minMatches; }

    //! returns the maximum matches constraint
    /* returns the maximum matches constraint
       \see setMatchConstraints
     */
    int maxMatches() const { return m_maxMatches; }

    //! returns true if the match constraints are fulfilled
    /*! \return true if the match constraints are fulfilled, that means
                the match count is between the set boundaries
        \see setMatchConstraints
     */
    virtual bool fullfilledConstraints() const;

    //! Match a log line
    /*! Takes line and passes it on to doMatch if the logLevel is correct
        Matcher checks if  the specified pattern (in the subclasses) matches the log line
        \param line the log line to match
        \return true if the pattern matched the log line, false otherwise
     */
    bool match(const AVLogMatcherLogLine &line);

    //! Returns all matched lines
    /*! Returns all matched lines and the pattern they matched.
        This is useful to see which lines were actually matched
     */
    QString matchedLines() const;

    //! Returns information on the pattern of this matcher
    /*! Returns information on the pattern of this matcher
     */
    virtual QString infoString() const = 0;

    //! Returns an error message
    /*! Returns an error message
        \note calling this will return an empty list if there
              was no error (fullfilledConstraints == true)
        \return the error message
        \see fullfilledConstraints
     */
    QString errorString() const;

    //! Reset the match count
    /*! Reset the match count
        Subclasses can define additional behavior
     */
    virtual void resetCount();

protected:
    //! Creates AVLogMatcherBase
    /*! Creates an AVLogMatcherBase with m_minMatches=1 and m_maxMatches=1
        Only accepts log entries of type, if type is -1 every  log entry is accepted
        \see setMatchConstraints
     */
    explicit AVLogMatcherBase(int type = -1);

    //! Creates a string containing the error message
    /*! Creates a string containing the error message
        By default infoString and getConstraintsMessage are combined.
        Subclass if you want different behaviour
        \see infoString
        \see getConstraintsMessage
     */
    virtual QString createErrorString() const;

    //! returns the constraints put into a human readable form
    /*! Constraints in human readable form
        \return a string containing the number of matches,
                the min constraint and the max constraint
     */
    QString getConstraintsMessage() const;

    //! Match a log line
    /*! Check if the pattern in the matcher matches the log input
        Matcher checks if  the specified pattern (in the subclasses) matches the log line
        \param line the log line to match
        \return true if the pattern matched the log line, false otherwise
     */
    virtual bool doMatch(const AVLogMatcherLogLine &line) = 0;

protected:
    int     m_type;         //!< type of accepted log types, -1 means all log lines are accepted
    int     m_matchCount;   //!< times of matches so far
    int     m_minMatches;   //!< minimum number of matches for fulfilling the constraint
                            //!< m_minMatches <= 0 means that there is constraint on the minimum
    int     m_maxMatches;   //!< maximum number of matches for fulfilling the constraint
                            //!< m_maxMatches < 0 means that there is constraint on the maximum
    QString m_matchedLines; //!< all lines that matched, containing the message type
};

class AVLIB_EXPORT AVLogStringMatcher : public AVLogMatcherBase
{
public:
    enum MatchingType {
        STARTS_WITH,    //!< the string shall start with the specified pattern
        ENDS_WITH,      //!< the string shall end with the specified pattern
        CONTAINS,       //!< the string shall contain the specified pattern
        EQUALS          //!< the string shall be equal to the specified pattern
    };

    explicit AVLogStringMatcher(const QString &pattern, MatchingType matchingType, int type = -1);
    ~AVLogStringMatcher() override;

    QString infoString() const override;

    //! returns the string represenation of the MatchingType
    static QString matchTypeToString(MatchingType matchingType);

protected:
    bool    doMatch(const AVLogMatcherLogLine &line) override;
    QString createErrorString() const override;

private:
    MatchingType m_matchType;    //<! type to use when matching
    QString      m_pattern; //<! pattern to match
};


//! Class for matching regular expressions
/*! This class makes it possible to match regular expressions.
    The behavior is the same as QString::contains(QRegExp)
 */
class AVLIB_EXPORT AVLogRegExpMatcher : public AVLogMatcherBase
{
public:
    //! Constructor taking the pattern to match
    explicit AVLogRegExpMatcher(const QRegExp &pattern, int type = -1);
    ~AVLogRegExpMatcher() override;

    QString infoString() const override;

protected:
    bool doMatch(const AVLogMatcherLogLine &line) override;

private:
    QRegExp m_pattern;  //<! pattern to match
};

//! Class for matching log messages of a specific type
class AVLIB_EXPORT AVLogTypeMatcher : public AVLogMatcherBase
{
public:
    //! Constructor taking the type to match
    explicit AVLogTypeMatcher(int type);
    ~AVLogTypeMatcher() override;

    void resetCount() override;

    QString infoString() const override;

protected:
    bool    doMatch(const AVLogMatcherLogLine &line) override;
    QString createErrorString() const override;

private:
    QString m_error; //!< string containing the error message
};

//! Class for matching any log message of the minimum log level
/*! Class for matching any log message of the minimum log level.
    Internally multiple AVLogTypeMatcher are used with minMatches=0 and
    maxMatches=0
    \note setMatchConstraints has no effect here
 */
class AVLIB_EXPORT AVLogNoProblemsMatcher : public AVLogMatcherBase
{
public:
    //! Constructor taking the minimum log level to match
    explicit AVLogNoProblemsMatcher(int minLevel = AVLog::LOG__WARNING);
    ~AVLogNoProblemsMatcher() override;

    bool fullfilledConstraints() const override;
    void resetCount() override;

    QString infoString() const override;

protected:
    bool    doMatch(const AVLogMatcherLogLine &line) override;
    QString createErrorString() const override;

private:
    int m_minType;                              //!< minimum log level to match
    QVector<AVLogTypeMatcher*> m_matchers;      //!< all matchers used internally
};

#endif

// End of file
