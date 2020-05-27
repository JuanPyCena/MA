///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author   Wolfgang Eder, w.eder@avibit.com
    \author   QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief    Test the AVExpressionParser class
*/


// AviBit common includes
#include "avexpressionparser.h"
#include <QTest>
#include <avunittestmain.h>


///////////////////////////////////////////////////////////////////////////////

class AVExpressionsTest : public QObject
{
    Q_OBJECT

public:
    void testValidExpression(
        const QString& string, const QString& debugString, const QString& toString="");
    void testInvalidExpression(const QString& string, const QString& errorMessage);

private slots:

    void testExpressionParser();
};




///////////////////////////////////////////////////////////////////////////////

void AVExpressionsTest::testValidExpression(
    const QString& string, const QString& debugString, const QString& toString)
{
    AVExpressionParser parser;
    std::unique_ptr<AVExpression> e = parser.parseExpression(string);
    QVERIFY(e != 0);
    QCOMPARE(e->debugString(), debugString);
    if (toString.isEmpty())
        QCOMPARE(e->toString(), string);
    else
        QCOMPARE(e->toString(), toString);
}

///////////////////////////////////////////////////////////////////////////////

void AVExpressionsTest::testInvalidExpression(const QString& string, const QString& errorMessage)
{
    AVExpressionParser parser;
    int prev = LOGGER_AVCOMMON_AVLIB.minLevel();
    LOGGER_AVCOMMON_AVLIB_EXPR.setMinLevel(AVLog::LOG__FATAL);
    std::unique_ptr<AVExpression> e = parser.parseExpression(string);
    LOGGER_AVCOMMON_AVLIB_EXPR.setMinLevel(prev);
    QVERIFY(e == 0);
    QCOMPARE(parser.formatErrorMessage(string), errorMessage);
}

///////////////////////////////////////////////////////////////////////////////

void AVExpressionsTest::testExpressionParser()
{
    testValidExpression("a==1", "a Equals 1\n");
    testValidExpression("a==", "a Equals empty\n");
    testValidExpression("a!=1", "a NotEquals 1\n");
    testValidExpression("a!=", "a NotEquals empty\n");
    testValidExpression("a<1", "a LessThan 1\n");
    testValidExpression("a<", "a LessThan empty\n");
    testValidExpression("a<=1", "a LessThanOrEqual 1\n");
    testValidExpression("a<=", "a LessThanOrEqual empty\n");
    testValidExpression("a>1", "a GreaterThan 1\n");
    testValidExpression("a>", "a GreaterThan empty\n");
    testValidExpression("a>=1", "a GreaterThanOrEqual 1\n");
    testValidExpression("a>=", "a GreaterThanOrEqual empty\n");
    testValidExpression("a=~1", "a Matches 1\n");
    testValidExpression("a=~", "a Matches empty\n");
    testValidExpression("a!~1", "a NotMatches 1\n");
    testValidExpression("a!~", "a NotMatches empty\n");
    testValidExpression("a=1", "a Assign 1\n");
    testValidExpression("a=", "a Assign empty\n");
    testValidExpression("a+=1", "a Append 1\n");
    testValidExpression("a+=", "a Append empty\n");

    testValidExpression("a==1 and b==2", "And\n a Equals 1\n b Equals 2\n");
    testValidExpression("a==1 and b==2 and c==3",
                        "And\n a Equals 1\n And\n  b Equals 2\n  c Equals 3\n");
    testValidExpression("a==1 or b==2", "Or\n a Equals 1\n b Equals 2\n");
    testValidExpression("a==1 or b==2 or c==3",
                        "Or\n a Equals 1\n Or\n  b Equals 2\n  c Equals 3\n");

    testValidExpression("(a==1)", "a Equals 1\n", "a==1");
    testValidExpression("((a==1))", "a Equals 1\n", "a==1");
    testValidExpression("a==1 and (b==2 or c==3)",
                        "And\n a Equals 1\n Or\n  b Equals 2\n  c Equals 3\n");
    testValidExpression("(a==1 and b==2) or c==3",
                        "Or\n And\n  a Equals 1\n  b Equals 2\n c Equals 3\n");
    testValidExpression("(a==1 and b==2) or (c==3 and d==4)",
                        "Or\n And\n  a Equals 1\n  b Equals 2\n And\n  c Equals 3\n  d Equals 4\n");
    testValidExpression("a==1 and (b==2 or c==3) and d==4",
                        "And\n a Equals 1\n And\n  Or\n   b Equals 2\n   c Equals 3\n"
                        "  d Equals 4\n");

    testValidExpression("  a  ==  1  and  b  ==  2  ", "And\n a Equals 1\n b Equals 2\n",
                        "a==1 and b==2");
    testValidExpression("a==1 and \nb==2", "And\n a Equals 1\n b Equals 2\n", "a==1 and b==2");
    testValidExpression("a==1\n and b==2", "And\n a Equals 1\n b Equals 2\n", "a==1 and b==2");
    testValidExpression("a==1\n and \nb==2", "And\n a Equals 1\n b Equals 2\n", "a==1 and b==2");
    testValidExpression("  (  a  ==  1  and  b  ==  2  )  or  c  ==  3  ",
                        "Or\n And\n  a Equals 1\n  b Equals 2\n c Equals 3\n",
                        "(a==1 and b==2) or c==3");
    testValidExpression("(a==1 and b==2) or \nc==3",
                        "Or\n And\n  a Equals 1\n  b Equals 2\n c Equals 3\n",
                        "(a==1 and b==2) or c==3");
    testValidExpression("(a==1 and b==2)\n or c==3",
                        "Or\n And\n  a Equals 1\n  b Equals 2\n c Equals 3\n",
                        "(a==1 and b==2) or c==3");
    testValidExpression("(a==1 and b==2)\n or \nc==3",
                        "Or\n And\n  a Equals 1\n  b Equals 2\n c Equals 3\n",
                        "(a==1 and b==2) or c==3");

    testInvalidExpression("", "[expected != !~ += <= == =~ >= < = >]");
    testInvalidExpression("a", "[expected != !~ += <= == =~ >= < = >]a");
    testInvalidExpression("==1", "[expected lhs]==1");
    testInvalidExpression("==", "[expected lhs]==");
    testInvalidExpression("!=1", "[expected lhs]!=1");
    testInvalidExpression("!=", "[expected lhs]!=");
    testInvalidExpression("<1", "[expected lhs]<1");
    testInvalidExpression("<", "[expected lhs]<");
    testInvalidExpression("<=1", "[expected lhs]<=1");
    testInvalidExpression("<=", "[expected lhs]<=");
    testInvalidExpression(">1", "[expected lhs]>1");
    testInvalidExpression(">", "[expected lhs]>");
    testInvalidExpression(">=1", "[expected lhs]>=1");
    testInvalidExpression(">=", "[expected lhs]>=");
    testInvalidExpression("=~1", "[expected lhs]=~1");
    testInvalidExpression("=~", "[expected lhs]=~");
    testInvalidExpression("!~1", "[expected lhs]!~1");
    testInvalidExpression("!~", "[expected lhs]!~");
    testInvalidExpression("=1", "[expected lhs]=1");
    testInvalidExpression("=", "[expected lhs]=");
    testInvalidExpression("+=1", "[expected lhs]+=1");
    testInvalidExpression("+=", "[expected lhs]+=");
    testInvalidExpression("()", "([expected != !~ += <= == =~ >= < = >])");
    testInvalidExpression("(a)", "([expected != !~ += <= == =~ >= < = >]a)");
    testInvalidExpression("(a=1)b=2", "(a=1)[expected and or or]b=2");
    testInvalidExpression("a=1 and b=2 or c=3", "a=1 and b=2[expected and] or c=3");
    testInvalidExpression("a=1 or b=2 and c=3", "a=1 or b=2[expected or] and c=3");
    testInvalidExpression("(", "([expected != !~ += <= == =~ >= < = >]");
    testInvalidExpression("(a==1", "(a==[expected )]1");

    // TODO this is a small bug in the parser that we ignore for now

    //testInvalidExpression("a==1)", "a Equals 1\n");
    testInvalidExpression("(a==1)))", "(a==1)[incomplete parse]))");
    testInvalidExpression("a==1) or b==2", "a==1[incomplete parse]) or b==2");
}



AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVExpressionsTest,"avlib/unittests/config")
#include "avexpressionstest.moc"

// End of file
