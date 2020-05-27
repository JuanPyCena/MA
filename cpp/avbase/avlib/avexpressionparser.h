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
    \brief    This file declares the expressions parser class.
*/

#ifndef AVEXPRESSIONPARSER_H
#define AVEXPRESSIONPARSER_H

// Qt includes
#include <QString>

// AviBit common includes
#include "avlib_export.h"
#include "avexpressions.h"

///////////////////////////////////////////////////////////////////////////////

//! This class implements a parser for expressions.

class AVLIB_EXPORT AVExpressionParser : public AVExpressionNamespace
{
    //We need this special wrapper around QString to allow custom sorting
    //We need custom sorting to get longer operators first (= would match before ==)
    class OperatorString {
        const QString m_operatorStr;

    public:
        OperatorString(const QString &str) : m_operatorStr(str) {}

        bool operator<(const OperatorString &other) const {
            if(m_operatorStr.count() == other.m_operatorStr.count())
                return m_operatorStr < other.m_operatorStr;
            return m_operatorStr.count() > other.m_operatorStr.count();
        }

        const QString &operatorStr() const { return m_operatorStr; }
    };

    static QMap<OperatorString, std::function<std::unique_ptr<AVExpression>(const QString &, const QString &)> > getDefaultOperators();

public:
    //! Constructor
    AVExpressionParser();
    //! Parse the specified string and answer the corresponding expression.
    //! Answer null if the expression cannot be parsed.
    std::unique_ptr<AVExpression> parseExpression(const QString& string, const QString& space=QString::null);
    //! If parseExpression fails, this method will answer an error message.
    const QString& errorMessage() { return m_errorMessage; }
    //! If parseExpression fails, this method will answer the position of the
    //! parse error.
    uint errorPos() { return m_errorPos; }
    //! If parseExpression fails, this method will insert the error message in
    //! the appropriate place in the string.
    QString formatErrorMessage(const QString& string);

    bool registerOperator(const QString &operatorStr,
                          std::function<std::unique_ptr<AVExpression>(const QString &, const QString &)> factory);

    template<typename T>
    bool registerOperator(const QString &operatorStr);

protected:
    //! Preprocess the source string, i.e. remove cr/lf
    QString preprocess(const QString& string) const;
    //! This method parses the next expression in the string.
    /*! \param string the string to be parsed
        \param space  if this string is found as a substring, it is replaced
                      with a single space character
        \param type   the type of expression that has already been parsed. Valid
                      values are Any, And, and Or. This is used to ensure that
                      And and Or expressions are not mixed without proper
                      parens.
        \param start  the first character position that should be parsed.
        \param last   this parameter will receive the last character position
                      that has been parsed.
        \result       the parsed expression, or null if a parse error occurs.
    */
    std::unique_ptr<AVExpression> parseExpression(
        const QString& string, const QString& space, Type type, uint start, uint& last);
    //! This method parses the next comparison expression (Equals, NotEquals,
    //! Matches) in the string.
    /*! \param string the string to be parsed
        \param space  if this string is found as a substring, it is replaced
                      with a single space character
        \param start  the first character position that should be parsed.
        \param last   this parameter will receive the last character position
                      that has been parsed.
        \result       the parsed expression, or null if a parse error occurs.
    */
    std::unique_ptr<AVExpression> parseComparison(
        const QString& string, const QString& space, uint start, uint len);
private:
    //! Copy-constructor: defined but not implemented
    AVExpressionParser(const AVExpressionParser& rhs);
    //! Assignment operator: defined but not implemented
    AVExpressionParser& operator=(const AVExpressionParser& rhs);

private:
    template<typename T>
    static std::unique_ptr<AVExpression> createExpression(const QString &lhs, const QString &rhs);

    QString m_andKeyword;   //! the keyword for And expressions
    QString m_orKeyword;    //! the keyword for Or expressions
    QString m_errorMessage; //! the error message for the last parse error
    uint    m_errorPos;     //! the character position of the last parse error

    QMap<OperatorString, std::function<std::unique_ptr<AVExpression>(const QString &, const QString &)> > m_operators;
};

///////////////////////////////////////////////////////////////////////////////

template<typename T>
bool AVExpressionParser::registerOperator(const QString &operatorStr)
{
    return registerOperator(operatorStr, createExpression<T>);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
std::unique_ptr<AVExpression> AVExpressionParser::createExpression(const QString &lhs, const QString &rhs)
{
    return std::make_unique<T>(lhs, rhs);
}

#endif // AVEXPRESSIONPARSER_H

// End of file
