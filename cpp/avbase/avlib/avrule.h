///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// Copyright:  AviBit data processing GmbH, 2001-2010
// QT-Version: QT4
//
// Module:     AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author   Wolfgang Eder, w.eder@avibit.com
    \author   Alexander Terbu, a.terbu@avibit.com
    \author   QT4-PORT: Peter Draxler, p.draxler@avibit.com
    \brief    This file declares the class that represents a configuration rule.
*/

#ifndef AVRULE_H
#define AVRULE_H

// Qt includes
#include <QMap>

// AviBit common includes
#include "avlib_export.h"
#include "avdomvariables.h"
#include "avexpressions.h"
#include "avexpressionparser.h"

class AVLogComputable;
class AVRule;

///////////////////////////////////////////////////////////////////////////////

/*! \page avrules Implementing configurable rules using AVRule

    The class AVRule can be used to implement configurable program logic.
    It can be read from an XML file (see \ref avdomvariables here for details.)
    It uses expressions to express the individual parts of each rule (see
    \ref avexpressions here for details.)

    In its simplest form, a rule looks like this:

    \verbatim
    <Rule>
     <if>selected==true</if>
     <then>backgroundColor=blue</then>
    </Rule>
    \endverbatim

    The code to evaluate the rule looks like this:

    \code
    AVRule rule(0, "Rule", true);
    rule.readFromString("...xml string shown above...");
    bool result = rule.applyRule(msg);
    \endcode

    The rule needs an AVComputable to evaluate its expressions;
    in this case, a message object is used. The result of the applyRule message
    indicates whether all rules have been evaluated properly.

    Rules can also have an else part, and they can be nested arbitrarily:

    \verbatim
    <Rule>
     <if>hasAlarm==true</if>
     <then>backgroundColor=brightRed and flashing=true</then>
     <elseIf>
      <Rule>
       <if>selected==true</if>
       <then>backgroundColor=blue</then>
       <else>backgroundColor=white</then>
      </Rule>
     </elseIf>
    </Rule>
    \endverbatim

    Whitespace is stripped from individual expressions to improve maintainability
    for large rule files that are edited manually. If spaces are explicitly required
    in a rule, an arbitrary keyword can be defined to represent a single space.
    "SPACE" and "_" are reasonable choices as a keyword:

    \verbatim
    <Rule>
     <if space="SPACE">stand==SPACE</if>
     <then>stand=</then>
    </Rule>
    \endverbatim

    There are special keywords to support log output from within the rules:

    \verbatim
    <Rule>
     <if>hasAlarm==true</if>
     <then>logwarning=Alarm for call-sign %1 and arg=@callSign</then>
    </Rule>
    \endverbatim

    The LOGGER_AVCOMMON_AVLIB is used write the log output.
    The following special keywords are evaluated by the rule itself, and not
    delegated to the AVComputable:

    <table>
     <tr><td>"alwaysTrue"</td><td>always evaluates to "true"</td></tr>
     <tr><td>"logfatal"</td><td>Outputs a LOG_FATAL message</td></tr>
     <tr><td>"logerror"</td><td>Outputs a LOG_ERROR message</td></tr>
     <tr><td>"logwarning"</td><td>Outputs a LOG_WARNING message</td></tr>
     <tr><td>"loginfo"</td><td>Outputs a LOG_INFO message</td></tr>
     <tr><td>"logdebug"</td><td>Outputs a LOG_DEBUG message</td></tr>
     <tr><td>"arg"</td><td>Calls QString::arg on the log message</td></tr>
    </table>

    A special "forEach" tag can be used to simplify repetitive rules:

    \verbatim
    <Rule>
     <forEach>CALLSIGN,WTC,STAND</forEach>
     <if>selectedField==each</if>
     <then>each.backgroundColor=blue</then>
    </Rule>
    <Rule>
     <forEach sep=";">1;2;3</forEach>
     <if>mark==each</if>
     <then>CALLSIGN.backgroundColor=markeach</then>
    </Rule>
    \endverbatim

    If the "forEach" tag is present, the rule is evaluated for each element in
    the string list, and the string "each" is replaced with the element in all
    expressions of the rule. It is possible to specify another "forEach" tag
    in a nested ("thenIf" or "elseIf") rule by using different forEachNames.
*/

///////////////////////////////////////////////////////////////////////////////

//! This class implements a list of AVRules.

class AVLIB_EXPORT AVRuleList : public AVDomList<AVRule>, public AVExpressionNamespace
{
public:
    AVRuleList(AVDomVariable *parent, const QString& tagName, bool autoCreate, const QString& elementTagName);
    //! \return Whether the rules were successfully applied.
    bool applyRules(AVComputable& c) const;
    bool applyRulesWithLog(AVLogComputable& c, const ForEachMap& forEachMap) const;
};

///////////////////////////////////////////////////////////////////////////////

class AVLIB_EXPORT ForEachList : public AVDomStringListSep
{
    Q_OBJECT
public:
    //! Construct a new instance, and add it to its parent's list of variables
    //!
    //! *ATTENTION* Note that the default separator here is " ", but is specified as ","
    //!             in AVRule::AVRule().
    ForEachList(
        AVDomVariable *parent, const QString& tagName, bool autoCreate, const QString& sep=" ",
        const QString& eachName="each");
    const QString& eachName() const { return m_eachName; }
    void setEachName(const QString& name);
protected:
    //! Compute the QStringList value from the specified text.
    void computeValue(const QString& text, QDomElement& element) override;

private:
    //! Copy-constructor: defined but not implemented
    ForEachList(const ForEachList& rhs);
    //! Assignment operator: defined but not implemented
    ForEachList& operator=(const ForEachList& rhs);
private:
    QString m_eachName; //!< the variable name
};

///////////////////////////////////////////////////////////////////////////////

//! This class implements a configurable rule. For details about using this
//! class, see \ref avrules.

class AVLIB_EXPORT AVRule : public AVDomObject, public AVExpressionNamespace
{
    Q_OBJECT
public:
    AVRule(AVDomVariable *parent, const QString& tagName, bool autoCreate);
    ~AVRule() override;
    bool isIfValid() const;
    bool isThenValid() const;
    bool isElseValid() const;
    bool applyRule(AVComputable& c) const;
    bool applyRuleWithLog(AVLogComputable& c, const ForEachMap& forEachMap) const;
protected:
    //! Handles forEach expressions by calling applyRuleWithLog(const QString& value,...)
    //! with each value from the list.
    bool applyRuleWithLog(
        const QStringList& list, AVLogComputable& c, const ForEachMap& forEachMap) const;
    /**
     * @brief applyRuleWithLog is called for each value in a forEach expression.
     *
     * @param value This can be a plaintext value, or can expand into a value list
     *              (either via @config to a QStringlist parameter, or via @<computable> to a
     *              list provided by the computable's listValueOfLHS implementation).
     *
     * @param c     The computable we are working on.
     * @param forEachMap The current name->value map for enclosing forEach expressions
     * @return      True if successful, false if an error occured.
     */
    bool applyRuleWithLog(
        const QString& value, AVLogComputable& c, const ForEachMap& forEachMap) const;
    bool doApplyRule(AVLogComputable& c, const ForEachMap& forEachMap) const;
    bool doApplyRule(const char *thenOrElse, const QString& expressionString, const AVExpression *expression,
        const AVRuleList &nestedRules, AVLogComputable& c, const ForEachMap& forEachMap) const;
protected slots:
    void slotComputeIfExpression();
    void slotComputeThenExpression();
    void slotComputeElseExpression();
private:
    //! Copy-constructor: defined but not implemented
    AVRule(const AVRule& rhs);
    //! Assignment operator: defined but not implemented
    AVRule& operator=(const AVRule& rhs);
public:
    AVExpressionParser& getParser();

    AVDomString description;
    ForEachList forEachList;
    AVDomString ifString;
    AVDomString thenString;
    AVDomString elseString;
    AVRuleList  thenRules;
    AVRuleList  elseRules;
private:
    std::unique_ptr<AVExpression> m_ifExpression;
    std::unique_ptr<AVExpression> m_thenExpression;
    std::unique_ptr<AVExpression> m_elseExpression;
};

///////////////////////////////////////////////////////////////////////////////

//! This is a wrapper class for an AVComputable. It understands the following
//! LHS expressions: logfatal, logerror, logwarning, loginfo, logdebug, arg.
//! All other expressions are forwarded to the wrapped AVComputable.

class AVLIB_EXPORT AVLogComputable : public AVComputable
{
public:
    explicit AVLogComputable(AVComputable& computable);
    void clearLogMessage();
    void writeLogMessage();
    void initializeTemps(const QStringList& tempNames);
    void addTemps(const QStringList& tempNames);
    const QString& logMessage() const { return m_logMessage; }
    //! Answer the value of the left-hand-side of an expression
    QString valueOfLHS(const AVHashString& lhs) const override;
    //! Answer the list value. This is currently used to evalueate
    //! rules with a <forEach> tag containing a "@lhs" expression
    QStringList listValueOfLHS(const QString& lhs) const override;
    //! Answer the value of the right-hand-side of an expression
    //! The default implementation answers valueOfLHS if the rhs starts with
    //! '@', or rhs itself otherwise.
    QString valueOfRHS(const AVHashString& rhs) const override;
    //! Perform an assignment. Answer whether the assignment was
    //! performed successfully.
    bool assignValue(const AVHashString& lhs, const QString& value) override;
    //! Hook for special treatment of AVEqualsExpression
    bool handleEvaluateEquals(const AVHashString& lhs, const AVHashString& rhs, bool& result) const override;

    AVComputable& computable();
    const AVComputable& computable() const;

private:
    //! Copy-constructor: defined but not implemented
    AVLogComputable(const AVLogComputable& rhs);
    //! Assignment operator: defined but not implemented
    AVLogComputable& operator=(const AVLogComputable& rhs);
public:
    bool debugRules;
private:
    AVComputable& m_computable;
    int           m_logLevel;
    QString       m_logMessage;
    typedef QMap<QString, QString> Temps;
    Temps         m_temps;
};

#endif // AVRULE_H

// End of file
