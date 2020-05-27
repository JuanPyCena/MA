///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - AviBit Library
//
/////////////////////////////////////////////////////////////////////////////

/*! \file
    \author   Wolfgang Eder, w.eder@avibit.com
    \author   QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief    This file declares the classes to represent expressions, and the
              interface class AVComputable that can be used to evaluate these
              expressions.
*/

#ifndef AVEXPRESSIONS_H
#define AVEXPRESSIONS_H

// Qt includes
#include <QHash>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QMetaType>
#include <QMetaProperty>

// AviBit common includes
#include "avlib_export.h"
#include "avexplicitsingleton.h"
#include "avfromtostring.h"

///////////////////////////////////////////////////////////////////////////////

/*! \page avexpressions Implementing generic expressions using AVExpression

    The class AVExpression and its subclasses implement a generic framework
    for expressions. There are two kinds of expressions: expressions that can
    be evaluated and expressions that alter values (i.e. perform assignments).

    An example for an evaluate expression: Check whether the call-sign
    starts with "AUA"

    \verbatim callSign=~AUA.* \endverbatim

    An example for for an assignment expression:

    \verbatim departureClearance=true \endverbatim

    To create an AVExpression, you usually use an AVExpressionParser.

    \code
    #include "avexpressionparser.h"

    AVExpressionParser parser;
    AVExpression *expression1 =
        parser.parseExpression("callSign=~AUA.*");
    AVExpression *expression2 =
        parser.parseExpression("departureClearance=false");
    \endcode

    In order to evaluate an expression, there needs to be an object that can
    resolve the variable names (e.g. "callSign") to its value. This is done
    by the class AVComputable, which you need to subclass.

    Once you have an AVExpression instance, you can use it in the following
    ways:

    \code bool result = expression1->evaluateExpression(msg); \endcode

    In this example, the expression is used to compute a boolean result.
    The values required to compute the result are taken from a message object,
    which is an AVComputable.
    In order to compute the result, the methods valueOfLHS and valueOfRHS are
    called to resolve the names to their values. (LHS stands for left-hand-side,
    RHS stands for right-hand-side.)
    For example, the expression "callSign=~AUA.*" matches the result of calling
    valueOfLHS("callSign") and valueOfRHS("AUA.*").

    Please note that valueOfLHS tries to resolve the name, while valueOfRHS
    usually just returns its argument. If the argument to valueOfRHS starts
    with an '@' character, it uses valueOfLHS to resolve the rhs name.

    For example, the expression "callSign==@prevCallSign" calls
    valueOfLHS("callSign") and valueOfRHS("@prevCallSign"), which in turn calls
    valueOfLHS("prevCallSign"), and thus compares the value of callSign with the
    value of prevCallSign.

    The second way to use an expression is to perform assignments:

    \code bool result = expression2->performAssignments(msg); \endcode

    The expression "clearance=false" ends up calling
    performAssignments("clearance", "false") for the specified AVComputable.
    The implementation of performAssignments usually uses valueOfRHS to evaluate
    the RHS of the assignment expression.

    For evaluate expressions, the following operators can be used:

    <table>
     <tr><td>"=="</td><td>Equals operator</td></tr>
     <tr><td>"!="</td><td>NotEquals operator</td></tr>
     <tr><td>"<"</td><td>int LessThan operator</td></tr>
     <tr><td>"<="</td><td>int LessThanOrEqual operator</td></tr>
     <tr><td>">"</td><td>int GreaterThan operator</td></tr>
     <tr><td>">="</td><td>int GreaterThanOrEqual operator</td></tr>
     <tr><td>"=~"</td><td>Matches QRegExp operator</td></tr>
     <tr><td>"!~"</td><td>NotMatches operator</td></tr>
     <tr><td>" and "</td><td>And operator</td></tr>
     <tr><td>" or "</td><td>Or operator</td></tr>
    </table>

    There is no implicit precedence order for And and Or operators.
    If both And and Or operators are used in the same expression, the precedence
    must be explicitly declared using parentheses.
    Please note the surrounding spaces for the And and Or operators. If you need
    to split an expression across multiple lines, please insert line breaks
    before the " and " and " or " keywords.

    For assignment expressions, the following operators can be used:

    <table>
     <tr><td>"="</td><td>Assign operator</td></tr>
     <tr><td>"+="</td><td>Append operator</td></tr>
     <tr><td>" and "</td><td>And operator</td></tr>
    </table>

    For more examples, please visit the unit tests, or consult the DIFLIS
    configuration file "striplayoutrules.xfg".
*/

///////////////////////////////////////////////////////////////////////////////

//! This class is a wrapper for a QString that caches its hash value, and
//! implements an optimized == operator. It is designed to optimize the speed
//! of name lookups, i.e. the expression pattern (hashString == "literal").

class AVLIB_EXPORT AVHashString
{
public:
    static uint computeHash(const char *key);
    static void printStats();
public:
    //! \param name The value of the string. TODO why call this name?
    //! \param expression This is just for statistics...
    explicit AVHashString(const QString& name, bool expression=false);
    virtual ~AVHashString();
    bool operator==(const char *value) const;
    const QString& qstring() const { return m_name; }
    const char *latin1() const { return m_latin1.constData(); }
    int find(char c) const;
    const AVHashString& left(uint len) const;
    const AVHashString& mid(uint index) const;
    bool startsWith(const char *s) const { return (left(qstrlen(s)) == s); }
    //! deprecated, do not use in new code
    QString right(uint len) const { return m_name.right(len); }
    //! deprecated, do not use in new code
    bool isEmpty() const { return m_name.isEmpty(); }
    //! deprecated, do not use in new code
    uint length() const { return m_name.length(); }
    //! deprecated, do not use in new code
    bool endsWith(const char *s) const { return m_name.endsWith(s); }
    //! deprecated, do not use in new code
    QString section(const char *sep, int start, int end=0xffffffff) const
        { return m_name.section(sep, start, end); }
    //! deprecated, do not use in new code
    uint toInt(bool *ok=0, int base=10) const { return m_name.toInt(ok, base); }
    //! deprecated, do not use in new code
    uint toUInt(bool *ok=0, int base=10) const { return m_name.toUInt(ok, base); }
    //! deprecated, do not use in new code
    int findRev(const char *str, int index=-1) const { return m_name.lastIndexOf(str, index); }
    //! deprecated, do not use in new code

    const char *ascii() const { return m_latin1.constData(); }

    //! deprecated, do not use in new code
    int compare(const QString& s) const { return m_name.compare(s); }
private:
    //! Copy-constructor: defined but not implemented
    AVHashString(const AVHashString& rhs);
    //! Assignment operator: defined but not implemented
    AVHashString& operator=(const AVHashString& rhs);
private:
    //! This dictionary is populated with all valid names (const char * literals).
    //! The key is the name itself and the value is its hash, i.e. computeHash(name).
    typedef QHash<void*, uint> ValueHashes;
    static std::unique_ptr<ValueHashes> m_valueHashes;
    static uint m_instances;            // total number of AVHashString instances
    static uint m_expressionInstances;  // number of AVHashString instances cached in AVExpressions
    static uint m_leftDicts;            // number of m_left dictionary instances
    static uint m_midDicts;             // number of m_mid dictionary instances
    static uint m_valueHashHit;         // value hash found in m_valueHashes
    static uint m_valueHashMiss;        // value hash not found, and thus added, to m_valueHashes
    static uint m_hashHit;              // name != value
    static uint m_hashMiss;             // name hash == value hash
    static uint m_hashClash;            // name hash == value hash, but name != value
private:
    QString      m_name;
    bool         m_expression;
    QByteArray   m_latin1;
    uint         m_nameHash;
    typedef QHash<uint, AVHashString*> LeftMidDict;
    mutable LeftMidDict *m_left;
    mutable LeftMidDict *m_mid;
};

template<> QString AVLIB_EXPORT AVToString(const AVHashString& arg, bool enable_escape);

///////////////////////////////////////////////////////////////////////////////

//! This interface class can be used to evaluate expressions.
/*! To use it, you need an instance of AVExpression, which can be obtained
    from an AVExpressionParser.
*/

class AVLIB_EXPORT AVComputable
{
public:
    //! This method converts a bool to a string. This method should be used to
    //! ensure that bool values can be used consistently within AVRules.
    static QString boolStr(bool b) { return (b ? m_trueString : m_falseString); }
    //! This method converts a string to a bool. This method should be used to
    //! ensure that bool values can be used consistently within AVRules.
    static bool asBool(const QString& string);
public:
    //! Constructor
    AVComputable() { }
    //! Destructor
    virtual ~AVComputable() { }
    //! Answer the value of the left-hand-side of an expression
    virtual QString valueOfLHS(const AVHashString& lhs) const = 0;
    //! Answer the list value. This is currently used to evalueate
    //! rules with a <forEach> tag containing a "@lhs" expression
    virtual QStringList listValueOfLHS(const QString& lhs) const;
    //! Answer the value of the right-hand-side of an expression
    //! The default implementation answers valueOfLHS if the rhs starts with
    //! '@', or rhs itself otherwise. You should rarely need to override this
    //! method.
    virtual QString valueOfRHS(const AVHashString& rhs) const;
    //! Perform an assignment by calling assignValue(lhs, valueOfRHS(rhs)).
    //! Answers whether the assignment was performed successfully.
    bool performAssignment(const AVHashString& lhs, const AVHashString& rhs);
    //! Perform an append assignment by calling assignValue(lhs, valueOfLHS(lhs) + valueOfRHS(rhs)).
    //! Answers whether the assignment was performed successfully.
    bool performAppendAssignment(const AVHashString& lhs, const AVHashString& rhs);
    //! Perform an assignment. Answer whether the assignment was
    //! performed successfully.
    virtual bool assignValue(const AVHashString& lhs, const QString& value) = 0;
    //! Hook for special treatment of AVEqualsExpression
    virtual bool handleEvaluateEquals(const AVHashString& lhs, const AVHashString& rhs, bool& result) const;
private:
    //! Copy-constructor: defined but not implemented
    AVComputable(const AVComputable& rhs);
    //! Assignment operator: defined but not implemented
    AVComputable& operator=(const AVComputable& rhs);
private:
    static const QString m_trueString;
    static const QString m_falseString;
};

Q_DECLARE_METATYPE(AVComputable*)
Q_DECLARE_METATYPE(const AVComputable*)

///////////////////////////////////////////////////////////////////////////////

//! This computable base class can be used to simplify getting/setting values
//! All scriptable Q_PROPERTIES can be used, even with . (point) syntax.
//!
//! When reading values (valueOfLHS) all values are converted to a QVariant (part
//! of the Qt property system). Later they are converted to string with QVariant::toString()
//! So ints, booleans, QStrings, QByteArray, ... are all supported!
//!
//! To implement the . (point) syntax: Properties with type AVComputable* are handled differently:
//! Everything after . will be forwarded to the new (nested) computable

class AVLIB_EXPORT AVPropertyComputable : public AVComputable
{
public:
    explicit AVPropertyComputable();
    virtual ~AVPropertyComputable() override;

    // must be implemented in every subclass
    // Q_OBJECT macro implements this correctly already
    // just return &staticMetaObject when using Q_GADGET
    virtual const QMetaObject *metaObject() const = 0;

    //! Answer the value of the left-hand-side of an expression
    QString valueOfLHS(const AVHashString& lhs) const override;

    //! Perform an assignment. Answer whether the assignment was
    //! performed successfully.
    bool assignValue(const AVHashString& lhs, const QString& value) override;

protected:
    QMetaProperty getMetaProperty(const AVHashString& name) const;
    QMetaProperty getReadableMetaProperty(const AVHashString& name) const;
    QMetaProperty getWriteableMetaProperty(const AVHashString& name) const;
    QVariant readProperty(const QMetaProperty &metaProperty) const;
    bool writeProperty(const QMetaProperty &metaProperty, const QVariant &value);
};

///////////////////////////////////////////////////////////////////////////////

//! This class encapsulates the Type enum

class AVLIB_EXPORT AVExpressionNamespace
{
public:
    //! Constructor
    AVExpressionNamespace() { }
    //! Destructor
    virtual ~AVExpressionNamespace() { }
    //! The Type enum lists all types of expressions. "Any" can be used to match
    //! all types.
    enum Type {
        Any, And, Or, Equals, NotEquals, LessThan, LessThanOrEqual, GreaterThan, GreaterThanOrEqual,
        Matches, NotMatches, Assign, Append };
    //! The ForEachMap maps loop variable names (usually just one, "each") to variable values.
    typedef QMap<QString, QString> ForEachMap;
protected:
    bool containsEach(const AVHashString& string, const ForEachMap& forEachMap) const;
    QString replaceEach(const QString& string, const ForEachMap& forEachMap) const;
    QString replaceEach(const AVHashString& string, const ForEachMap& forEachMap) const
        { return replaceEach(string.qstring(), forEachMap); }
private:
    //! Copy-constructor: defined but not implemented
    AVExpressionNamespace(const AVExpressionNamespace& rhs);
    //! Assignment operator: defined but not implemented
    AVExpressionNamespace& operator=(const AVExpressionNamespace& rhs);
};

///////////////////////////////////////////////////////////////////////////////

//! This is the abstract base class for all expressions. Expressions can be
//! evaluated (e.g. "callSign=~AUA.*"), or to perform assignment
//! (e.g. "clearance=false"). Only And and Assign expressions can be used to
//! perform assignment.

class AVLIB_EXPORT AVExpression : public AVExpressionNamespace
{
public:
    //! Constructor
    AVExpression() { }
    //! Answer the type of expression
    virtual Type type() const = 0;
    //! Answer a string representation for the expression.
    virtual QString toString() const = 0;
    //! Answer a hierarchical string representation for the expression.
    virtual QString debugString(int indent=0) const = 0;
    //! Answer whether the expression can be used to perform assignment.
    virtual bool isEvaluateExpression() const { return true; }
    //! Answer whether the expression can be used to perform assignment.
    virtual bool isAssignmentExpression() const { return false; }
    //! Evaluate the expression.
    /*! Please note that And and Or expressions are evaluated lazily,
        i.e. the rhs is not evaluated if the lhs already determines the result.
    */
    virtual bool evaluateExpression(const AVComputable& c, const ForEachMap& forEachMap) const = 0;
    //! Perform all assignments of the expression. Answer true if all
    //! assignments were performed successfully, false otherwise.
    /*! Please note that all assignments are tried to be performed even if some
        of them answer false. In other words, there is no lazy evaluation as
        opposed to the method evaluateExpression.
    */
    virtual bool performAssignments(AVComputable& c, const ForEachMap& forEachMap) const;
    //! Answer a string representation of the lhs
    virtual QString lhsString() const = 0;
    //! Answer a string representation of the rhs
    virtual QString rhsString() const = 0;

    //! What should happen in case of misconfigured rules?
    enum FAILURE_MODE
    {
        FM_IGNORE,
        FM_WARNING,
        FM_FATAL
    };
    static void setFailureMode(FAILURE_MODE mode);
    static FAILURE_MODE getFailureMode();
    static AVLogStream getFailureLogstream();
    //! Performs string comparison to emulate qt3 behaviour, added to fix DIFLIS-5327
    //! Probably not necessary as the null string is used to indicate unknown values
    //! TODO Cleanup occurances of empty string and null string comparison and remove this
    static bool qstringEqual(const QString &s1, const QString &s2);
    static bool qstringEqual(const QString &s1, const QLatin1String &s2);

protected:
    static const QString& debugArgumentString(const QString& arg);
    static const QString& debugArgumentString(const AVHashString& arg)
    { return debugArgumentString(arg.qstring()); }

    static FAILURE_MODE m_failureMode;
private:
    //! Copy-constructor: defined but not implemented
    AVExpression(const AVExpression& rhs);
    //! Assignment operator: defined but not implemented
    AVExpression& operator=(const AVExpression& rhs);
};

///////////////////////////////////////////////////////////////////////////////

//! This class represents an And expression.

class AVLIB_EXPORT AVAndExpression : public AVExpression
{
public:
    //! Constructor
    AVAndExpression(std::unique_ptr<AVExpression> lhs, std::unique_ptr<AVExpression> rhs);
    //! Destructor
    ~AVAndExpression() override;
    //! Answer the type of expression
    Type type() const override { return And; }
    //! Answer a string representation for the expression.
    QString toString() const override;
    //! Answer a hierarchical string representation for the expression.
    QString debugString(int indent = 0) const override;
    //! Answer whether the expression can be used to perform assignment.
    bool isAssignmentExpression() const override { return true; }
    //! Evaluate the expression.
    bool evaluateExpression(const AVComputable& c, const ForEachMap& forEachMap) const override;
    //! Perform all assignments of the expression.
    bool performAssignments(AVComputable& c, const ForEachMap& forEachMap) const override;
    //! Answer a string representation of the lhs
    QString lhsString() const override { return m_lhs->toString(); }
    //! Answer a string representation of the rhs
    QString rhsString() const override { return m_rhs->toString(); }
    //! access to lhs expression
    AVExpression *lhs() const { return m_lhs.get(); }
    //! access to rhs expression
    AVExpression *rhs() const { return m_rhs.get(); }
private:
    //! Copy-constructor: defined but not implemented
    AVAndExpression(const AVAndExpression& rhs);
    //! Assignment operator: defined but not implemented
    AVAndExpression& operator=(const AVAndExpression& rhs);
private:
    std::unique_ptr<AVExpression> m_lhs; //! the expression left-hand-side
    std::unique_ptr<AVExpression> m_rhs; //! the expression right-hand-side
};

///////////////////////////////////////////////////////////////////////////////

//! This class represents an Or expression.

class AVLIB_EXPORT AVOrExpression : public AVExpression
{
public:
    //! Constructor
    AVOrExpression(std::unique_ptr<AVExpression> lhs, std::unique_ptr<AVExpression> rhs);
    //! Destructor
    ~AVOrExpression() override;
    //! Answer the type of expression
    Type type() const override { return Or; }
    //! Answer a string representation for the expression.
    QString toString() const override;
    //! Answer a hierarchical string representation for the expression.
    QString debugString(int indent = 0) const override;
    //! Evaluate the expression.
    bool evaluateExpression(const AVComputable& c, const ForEachMap& forEachMap) const override;
    //! Answer a string representation of the lhs
    QString lhsString() const override { return m_lhs->toString(); }
    //! Answer a string representation of the rhs
    QString rhsString() const override { return m_rhs->toString(); }
    //! access to lhs expression
    AVExpression *lhs() const { return m_lhs.get(); }
    //! access to rhs expression
    AVExpression *rhs() const { return m_rhs.get(); }
private:
    //! Copy-constructor: defined but not implemented
    AVOrExpression(const AVOrExpression& rhs);
    //! Assignment operator: defined but not implemented
    AVOrExpression& operator=(const AVOrExpression& rhs);
private:
    std::unique_ptr<AVExpression> m_lhs; //! the expression left-hand-side
    std::unique_ptr<AVExpression> m_rhs; //! the expression right-hand-side
};

///////////////////////////////////////////////////////////////////////////////

//! This class represents an Equals expression.

class AVLIB_EXPORT AVEqualsExpression : public AVExpression
{
public:
    //! Constructor
    AVEqualsExpression(const QString& lhs, const QString& rhs)
        : m_lhs(lhs, true), m_rhs(rhs, true), m_first(true), m_needToReplaceEach(true) { }
    //! Answer the type of expression
    Type type() const override { return Equals; }
    //! Answer a string representation for the expression.
    QString toString() const override;
    //! Answer a hierarchical string representation for the expression.
    QString debugString(int indent = 0) const override;
    //! Evaluate the expression.
    bool evaluateExpression(const AVComputable& c, const ForEachMap& forEachMap) const override;
    //! Answer a string representation of the lhs
    QString lhsString() const override { return m_lhs.qstring(); }
    //! Answer a string representation of the rhs
    QString rhsString() const override { return m_rhs.qstring(); }

private:
    //! Copy-constructor: defined but not implemented
    AVEqualsExpression(const AVEqualsExpression& rhs);
    //! Assignment operator: defined but not implemented
    AVEqualsExpression& operator=(const AVEqualsExpression& rhs);
private:
    AVHashString m_lhs; //! the expression left-hand-side
    AVHashString m_rhs; //! the expression right-hand-side
    mutable bool m_first;
    mutable bool m_needToReplaceEach;
};

///////////////////////////////////////////////////////////////////////////////

//! This class represents a NotEquals expression.

class AVLIB_EXPORT AVNotEqualsExpression : public AVExpression
{
public:
    //! Constructor
    AVNotEqualsExpression(const QString& lhs, const QString& rhs)
        : m_lhs(lhs, true), m_rhs(rhs, true), m_first(true), m_needToReplaceEach(true) { }
    //! Answer the type of expression
    Type type() const override { return NotEquals; }
    //! Answer a string representation for the expression.
    QString toString() const override;
    //! Answer a hierarchical string representation for the expression.
    QString debugString(int indent = 0) const override;
    //! Evaluate the expression.
    bool evaluateExpression(const AVComputable& c, const ForEachMap& forEachMap) const override;
    //! Answer a string representation of the lhs
    QString lhsString() const override { return m_lhs.qstring(); }
    //! Answer a string representation of the rhs
    QString rhsString() const override { return m_rhs.qstring(); }

private:
    //! Copy-constructor: defined but not implemented
    AVNotEqualsExpression(const AVNotEqualsExpression& rhs);
    //! Assignment operator: defined but not implemented
    AVNotEqualsExpression& operator=(const AVNotEqualsExpression& rhs);
private:
    AVHashString m_lhs; //! the expression left-hand-side
    AVHashString m_rhs; //! the expression right-hand-side
    mutable bool m_first;
    mutable bool m_needToReplaceEach;
};

///////////////////////////////////////////////////////////////////////////////

//! This class represents an integer comparison expression.

class AVLIB_EXPORT AVIntExpression : public AVExpression
{
public:
    //! Constructor
    AVIntExpression(const QString& lhs, const QString& rhs)
        : m_lhs(lhs, true), m_rhs(rhs, true) { }
    //! Evaluate the expression.
    bool evaluateExpression(const AVComputable& c, const ForEachMap& forEachMap) const override;
    //! Answer a string representation of the lhs
    QString lhsString() const override { return m_lhs.qstring(); }
    //! Answer a string representation of the rhs
    QString rhsString() const override { return m_rhs.qstring(); }

protected:
    //! Evaluate the expression.
    virtual bool evaluateIntExpression(int lhs, int rhs) const = 0;
private:
    //! Copy-constructor: defined but not implemented
    AVIntExpression(const AVIntExpression& rhs);
    //! Assignment operator: defined but not implemented
    AVIntExpression& operator=(const AVIntExpression& rhs);
protected:
    AVHashString m_lhs; //! the expression left-hand-side
    AVHashString m_rhs; //! the expression right-hand-side
};

///////////////////////////////////////////////////////////////////////////////

//! This class represents an LessThan expression.

class AVLIB_EXPORT AVLessThanExpression : public AVIntExpression
{
public:
    //! Constructor
    AVLessThanExpression(const QString& lhs, const QString& rhs)
        : AVIntExpression(lhs, rhs) { }
    //! Answer the type of expression
    Type type() const override { return LessThan; }
    //! Answer a string representation for the expression.
    QString toString() const override;
    //! Answer a hierarchical string representation for the expression.
    QString debugString(int indent = 0) const override;

protected:
    //! Evaluate the expression.
    bool evaluateIntExpression(int lhs, int rhs) const override;

private:
    //! Copy-constructor: defined but not implemented
    AVLessThanExpression(const AVLessThanExpression& rhs);
    //! Assignment operator: defined but not implemented
    AVLessThanExpression& operator=(const AVLessThanExpression& rhs);
};

///////////////////////////////////////////////////////////////////////////////

//! This class represents an LessThanOrEqual expression.

class AVLIB_EXPORT AVLessThanOrEqualExpression : public AVIntExpression
{
public:
    //! Constructor
    AVLessThanOrEqualExpression(const QString& lhs, const QString& rhs)
        : AVIntExpression(lhs, rhs) { }
    //! Answer the type of expression
    Type type() const override { return LessThanOrEqual; }
    //! Answer a string representation for the expression.
    QString toString() const override;
    //! Answer a hierarchical string representation for the expression.
    QString debugString(int indent = 0) const override;

protected:
    //! Evaluate the expression.
    bool evaluateIntExpression(int lhs, int rhs) const override;

private:
    //! Copy-constructor: defined but not implemented
    AVLessThanOrEqualExpression(const AVLessThanOrEqualExpression& rhs);
    //! Assignment operator: defined but not implemented
    AVLessThanOrEqualExpression& operator=(const AVLessThanOrEqualExpression& rhs);
};

///////////////////////////////////////////////////////////////////////////////

//! This class represents an GreaterThan expression.

class AVLIB_EXPORT AVGreaterThanExpression : public AVIntExpression
{
public:
    //! Constructor
    AVGreaterThanExpression(const QString& lhs, const QString& rhs)
        : AVIntExpression(lhs, rhs) { }
    //! Answer the type of expression
    Type type() const override { return GreaterThan; }
    //! Answer a string representation for the expression.
    QString toString() const override;
    //! Answer a hierarchical string representation for the expression.
    QString debugString(int indent = 0) const override;

protected:
    //! Evaluate the expression.
    bool evaluateIntExpression(int lhs, int rhs) const override;

private:
    //! Copy-constructor: defined but not implemented
    AVGreaterThanExpression(const AVGreaterThanExpression& rhs);
    //! Assignment operator: defined but not implemented
    AVGreaterThanExpression& operator=(const AVGreaterThanExpression& rhs);
};

///////////////////////////////////////////////////////////////////////////////

//! This class represents an GreaterThanOrEqual expression.

class AVLIB_EXPORT AVGreaterThanOrEqualExpression : public AVIntExpression
{
public:
    //! Constructor
    AVGreaterThanOrEqualExpression(const QString& lhs, const QString& rhs)
        : AVIntExpression(lhs, rhs) { }
    //! Answer the type of expression
    Type type() const override { return GreaterThanOrEqual; }
    //! Answer a string representation for the expression.
    QString toString() const override;
    //! Answer a hierarchical string representation for the expression.
    QString debugString(int indent = 0) const override;

protected:
    //! Evaluate the expression.
    bool evaluateIntExpression(int lhs, int rhs) const override;

private:
    //! Copy-constructor: defined but not implemented
    AVGreaterThanOrEqualExpression(const AVGreaterThanOrEqualExpression& rhs);
    //! Assignment operator: defined but not implemented
    AVGreaterThanOrEqualExpression& operator=(const AVGreaterThanOrEqualExpression& rhs);
};

///////////////////////////////////////////////////////////////////////////////

//! This class represents a Matches expression.

class AVLIB_EXPORT AVMatchesExpression : public AVExpression
{
public:
    //! Constructor
    AVMatchesExpression(const QString& lhs, const QString& rhs)
        : m_lhs(lhs, true), m_rhs(rhs, true) { }
    //! Answer the type of expression
    Type type() const override { return Matches; }
    //! Answer a string representation for the expression.
    QString toString() const override;
    //! Answer a hierarchical string representation for the expression.
    QString debugString(int indent = 0) const override;
    //! Evaluate the expression.
    bool evaluateExpression(const AVComputable& c, const ForEachMap& forEachMap) const override;
    //! Answer a string representation of the lhs
    QString lhsString() const override { return m_lhs.qstring(); }
    //! Answer a string representation of the rhs
    QString rhsString() const override { return m_rhs.qstring(); }

private:
    //! Copy-constructor: defined but not implemented
    AVMatchesExpression(const AVMatchesExpression& rhs);
    //! Assignment operator: defined but not implemented
    AVMatchesExpression& operator=(const AVMatchesExpression& rhs);
private:
    AVHashString m_lhs; //! the expression left-hand-side
    AVHashString m_rhs; //! the expression right-hand-side
};

///////////////////////////////////////////////////////////////////////////////

//! This class represents a NotMatches expression.

class AVLIB_EXPORT AVNotMatchesExpression : public AVExpression
{
public:
    //! Constructor
    AVNotMatchesExpression(const QString& lhs, const QString& rhs)
        : m_lhs(lhs, true), m_rhs(rhs, true) { }
    //! Answer the type of expression
    Type type() const override { return NotMatches; }
    //! Answer a string representation for the expression.
    QString toString() const override;
    //! Answer a hierarchical string representation for the expression.
    QString debugString(int indent = 0) const override;
    //! Evaluate the expression.
    bool evaluateExpression(const AVComputable& c, const ForEachMap& forEachMap) const override;
    //! Answer a string representation of the lhs
    QString lhsString() const override { return m_lhs.qstring(); }
    //! Answer a string representation of the rhs
    QString rhsString() const override { return m_rhs.qstring(); }

private:
    //! Copy-constructor: defined but not implemented
    AVNotMatchesExpression(const AVNotMatchesExpression& rhs);
    //! Assignment operator: defined but not implemented
    AVNotMatchesExpression& operator=(const AVNotMatchesExpression& rhs);
private:
    AVHashString m_lhs; //! the expression left-hand-side
    AVHashString m_rhs; //! the expression right-hand-side
};

///////////////////////////////////////////////////////////////////////////////

//! This class represents an Assign expression.

class AVLIB_EXPORT AVAssignExpression : public AVExpression
{
public:
    //! Constructor
    AVAssignExpression(const QString& lhs, const QString& rhs)
        : m_lhs(lhs, true), m_rhs(rhs, true), m_first(true), m_needToReplaceEach(true) { }
    //! Answer the type of expression
    Type type() const override { return Assign; }
    //! Answer a string representation for the expression.
    QString toString() const override;
    //! Answer a hierarchical string representation for the expression.
    QString debugString(int indent = 0) const override;
    //! Answer whether the expression can be used to perform assignment.
    bool isEvaluateExpression() const override { return false; }
    //! Answer whether the expression can be used to perform assignment.
    bool isAssignmentExpression() const override { return true; }
    //! Evaluate the expression.
    bool evaluateExpression(const AVComputable& c, const ForEachMap& forEachMap) const override;
    //! Perform all assignments of the expression.
    bool performAssignments(AVComputable& c, const ForEachMap& forEachMap) const override;
    //! Answer a string representation of the lhs
    QString lhsString() const override { return m_lhs.qstring(); }
    //! Answer a string representation of the rhs
    QString rhsString() const override { return m_rhs.qstring(); }

private:
    //! Copy-constructor: defined but not implemented
    AVAssignExpression(const AVAssignExpression& rhs);
    //! Assignment operator: defined but not implemented
    AVAssignExpression& operator=(const AVAssignExpression& rhs);
private:
    AVHashString m_lhs; //! the expression left-hand-side
    AVHashString m_rhs; //! the expression right-hand-side
    mutable bool m_first;
    mutable bool m_needToReplaceEach;
};

///////////////////////////////////////////////////////////////////////////////

//! This class represents an Append expression.

class AVLIB_EXPORT AVAppendExpression : public AVExpression
{
public:
    //! Constructor
    AVAppendExpression(const QString& lhs, const QString& rhs)
        : m_lhs(lhs, true), m_rhs(rhs, true), m_first(true), m_needToReplaceEach(true) { }
    //! Answer the type of expression
    Type type() const override { return Append; }
    //! Answer a string representation for the expression.
    QString toString() const override;
    //! Answer a hierarchical string representation for the expression.
    QString debugString(int indent = 0) const override;
    //! Answer whether the expression can be used to perform assignment.
    bool isEvaluateExpression() const override { return false; }
    //! Answer whether the expression can be used to perform assignment.
    bool isAssignmentExpression() const override { return true; }
    //! Evaluate the expression.
    bool evaluateExpression(const AVComputable& c, const ForEachMap& forEachMap) const override;
    //! Perform all assignments of the expression.
    bool performAssignments(AVComputable& c, const ForEachMap& forEachMap) const override;
    //! Answer a string representation of the lhs
    QString lhsString() const override { return m_lhs.qstring(); }
    //! Answer a string representation of the rhs
    QString rhsString() const override { return m_rhs.qstring(); }

private:
    //! Copy-constructor: defined but not implemented
    AVAppendExpression(const AVAppendExpression& rhs);
    //! Assignment operator: defined but not implemented
    AVAppendExpression& operator=(const AVAppendExpression& rhs);
private:
    AVHashString m_lhs; //! the expression left-hand-side
    AVHashString m_rhs; //! the expression right-hand-side
    mutable bool m_first;
    mutable bool m_needToReplaceEach;
};

#endif // AVEXPRESSIONS_H

// End of file
