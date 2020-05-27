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
    \author  Kevin Krammer, k.krammer@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Unit Test for AVDateTime class
*/


// AVLib includes
#include "avdatetime.h"
#include <QTest>
#include <avunittestmain.h>

Q_DECLARE_METATYPE(AVDateTime::UtcOffsetBehaviour);

///////////////////////////////////////////////////////////////////////////////

class AVDateTimeTest : public QObject
{
    Q_OBJECT

private slots:
    void init()
    {
    }

    void cleanup()
    {
    }

    void testAddMSecs()
    {
        // test adding ms milliseconds; with 0 < ms < msecs per day
        AVDateTime t1 = AVDateTime(AVDateTime( QDate(2006, 8, 23), QTime(14, 31, 20, 100) ));
        AVDateTime t2 = AVDateTime(t1.addMSecs(123));
        QVERIFY(t2.date() == t1.date());
        QVERIFY(t2.time() == t1.time().addMSecs(123));

        t2 = t1.addMSecs(-123);
        QVERIFY(t2.date() == t1.date());
        QVERIFY(t2.time() == t1.time().addMSecs(-123));

        // test adding a full day in milliseconds
        // result:
        // - second datetime is the same as addDays(1) would create
        AVDateTime dayOne = AVDateTime(AVDateTime::currentDateTimeUtc());
        AVDateTime dayTwo(dayOne.addMSecs(MSECS_PER_DAY));

        QVERIFY(dayOne.addDays(1) == dayTwo);

        // test subtracting a full day in milliseconds
        // result:
        // - second datetime is the same as addDays(-1) would create
        dayTwo = AVDateTime::currentDateTimeUtc();
        dayOne = dayTwo.addMSecs(-int(MSECS_PER_DAY));

        QVERIFY(dayTwo.addDays(-1) == dayOne);

        // test forward transitioning midnight
        // result:
        // - time has transitioned midnight
        // - date has transitioned day
        AVDateTime beforeMidnight = AVDateTime(AVDateTime::currentDateTimeUtc());
        beforeMidnight.setTime(QTime(23, 59, 59, 999));

        AVDateTime afterMidnight(beforeMidnight.addMSecs(2));

        QVERIFY(QTime(0, 0, 0, 1) == afterMidnight.time());
        QVERIFY(beforeMidnight.addDays(1).date() == afterMidnight.date());

        // test backward transitioning midnight
        // result:
        // - time has transitioned midnight
        // - date has transitioned day
        afterMidnight = AVDateTime::currentDateTimeUtc();
        afterMidnight.setTime(QTime(0, 0, 0, 1));

        beforeMidnight = afterMidnight.addMSecs(-2);

        QVERIFY(QTime(23, 59, 59, 999) == beforeMidnight.time());
        QVERIFY(beforeMidnight.addDays(1).date() == afterMidnight.date());
    }

    void testToMSecsSinceEpoch()
    {
        AVDateTime t1 = AVDateTime(QDate(2011, 3, 1), QTime(12, 0, 0));
        qint64 msecs_t1 = t1.toMSecsSinceEpoch();
        AVDateTime t1a;
        t1a.setMSecsSinceEpoch(msecs_t1);

        QVERIFY(t1 == t1a);
        qint64 msecs_t1_ref = 1298980800;
        msecs_t1_ref *= 1000;
        QVERIFY(msecs_t1 == msecs_t1_ref);

        AVDateTime t2(QDate(2011, 3, 31), QTime(12, 0, 0));
        qint64 msecs_t2 = t2.toMSecsSinceEpoch();
        AVDateTime t2a;
        t2a.setMSecsSinceEpoch(msecs_t2);

        QVERIFY(t2 == t2a);
        qint64 msecs_t2_ref = 1301572800;
        msecs_t2_ref *= 1000;
        QVERIFY(msecs_t2 == msecs_t2_ref);
    }

    void testToMSecsSinceEpoch2()
    {
        AVDateTime base_date_time(QDate(2011, 1, 1), QTime(12, 0, 0));
        AVDateTime act_date_time = base_date_time;
        const qint64 msecs_ref = qint64(1293883200)*1000;
        const qint64 msecs_daily_inc = 24*60*60*1000;
        qint64 msecs_act = msecs_ref;
        while(act_date_time.date().year() == 2011)
        {
            AVDateTime t1 = act_date_time;
            qint64 msecs_t1 = t1.toMSecsSinceEpoch();
            AVDateTime t1a;
            t1a.setMSecsSinceEpoch(msecs_t1);

            QVERIFY(t1 == t1a);
            QVERIFY(msecs_t1 == msecs_act);

            act_date_time = act_date_time.addDays(1);
            msecs_act += msecs_daily_inc;
            //qDebug("%s",AVPrintDateTime(act_date_time).latin1());
        }
    }

    void testFromString()
    {
        {
            QString format = "yyyy-MM-ddThh:mm:ss";
            QString string = "2002-02-06T15:32:27";
            QDateTime avdt = AVDateTime::fromString(string, format);
            AVLogInfo << "testFromString avdt "<<avdt;

            // check that the same time is returned, but different time spec
            QCOMPARE(avdt.time().hour(), 15);
            QCOMPARE(avdt.timeSpec(), Qt::UTC);
        }

        // ensure offset yields a syntax error
        {
            QString format = "yyyy-MM-ddThh:mm:ss";
            QString string = "2002-02-06T15:32:27+01:00";
            QDateTime dt = AVDateTime::fromString(string, format);
            QVERIFY(!dt.isValid());
        }
    }

    void testFromStringWithDateFormat_data()
    {
        QTest::addColumn<QString>       ("string");
        QTest::addColumn<Qt::DateFormat>("format");
        QTest::addColumn<AVDateTime::UtcOffsetBehaviour>("utc_offset_behaviour");
        QTest::addColumn<AVDateTime>    ("result");

        QTest::newRow("UTC")
                << "2002-02-06T15:32:27"
                << Qt::ISODate
                << AVDateTime::UtcOffsetBehaviour::KeepUtcOffset
                << AVDateTime(QDate(2002, 2, 6), QTime(15, 32, 27), Qt::UTC);
        QTest::newRow("UTC milliseconds")
                << "2002-02-06T15:32:27.123"
                << Qt::ISODate
                << AVDateTime::UtcOffsetBehaviour::ConvertToUtc
                << AVDateTime(QDate(2002, 2, 6), QTime(15, 32, 27, 123), Qt::UTC);
        QTest::newRow("UTC milliseconds with offset, don't keep")
                << "2002-02-06T15:32:27.123+01:00"
                << Qt::ISODate
                << AVDateTime::UtcOffsetBehaviour::ConvertToUtc
                << AVDateTime(QDate(2002, 2, 6), QTime(14, 32, 27, 123), Qt::UTC);
        QTest::newRow("UTC milliseconds with offset, keep")
                << "2002-02-06T15:32:27.123+01:00"
                << Qt::ISODate
                << AVDateTime::UtcOffsetBehaviour::KeepUtcOffset
                << AVDateTime(QDate(2002, 2, 6), QTime(15, 32, 27, 123), Qt::OffsetFromUTC, 60*60);
    }
    void testFromStringWithDateFormat()
    {
        QFETCH(QString,        string);
        QFETCH(Qt::DateFormat, format);
        QFETCH(AVDateTime::UtcOffsetBehaviour, utc_offset_behaviour);
        QFETCH(AVDateTime,     result);

        QCOMPARE(AVDateTime::fromString(string, format, utc_offset_behaviour), static_cast<QDateTime>(result));
    }

    void testPlatformTimeZoneHandling()
    {
        //This test ensures the platform timezone handling works sanely SWE-5506
        AVLOG_ENTER_METHOD();

        QVERIFY(AVDateTime::currentDateTimeLocal().timeSpec() == Qt::LocalTime);
        QVERIFY(AVDateTime::currentDateTimeUtc().timeSpec()   == Qt::UTC);

        QCOMPARE(AVDateTime::currentDateTimeUtc().secsTo(AVDateTime::currentDateTimeLocal()), 0);
    }
};

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVDateTimeTest,"avlib/unittests/config")
#include "avdatetimetest.moc"


// End of file
