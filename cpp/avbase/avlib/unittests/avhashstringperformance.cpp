///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2013
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

#include <QtTest>
#include <QString>
#include <QMap>
#include <QHash>

#include <avunittestmain.h>

#include "avexpressions.h"

// This performance benchmark should compare different approaches to implement ::valueOfLHS
// of a AVComputable (every test "simulates" 400 different valueOFLHS entries)
class AVHashStringPerformance : public QObject
{
    Q_OBJECT

public slots:
    void init();
    void cleanup();

private slots:
    // comparing AVHashString == "literal"
    void tst_avhashstring();

    // comparing QString == "literal"
    void tst_qstring();

    // comparing QString == QStringLiteral("literal")
    void tst_qstringOptimized();

    // comparing AVHashString::qstring() == QStringLiteral("literal")
    void tst_avHashStringOptimized();

    // lookup using QMap<QString, std::function<QString()> >
    void tst_qstringmap();

    // lookup QHash<QString, std::function<QString()> >
    void tst_qstringhash();

private:
    template<typename T>
    QString doStringLookup(const T &name);

    template<typename T>
    QString doStringLiteralLookup(const T &name);

    QString doAVHashStringLiteralLookup(const AVHashString &name);

    template<template<class, class> class T>
    QString doDictionaryLookup(const QString &name);
};

///////////////////////////////////////////////////////////////////////////////

//! This will be called before each test function is executed.
void AVHashStringPerformance::init()
{
}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after every test function.
void AVHashStringPerformance::cleanup()
{
}

///////////////////////////////////////////////////////////////////////////////

void AVHashStringPerformance::tst_avhashstring()
{
    const AVHashString a_string(QStringLiteral("this_is_a_test"));

    QBENCHMARK {
        doStringLookup(a_string);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVHashStringPerformance::tst_qstring()
{
    const QString a_string = QStringLiteral("this_is_a_test");

    QBENCHMARK {
        doStringLookup(a_string);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVHashStringPerformance::tst_qstringOptimized()
{
    const QString a_string = QStringLiteral("this_is_a_test");

    QBENCHMARK {
        doStringLiteralLookup(a_string);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVHashStringPerformance::tst_avHashStringOptimized()
{
    const AVHashString a_string(QStringLiteral("this_is_a_test"));

    QBENCHMARK {
        doAVHashStringLiteralLookup(a_string);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVHashStringPerformance::tst_qstringmap()
{
    static const QString name = QStringLiteral("this_is_a_test");

    QBENCHMARK {
        doDictionaryLookup<QMap>(name);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVHashStringPerformance::tst_qstringhash()
{
    static const QString name = QStringLiteral("this_is_a_test");

    QBENCHMARK {
        doDictionaryLookup<QHash>(name);
    }
}

///////////////////////////////////////////////////////////////////////////////

#define PACK_OF_10(TEST_CASE, SUFFIX) \
    TEST_CASE("this_is_a_test" SUFFIX "0") TEST_CASE("this_is_a_test" SUFFIX "1") TEST_CASE("this_is_a_test" SUFFIX "2") \
    TEST_CASE("this_is_a_test" SUFFIX "3") TEST_CASE("this_is_a_test" SUFFIX "4") TEST_CASE("this_is_a_test" SUFFIX "5") \
    TEST_CASE("this_is_a_test" SUFFIX "6") TEST_CASE("this_is_a_test" SUFFIX "7") TEST_CASE("this_is_a_test" SUFFIX "8") \
    TEST_CASE("this_is_a_test" SUFFIX "9")

#define REPEAT_TEST_CASES(TEST_CASE) \
    PACK_OF_10(TEST_CASE, "") PACK_OF_10(TEST_CASE, "1") PACK_OF_10(TEST_CASE, "2") PACK_OF_10(TEST_CASE, "3") \
    PACK_OF_10(TEST_CASE, "4") PACK_OF_10(TEST_CASE, "5") PACK_OF_10(TEST_CASE, "6") PACK_OF_10(TEST_CASE, "7") \
    PACK_OF_10(TEST_CASE, "8") PACK_OF_10(TEST_CASE, "9") PACK_OF_10(TEST_CASE, "10") PACK_OF_10(TEST_CASE, "11") \
    PACK_OF_10(TEST_CASE, "12") PACK_OF_10(TEST_CASE, "13") PACK_OF_10(TEST_CASE, "14") PACK_OF_10(TEST_CASE, "15") \
    PACK_OF_10(TEST_CASE, "16") PACK_OF_10(TEST_CASE, "17") PACK_OF_10(TEST_CASE, "18") PACK_OF_10(TEST_CASE, "19") \
    PACK_OF_10(TEST_CASE, "20") PACK_OF_10(TEST_CASE, "21") PACK_OF_10(TEST_CASE, "22") PACK_OF_10(TEST_CASE, "23") \
    PACK_OF_10(TEST_CASE, "24") PACK_OF_10(TEST_CASE, "25") PACK_OF_10(TEST_CASE, "26") PACK_OF_10(TEST_CASE, "27") \
    PACK_OF_10(TEST_CASE, "28") PACK_OF_10(TEST_CASE, "29") PACK_OF_10(TEST_CASE, "30") PACK_OF_10(TEST_CASE, "31") \
    PACK_OF_10(TEST_CASE, "12") PACK_OF_10(TEST_CASE, "13") PACK_OF_10(TEST_CASE, "34") PACK_OF_10(TEST_CASE, "35") \
    PACK_OF_10(TEST_CASE, "36") PACK_OF_10(TEST_CASE, "37") PACK_OF_10(TEST_CASE, "38") PACK_OF_10(TEST_CASE, "39")

template<typename T>
QString AVHashStringPerformance::doStringLookup(const T &name)
{
    //none of these will match, so no early exit and no console output!
#define MY_TEST_CASE(x) if(name == x) return QStringLiteral("it was " x);
    REPEAT_TEST_CASES(MY_TEST_CASE)
#undef MY_TEST_CASE

    return QString();
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
QString AVHashStringPerformance::doStringLiteralLookup(const T &name)
{
    //none of these will match, so no early exit and no console output!
#define MY_TEST_CASE(x) if(name == QStringLiteral(x)) return QStringLiteral("it was " x);
    REPEAT_TEST_CASES(MY_TEST_CASE)
#undef MY_TEST_CASE

    return QString();
}

///////////////////////////////////////////////////////////////////////////////

QString AVHashStringPerformance::doAVHashStringLiteralLookup(const AVHashString &name)
{
    //none of these will match, so no early exit and no console output!
#define MY_TEST_CASE(x) if(name.qstring() == QStringLiteral(x)) return QStringLiteral("it was " x);
    REPEAT_TEST_CASES(MY_TEST_CASE)
#undef MY_TEST_CASE

    return QString();
}

///////////////////////////////////////////////////////////////////////////////

template<template<class, class> class T>
QString AVHashStringPerformance::doDictionaryLookup(const QString &name)
{
    static const T<QString, std::function<QString()> > map {
#define MY_TEST_CASE(x) { x, [](){ return QStringLiteral("it was " x); } },
    REPEAT_TEST_CASES(MY_TEST_CASE)
#undef MY_TEST_CASE
    };

    auto iter = map.find(name);
    if(iter != map.constEnd())
        return iter.value()();

    return QString();
}

///////////////////////////////////////////////////////////////////////////////

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVHashStringPerformance,"avlib/unittests/config")
#include "avhashstringperformance.moc"

// End of file
