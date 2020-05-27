///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    TESTS - AVCOMMON Tests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Christian Muschick, c.muschick@avibit.com
    \author  QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief   Tests the avfromtostring functionality.
 */


#include <limits>

// Qt includes
#include <QString>
#include <QTest>
#include <QVector>
#include <QPen>
#include <QUuid>

#include "avunittestmain.h"

#include "avfromtostring.h"
#include "avdaemon.h"
#include "avlog.h"
#include "avtimereference.h"
#include "avtostringrepresentation.h"

// work around windows namespace pollution
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif


///////////////////////////////////////////////////////////////////////////////

class AVFromToStringTest : public QObject
{
    Q_OBJECT

private slots:
    virtual void init();
    virtual void cleanup();
    void testNestedSplit();
    void testHarmless();
    void testNestedEscapedString();
    void testSplitFailure();
    void testQString();
    void testQColor();
    void testQPoint();
    void testQPolygon();
    void testQSize();
    void testQRect();
    void testQDateTime();
    void testQDateTimeInMap();
    void testQDate();
    void testQTime();
    void testQVariant();
    void testPrimitiveNumberTypes();
    void testQUuid();
    void testQEnum();
    void testAVToStringRepresentation();
};

///////////////////////////////////////////////////////////////////////////////

void AVFromToStringTest::init()
{
}

///////////////////////////////////////////////////////////////////////////////

void AVFromToStringTest::cleanup()
{
}

///////////////////////////////////////////////////////////////////////////////

void AVFromToStringTest::testNestedSplit()
{
    QString str;
    QStringList split;

    {
        str = "  test    ";
        QVERIFY(avfromtostring::nestedSplit(split, ";", str, false));
        QVERIFY(split.size() == 1);
        QVERIFY(split[0] == "test");
    }

    {
        str = " [abc[[]];;def];abc;\"];   \";  ;  ";

        QVERIFY(avfromtostring::nestedSplit(split, ";", str, false));
        QVERIFY(split.size() == 5);
        QVERIFY(split[0] == "[abc[[]];;def]");
        QVERIFY(split[1] == "abc");
        QVERIFY(split[2] == "\"];   \"");
        QVERIFY(split[3] == "");
        QVERIFY(split[4] == "");
    }

    {
        str = "";
        QVERIFY(avfromtostring::nestedSplit(split, ";", str, false));
        QVERIFY(split.isEmpty());
    }

    {
        str = "   ";
        QVERIFY(avfromtostring::nestedSplit(split, ";", str, false));
        QVERIFY(split.isEmpty());
    }

    {
        str = "\"\"";
        QVERIFY(avfromtostring::nestedSplit(split, ";", str, false));
        QVERIFY(split.size() == 1);
        QVERIFY(split[0] == "\"\"");
    }

    {
        str = ";  ";
        QVERIFY(avfromtostring::nestedSplit(split, ";", str, false));
        QVERIFY(split.size() == 2);
        QVERIFY(split[0] == "");
        QVERIFY(split[1] == "");
    }

    {
        str = "a;b";
        QVERIFY(avfromtostring::nestedSplit(split, ";", str, false));
        QVERIFY(split.size() == 2);
        QVERIFY(split[0] == "a");
        QVERIFY(split[1] == "b");
    }

    {
        str = ";";
        QVERIFY(avfromtostring::nestedSplit(split, ";", str, false));
        QVERIFY(split.size() == 2);
        QVERIFY(split[0] == "");
        QVERIFY(split[1] == "");
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVFromToStringTest::testHarmless()
{
    typedef QPair<QString, QString> TestPair;
    TestPair pair = qMakePair(QString("abc"), QString("def"));
    TestPair pair2;

    QString str = AVToString(pair);
//     AVLogInfo << "AVFromToStringTest::testHarmless: original string is " << str;

    QVERIFY(AVFromString(str, pair2));

//     AVLogInfo << "AVFromToStringTest::testHarmless: read string is " << AVToString(pair2);

    QVERIFY(pair == pair2);
}

///////////////////////////////////////////////////////////////////////////////

void AVFromToStringTest::testNestedEscapedString()
{
    typedef QPair<QString, QString> TestKey;
    typedef QMap<TestKey, QVector<QString> > TestContainer;

    TestContainer test_container, test_container2;
    TestKey key = qMakePair(QString("[:;\\\"\\\\"), QString("[qwdwq;fqef]"));

    test_container[key].push_back(" a ");
    test_container[key].push_back("\"]:\"");
    test_container[key].push_back("\";\"");

    QString str = AVToString(test_container);
    AVLogInfo << "AVFromToStringTest::testNestedEscapedString: string is " << str;

    QVERIFY(AVFromString(str, test_container2));

    // We are lazy, and test_container == test_container2 doesn't work.
    // Assume that AVToString does the proper thing.
    QVERIFY(AVToString(test_container) == AVToString(test_container2));
}

///////////////////////////////////////////////////////////////////////////////

void AVFromToStringTest::testSplitFailure()
{
    QString str("]");
    QStringList list;
    QVERIFY(!avfromtostring::nestedSplit(list, ";", str, false));
}

///////////////////////////////////////////////////////////////////////////////

void AVFromToStringTest::testQString()
{
    QString str;
    QString res;

    {
        str = "\"\"";
        QVERIFY(AVFromString(str, res));
        QVERIFY(res.isEmpty());
        QVERIFY(!res.isNull());
    }

    {
        str = QString::null;
        QString null_rep = AVToString(str);
        QVERIFY(AVFromString(null_rep, res));
        QVERIFY(res == str);
        // This doesn't work out-of-the-box for Qt5 because of the way the null string is implemented as struct...
        QVERIFY(null_rep == AVToString(QString::null));
    }

    {
        str = "\"";
        QString tostr = AVToString(str, true);
        QVERIFY(AVFromString(tostr, res));
        QVERIFY(res == str);
    }

    {
        str = "   ";
        QVERIFY(AVFromString(AVToString(str, true), res));
        QVERIFY(res == str);
    }

    {
        str = ";";
        QString to_string = AVToString(str, true);
        QCOMPARE(QString("\";\""), to_string);
        QVERIFY(AVFromString(to_string, res));
        QVERIFY(res == str);
    }

    {
        str = "\\";
        QString to_string = AVToString(str, true);
        QCOMPARE(QString("\"\\\\\""), to_string);
        QVERIFY(AVFromString(to_string, res));
        QVERIFY(res == str);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVFromToStringTest::testQColor()
{
    // basic to/from test (color with transparency)
    {
        QColor input(0x0, 0xff, 0x00, 0xdd);
        QString refToStr = "#00ff00dd";
        QCOMPARE(AVToString(input), refToStr);

        QColor resultFromString;
        QVERIFY(AVFromString(refToStr, resultFromString));
        QVERIFY(input == resultFromString);
    }

    // Test from/to string of an invalid color
    {
        const QColor input;
        // AVToString of the null string is used to represent invalid colors
        QString refToStr = AVToString(QString());
        QCOMPARE(AVToString(input), refToStr);

        QColor resultFromString;
        QVERIFY(AVFromString(refToStr, resultFromString));
        QVERIFY(input == resultFromString);
    }

    // Test if from string works for the #RRGGBB format.
    {
        const QColor input(Qt::blue);
        QString refToStr = "#0000ff";
        QColor resultFromString;
        QVERIFY(AVFromString(refToStr, resultFromString));
        QVERIFY(input == resultFromString);
    }

    // Test if to string uses #RRGGBB format for opaque colors.
    {
        const QColor input(Qt::blue);
        QString refToStr = "#0000ff";
        QCOMPARE(AVToString(input), refToStr);
    }

    // Test if from string works for strings that use upper case hex digits
    {
        const QColor input(0x0a, 0x0b, 0x0c);
        QString refToStr = "#0A0B0C";
        QColor resultFromString;
        QVERIFY(AVFromString(refToStr, resultFromString));
        QVERIFY(input == resultFromString);
    }

    // Test if from string detects an invalid input properly
    {
        QString refToStr = "0A0B0";
        QColor resultFromString;
        QVERIFY(!AVFromString(refToStr, resultFromString));
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVFromToStringTest::testQPoint()
{
    // basic to/from test
    {
        const QPoint  input(100, 200);
        QString refToStr = "[100; 200]";
        QCOMPARE(AVToString(input),refToStr);

        QPoint resultFromString;
        QVERIFY(AVFromString(refToStr, resultFromString));
        QVERIFY(input == resultFromString);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVFromToStringTest::testQPolygon()
{
    // basic to/from test
    {
        QPolygon input(3);
        input.setPoint(0, 100, 200);
        input.setPoint(1, -100, -200);
        input.setPoint(2, 500, 800);

        QString refToStr = "[[100; 200]; [-100; -200]; [500; 800]]";
        QCOMPARE(AVToString(input),refToStr);

        QPolygon resultFromString;
        QVERIFY(AVFromString(refToStr, resultFromString));
        QVERIFY(input == resultFromString);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVFromToStringTest::testQSize()
{
    // basic to/from test
    {
        const QSize input(100, 200);
        QString refToStr = "[100x 200]";
        QCOMPARE(AVToString(input),refToStr);

        QSize resultFromString;
        QVERIFY(AVFromString(refToStr, resultFromString));
        QVERIFY(input == resultFromString);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVFromToStringTest::testQRect()
{
    // basic to/from test
    {
        const QRect input(0, 0, 100, 200);
        QString refToStr = "[[0; 0]; [100x 200]]";
        QCOMPARE(AVToString(input),refToStr);

        QRect resultFromString;
        QVERIFY(AVFromString(refToStr, resultFromString));
        QVERIFY(input == resultFromString);
    }
}


///////////////////////////////////////////////////////////////////////////////

void AVFromToStringTest::testQDateTime()
{
    AVDateTime result;
    {
        QDateTime dt = AVTimeReference::currentDateTime();
        QString string = AVToString(dt);
        AVLogInfo << "AVFromToStringTest::testQDateTime: current time: "
                  << string;
        QVERIFY(AVFromString(string, result));
        QVERIFY(dt == result);
    }

    // single digits in date/time; this cannot be parsed in Qt 5.12 with the "hh" etc format specifiers (parsing
    // got more strict compared to Qt 5.9)
    // see SWE-6656
    QVERIFY(AVFromString("2010-4-3T8:3:5.1", result));
    QCOMPARE(result, AVDateTime(QDate(2010,4,3), QTime(8,3,5,100)));

    QVERIFY(AVFromString("2010-4-03T18:31:25", result));
    QCOMPARE(result, AVDateTime(QDate(2010,4,3), QTime(18,31,25)));

    QVERIFY(AVFromString("20100203163025.003", result));
    QCOMPARE(result, AVDateTime(QDate(2010,2,3), QTime(16,30,25,3)));

    QVERIFY(AVFromString("20110304173126", result));
    QCOMPARE(result, AVDateTime(QDate(2011,3,4), QTime(17,31,26)));

    QVERIFY(AVFromString("201103041731", result));
    QCOMPARE(result, AVDateTime(QDate(2011,3,4), QTime(17,31,00)));

    AVDateTime null_date_time;
    QVERIFY(null_date_time.isNull());
    QString null_rep = AVToString(null_date_time);
    null_date_time = AVTimeReference::currentDateTime();
    QVERIFY(AVFromString(null_rep, null_date_time));
    QVERIFY(null_date_time.isNull());
    null_date_time = AVTimeReference::currentDateTime();
    QVERIFY(AVFromString("", null_date_time));
    QVERIFY(null_date_time.isNull());
}

///////////////////////////////////////////////////////////////////////////////

void AVFromToStringTest::testQDateTimeInMap()
{
    // ':' is used both as key:value separator and in the time portion of the datetime representation - make sure that
    // conversion to and fro works (datetime is quoted)

    QMap<QString, QDateTime> map_out, map_in;
    map_out["1"] = AVDateTime::currentDateTimeUtc();
    map_out[":"] = AVDateTime();
    QString string_rep = AVToString(map_out);
    AVLogInfo << string_rep;
    QVERIFY(AVFromString(string_rep, map_in));
    QVERIFY(map_out == map_in);
}

///////////////////////////////////////////////////////////////////////////////

void AVFromToStringTest::testQDate()
{
    QDate dt = AVTimeReference::currentDateTime().date();

    QString string = AVToString(dt);

    AVLogInfo << "AVFromToStringTest::testQDate: "<<string;

    QDate result;
    QVERIFY(AVFromString(string, result));

    QVERIFY(dt == result);

    QVERIFY(AVFromString("2010-2-23", result));
    QCOMPARE(result, QDate(2010,2,23));

    QVERIFY(AVFromString("20100203", result));
    QCOMPARE(result, QDate(2010,2,3));

    QDate null_date;
    QVERIFY(null_date.isNull());
    QString null_rep = AVToString(null_date);
    null_date = AVTimeReference::currentDateTime().date();
    QVERIFY(AVFromString(null_rep, null_date));
    QVERIFY(null_date.isNull());
    null_date = AVTimeReference::currentDateTime().date();
    QVERIFY(AVFromString("", null_date));
    QVERIFY(null_date.isNull());
}

///////////////////////////////////////////////////////////////////////////////

void AVFromToStringTest::testQTime()
{
    QTime dt = AVTimeReference::currentDateTime().time();

    QString string = AVToString(dt);

    AVLogInfo << "AVFromToStringTest::testQTime: "<<string;

    QTime result;
    QVERIFY(AVFromString(string, result));

    QVERIFY(dt == result);

    QVERIFY(AVFromString("6:30:25.3", result));
    AVLogInfo << result;
    QCOMPARE(result, QTime(6,30,25,300));

    QVERIFY(AVFromString("163025.003", result));
    QCOMPARE(result, QTime(16,30,25,3));

    QVERIFY(AVFromString("173126", result));
    QCOMPARE(result, QTime(17,31,26));

    QTime null_time;
    QVERIFY(null_time.isNull());
    QString null_rep = AVToString(null_time);
    null_time = AVTimeReference::currentDateTime().time();
    QVERIFY(AVFromString(null_rep, null_time));
    QVERIFY(null_time.isNull());
    null_time = AVTimeReference::currentDateTime().time();
    QVERIFY(AVFromString("", null_time));
    QVERIFY(null_time.isNull());
}

///////////////////////////////////////////////////////////////////////////////

void AVFromToStringTest::testQVariant()
{
    auto string = QString::null;
    auto string2 = QString("Test");
    QVariant string_var(string);
    QVariant string2_var(string2);
    QVERIFY(AVToString(string_var) == AVToString(string));
    QVERIFY(AVToString(string2_var) == AVToString(string2));

    auto string_list = QStringList() << "T1" << "T2";
    QVariant string_list_var(string_list);
    QVERIFY(AVToString(string_list_var) == AVToString(string_list));

    auto rect = QRect(0, 1, 100, 200);
    QVariant rect_var(rect);
    QVERIFY(AVToString(rect_var) == AVToString(rect));

    auto size = QSize(200, 300);
    QVariant size_var(size);
    QVERIFY(AVToString(size_var) == AVToString(size));

    auto color = QColor(255, 128, 0);
    QVariant color_var(color);
    QVERIFY(AVToString(color_var) == AVToString(color));

    auto point = QPoint(-20, 20);
    QVariant point_var(point);
    QVERIFY(AVToString(point_var) == AVToString(point));

    int int_val = -5;
    QVariant int_val_var(int_val);
    QVERIFY(AVToString(int_val_var) == AVToString(int_val));

    uint uint_val = 5;
    QVariant uint_val_var(uint_val);
    QVERIFY(AVToString(uint_val_var) == AVToString(uint_val));

    bool truth = true;
    QVariant truth_var(truth);
    QVERIFY(AVToString(truth_var) == AVToString(truth));

    double num = 4.32;
    QVariant num_var(num);
    QVERIFY(AVToString(num_var) == AVToString(num));

    auto date = QDate(2014, 12, 12);
    auto invalid_date = QDate();
    QVariant date_var(date);
    QVariant invalid_date_var(invalid_date);
    QVERIFY(AVToString(date_var) == AVToString(date));
    QVERIFY(AVToString(invalid_date_var) == AVToString(invalid_date));

    auto time = QTime(5, 20, 10, 543);
    QVariant time_var(time);
    QVERIFY(AVToString(time_var) == AVToString(time));

    auto date_time = AVDateTime(date, QTime(5, 20, 10, 500));
    QVariant date_time_var(date_time);
    QVERIFY(AVToString(date_time_var) == AVToString(date_time));

    qlonglong llong_num = -1;
    QVariant llong_num_var(llong_num);
    QVERIFY(AVToString(llong_num_var) == AVToString(llong_num));

    qulonglong  ullong_num = 1;
    QVariant ullong_num_var(ullong_num);
    QVERIFY(AVToString(ullong_num_var) == AVToString(ullong_num));

    QPen pen;
    QVariant pen_var(pen);
    QCOMPARE(AVToString(pen_var), QString("QVariant(QPen)"));
}

///////////////////////////////////////////////////////////////////////////////

void AVFromToStringTest::testPrimitiveNumberTypes()
{
    QString uint8_str1 = "256";
    QString uint8_str2 = "-1";
    QString uint8_str3 = "255";
    quint8 uint8_var;
    QVERIFY(AVFromString(uint8_str1, uint8_var) == false);
    QVERIFY(AVFromString(uint8_str2, uint8_var) == false);
    QVERIFY(AVFromString(uint8_str3, uint8_var) == true);
    QVERIFY(AVToString(uint8_var) == "255");

    QString int8_str1 = "128";
    QString int8_str2 = "-1";
    QString int8_str3 = "127";
    qint8 int8_var;
    QVERIFY(AVFromString(int8_str1, int8_var) == false);
    QVERIFY(AVFromString(int8_str2, int8_var) == true);
    QVERIFY(AVToString(int8_var) == "-1");
    QVERIFY(AVFromString(int8_str3, int8_var) == true);
    QVERIFY(AVToString(int8_var) == "127");

    QString int64_str1 = "9223372036854775808";
    QString int64_str2 = "9223372036854775807";
    QString int64_str3 = "-9223372036854775808";
    qint64 int64_var;
    QVERIFY(AVFromString(int64_str1, int64_var) == false);
    QVERIFY(AVFromString(int64_str2, int64_var) == true);
    QVERIFY(int64_var == std::numeric_limits<qint64>::max());
    QVERIFY(AVFromString(int64_str3, int64_var) == true);
    QVERIFY(int64_var == std::numeric_limits<qint64>::min());

    QString uint64_str1 = "18446744073709551615";
    QString uint64_str2 = "18446744073709551616";
    QString uint64_str3 = "-1";
    quint64 uint64_var;
    QVERIFY(AVFromString(uint64_str1, uint64_var) == true);
    QVERIFY(uint64_var == std::numeric_limits<quint64>::max());
    QVERIFY(AVFromString(uint64_str2, uint64_var) == false);
    QVERIFY(AVFromString(uint64_str3, uint64_var) == false);


    QString int_str1 = "123,123";
    QString int_str2 = "123A";
    QString int_str3 = " 314 ";
    QString int_str4 = "123 123";
    int int_var;
    QVERIFY(AVFromString(int_str1, int_var) == false);
    QVERIFY(AVFromString(int_str2, int_var) == false);
    QVERIFY(AVFromString(int_str3, int_var) == true);
    QVERIFY(int_var == 314);
    QVERIFY(AVFromString(int_str4, int_var) == false);

    QString ulong_string = "123123";
    unsigned long ulong_var;
    QVERIFY(AVFromString(ulong_string, ulong_var));
    QCOMPARE(AVToString(ulong_var), ulong_string);

    QString long_string = "-123123";
    long long_var;
    QVERIFY(AVFromString(long_string, long_var));
    QCOMPARE(AVToString(long_var), long_string);

    QString double_str1 = "12.45";
    double double_value;
    QVERIFY(AVFromString(double_str1, double_value) == true);
    QCOMPARE(double_value,12.45);
    QCOMPARE(AVToString(double_value), double_str1);
    AVLogInfo << "double: "<<AVToString(double_value);
}

///////////////////////////////////////////////////////////////////////////////

void AVFromToStringTest::testQUuid()
{
    QString uuid_rep = "{abcde123-1234-abcd-ef12-123456789012}";

    QUuid uuid;
    QVERIFY(AVFromString(uuid_rep, uuid));
    QVERIFY(!uuid.isNull());
    QVERIFY(AVToString(uuid) == uuid_rep);

    QUuid null_uuid;
    QString null_rep = AVToString(null_uuid);
    QVERIFY(AVFromString(null_rep, uuid));
    QVERIFY(uuid.isNull());
}

///////////////////////////////////////////////////////////////////////////////

struct TestQEnums
{
    Q_GADGET
public:
    enum TstQEnum {
        Value1,
        Value2
    };
    Q_ENUM(TstQEnum)
    enum class TstQTypeSafeEnum {
        TSValue1,
        TSValue2
    };
    Q_ENUM(TstQTypeSafeEnum)
};

void AVFromToStringTest::testQEnum()
{
    AVLogInfo << "---- testQEnums launched ----";
    QCOMPARE(AVToString(TestQEnums::TstQEnum::Value1), QString("Value1"));

    TestQEnums::TstQEnum value = TestQEnums::TstQEnum::Value1;
    QVERIFY(AVFromString("\"Value2\"", value));
    QCOMPARE(value, TestQEnums::TstQEnum::Value2);
    QVERIFY(AVFromString("Value1", value));
    QCOMPARE(value, TestQEnums::TstQEnum::Value1);

    TestQEnums::TstQTypeSafeEnum ts_value = TestQEnums::TstQTypeSafeEnum::TSValue2;
    QVERIFY(AVFromString("TSValue1", ts_value));
    QCOMPARE(ts_value, TestQEnums::TstQTypeSafeEnum::TSValue1);
    QVERIFY(AVFromString("TSValue2", ts_value));
    QCOMPARE(ts_value, TestQEnums::TstQTypeSafeEnum::TSValue2);

    AVLogInfo << "---- testQEnums ended ----";
}

void AVFromToStringTest::testAVToStringRepresentation()
{
    const QStringList test_cases =
    {
        "", " ", ";", "\"\"", "\\0",
        "[0; STRING]", "[;2]", "[ \"QUOTED\":[ 0; 1]",
        "SIMPLE", " LEFT", "RIGHT ", " BOTH ", "IN BETWEEN",
        " EVERY WHERE ", " !SPECIAL? [CHARACTERS]; $% *& () ",
        "\" ESCAPED \"", "\" Broken escapes "
    };

    for (const QString& test_case : test_cases)
    {
        AVToStringRepresentation representation = test_case;
        QString intermediary = AVToString(representation);

        QCOMPARE(intermediary, representation);
        QCOMPARE(intermediary, test_case);

        AVToStringRepresentation result;
        QVERIFY(AVFromString(intermediary, result));

        QCOMPARE(result, test_case);
        QCOMPARE(result, representation);
        QCOMPARE(result, intermediary);
    }
}

///////////////////////////////////////////////////////////////////////////////

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVFromToStringTest,"avlib/unittests/config")
#include "avfromtostringtest.moc"

// End of file
