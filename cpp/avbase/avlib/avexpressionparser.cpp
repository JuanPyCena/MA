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
    \brief    This file defines the expressions parser class.
*/


// AviBit common includes
#include "avexpressionparser.h"
#include "avlog.h"
#include "avmisc.h"

QMap<AVExpressionParser::OperatorString, std::function<std::unique_ptr<AVExpression>(const QString &, const QString &)> > AVExpressionParser::getDefaultOperators() {
    static QMap<AVExpressionParser::OperatorString, std::function<std::unique_ptr<AVExpression>(const QString &, const QString &)> > map {
      { QStringLiteral("=="), createExpression<AVEqualsExpression> },
      { QStringLiteral("!="), createExpression<AVNotEqualsExpression> },
      { QStringLiteral("<="), createExpression<AVLessThanOrEqualExpression> },
      { QStringLiteral(">="), createExpression<AVGreaterThanOrEqualExpression> },
      { QStringLiteral("=~"), createExpression<AVMatchesExpression> },
      { QStringLiteral("!~"), createExpression<AVNotMatchesExpression> },
      { QStringLiteral("+="), createExpression<AVAppendExpression> },
      { QStringLiteral("="), createExpression<AVAssignExpression> },
      { QStringLiteral("<"), createExpression<AVLessThanExpression> },
      { QStringLiteral(">"), createExpression<AVGreaterThanExpression> },
    };
    return map;
};

///////////////////////////////////////////////////////////////////////////////

AVExpressionParser::AVExpressionParser() :
    AVExpressionNamespace(), m_andKeyword(" and "), m_orKeyword(" or "), m_errorPos(0),
    m_operators(getDefaultOperators())
{
}

///////////////////////////////////////////////////////////////////////////////

QString AVExpressionParser::preprocess(const QString& string) const
{
    QString result = string;
    result.remove('\n');
    result.remove('\r');
    return result;
}

///////////////////////////////////////////////////////////////////////////////

QString AVExpressionParser::formatErrorMessage(const QString& string)
{
    QString s = preprocess(string);
    s.insert(m_errorPos, "[" + m_errorMessage + "]");
    return s;
}

///////////////////////////////////////////////////////////////////////////////

bool AVExpressionParser::registerOperator(const QString &operatorStr,
                                          std::function<std::unique_ptr<AVExpression>(const QString &, const QString &)> factory)
{
    m_operators.insert(AVExpressionParser::OperatorString(operatorStr), factory);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

std::unique_ptr<AVExpression> AVExpressionParser::parseExpression(const QString& string, const QString& space)
{
    QString s = preprocess(string);
    m_errorMessage = "";
    m_errorPos = 0;
    uint last = 0;
    std::unique_ptr<AVExpression> e = parseExpression(s, space, Any, 0, last);
    if (e == 0) {
        LOGGER_AVCOMMON_AVLIB_EXPR
            .Write(LOG_ERROR, "parseExpression: %s", qPrintable(formatErrorMessage(s)));
        return 0;
    }
    if (static_cast<int>(last) != s.length() - 1) {
        m_errorMessage = "incomplete parse";
        m_errorPos = last;
        return 0;
    }
    //LOGGER_AVCOMMON_AVLIB_EXPR
    //    .Write(LOG_INFO, "parseExpression: %s\n%s", qPrintable(s), qPrintable(e->debugString()));
    return e;
}

///////////////////////////////////////////////////////////////////////////////

std::unique_ptr<AVExpression> AVExpressionParser::parseExpression(
    const QString& string, const QString& space, Type type, uint start, uint& last)
{
    uint andLength = m_andKeyword.length();
    uint orLength = m_orKeyword.length();
    AVASSERT(type == Any || type == And || type == Or);
    uint length = string.length();
    while (start < length && string.at(start) == ' ') ++start;
    if (start < length && string.at(start) == '(') {
        std::unique_ptr<AVExpression> e = parseExpression(string, space, Any, start + 1, last);
        if (e == 0) return 0;
        AVASSERT(last < length);
        if (string.at(last) != ')') {
            m_errorMessage = "expected )";
            m_errorPos = last;
            return 0;
        }
        if (last == length - 1) return e;
        if (string.at(last + 1) == ')') {
            ++last;
            return e;
        }
        while (string.mid(last + 1, 2) == "  ") ++last;
        if (string.mid(last + 1, andLength) == m_andKeyword) {
            if (type == Or) {
                m_errorMessage = "expected or";
                m_errorPos = last + 1;
                return 0;
            }
            std::unique_ptr<AVExpression> rhs = parseExpression(string, space, And, last + 1 + andLength, last);
            if (rhs == 0) {
                return 0;
            }
            return std::make_unique<AVAndExpression>(std::move(e), std::move(rhs));
        }
        if (string.mid(last + 1, orLength) == m_orKeyword) {
            if (type == And) {
                m_errorMessage = "expected and";
                m_errorPos = last + 1;
                return 0;
            }
            std::unique_ptr<AVExpression> rhs = parseExpression(string, space, Or, last + 1 + orLength, last);
            if (rhs == 0) {
                return 0;
            }
            return std::make_unique<AVOrExpression>(std::move(e), std::move(rhs));
        }
        m_errorMessage = "expected and or or";
        m_errorPos = last + 1;
        return 0;
    }
    for (uint i = start; i < length; i++) {
        if (string.at(i) == ')') {
            std::unique_ptr<AVExpression> e = parseComparison(string, space, start, i - start);
            last = i;
            return e;
        }
        if (string.mid(i, andLength) == m_andKeyword) {
            if (type == Or) {
                m_errorMessage = "expected or";
                m_errorPos = i;
                return 0;
            }
            std::unique_ptr<AVExpression> lhs = parseComparison(string, space, start, i - start);
            if (lhs == 0) return 0;
            std::unique_ptr<AVExpression> rhs = parseExpression(string, space, And, i + andLength, last);
            if (rhs == 0) {
                return 0;
            }
            return std::make_unique<AVAndExpression>(std::move(lhs), std::move(rhs));
        }
        if (string.mid(i, orLength) == m_orKeyword) {
            if (type == And) {
                m_errorMessage = "expected and";
                m_errorPos = i;
                return 0;
            }
            std::unique_ptr<AVExpression> lhs = parseComparison(string, space, start, i - start);
            if (lhs == 0) return 0;
            std::unique_ptr<AVExpression> rhs = parseExpression(string, space, Or, i + orLength, last);
            if (rhs == 0) {
                return 0;
            }
            return std::make_unique<AVOrExpression>(std::move(lhs), std::move(rhs));
        }
    }
    last = length - 1;
    return parseComparison(string, space, start, length - start);
}

///////////////////////////////////////////////////////////////////////////////

std::unique_ptr<AVExpression> AVExpressionParser::parseComparison(
    const QString& string, const QString& space, uint start, uint len)
{
    for (uint i = start; i < start + len; i++) {
        QStringRef string_ref = string.midRef(i);
        for(auto iter = m_operators.constBegin(); iter != m_operators.constEnd(); iter++)
        {
            const auto& operator_str = iter.key().operatorStr();
            if(!string_ref.startsWith(operator_str))
                continue;

            QString lhs = string.mid(start, i - start).simplified();
            if (lhs.isEmpty()) {
                m_errorMessage = "expected lhs";
                m_errorPos = i;
                return nullptr;
            }
            if (!space.isNull()) lhs.replace(space, " ");
            QString rhs = string.mid(i + operator_str.length(),
                                     start + len - (i + operator_str.length()))
                .trimmed();
            if (!space.isNull()) rhs.replace(space, " ");
            if (rhs.isNull()) rhs = "";

            auto factory = iter.value();
            return factory(std::move(lhs), std::move(rhs));
        }
    }
    m_errorMessage = "expected";
    for(auto operatorStr : m_operators.keys())
        m_errorMessage.append(" " % operatorStr.operatorStr());
    m_errorPos = start;
    return nullptr;
}

// End of file
