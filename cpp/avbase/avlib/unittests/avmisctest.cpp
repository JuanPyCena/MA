///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Clemens Ender
    \author  Markus Luttenberger, m.luttenberger@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Tests some of the methods in avmisc.h
*/

#include "avmisc.h"

// Qt includes
#include <QString>
#include <QTest>
#include <avunittestmain.h>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QVector>
#include <QHostInfo>
#include <QTemporaryDir>

// AVLib includes
#include "avconfig.h" // TODO QTP-10
#include "avlog.h"
#include "avlog/avlogmonitor.h"

#if defined(Q_OS_UNIX)
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif
#if defined(Q_OS_WIN32)
#include <winsock.h>
#endif

// constants


////////////////////////////////////////////////////////////////////////////////

class AVMiscTest : public QObject
{

    Q_OBJECT
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
public:
    AVMiscTest()
    : buf(0),
      tmppath(QString::null),
      dt(QDate::currentDate())
    {

    }


private:

    char *buf;
    QString tmppath;
    QDate dt;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

private slots:

    void init()
    {
        buf = new char [30];
        QVERIFY(buf != 0);
        tmppath = AVEnvironment::getApplicationHome() + "/tmp";
        dt = QDate::currentDate();
    }

///////////////////////////////////////////////////////////////////////////////

    void cleanup()
    {
        delete[] buf;
        buf = 0;
    }

///////////////////////////////////////////////////////////////////////////////

    void testAVHexDump() {
        for ( int i=0; i<30; i++ ) {
            buf[i] = i+28;
        }
        qDebug ("%s", qPrintable(AVHexDump((unsigned char*)buf, 30)));
        QString hexdump = AVHexDump ( (unsigned char*) buf, 30);
        QVERIFY2(hexdump != QString (
                "       0    1c 1d 1e 1f 20 21 22 23 24 25 26 27 28 29 2a "
                "2b      !\"#$%&'()*+\n"\
                "      10    2c 2d 2e 2f 30 31 32 33 34 35 36 37 38 39    "
                "   ,-./0123456789\n"), "output doesnt match");

    }

///////////////////////////////////////////////////////////////////////////////

    void testAVLogMultiLines()
    {
        QTemporaryDir temp_dir;

        AVLog logger1("test1", temp_dir.path());
        AVLog logger2("test2", temp_dir.path());

        AVLogMultiLines (LOG_INFO, "line1\nline2", &logger1);
        logger2.Write (LOG_INFO, "line1");
        logger2.Write (LOG_INFO, "line2");

        QFile test1f (logger1.curFileName());
        QFile test2f (logger2.curFileName());

        // compare the two log files
        QVERIFY( test1f.open(QIODevice::ReadOnly) && test2f.open(QIODevice::ReadOnly) );

        QTextStream t1 ( &test1f );
        QTextStream t2 ( &test2f );

        QVERIFY( !t1.atEnd() );
        QVERIFY( !t2.atEnd() );

        while ( !t1.atEnd() )
        {
            QVERIFY( !t2.atEnd() );
            QString s1 = t1.readLine().mid(9);
            QString s2 = t2.readLine().mid(9);
            QVERIFY(s1 == s2);
        }
        QVERIFY( t2.atEnd() );

        test1f.remove();
        test2f.remove();
    }

///////////////////////////////////////////////////////////////////////////////

    void testAVHostName ()
    {
        QString hname = AVHostName();
        QCOMPARE(hname,QHostInfo::localHostName());
        QVERIFY(!hname.isEmpty());

        AVLogInfo << "hname: " << hname;

        QHostAddress haddress;
        if(AVResolveHost (hname, haddress))
        {
            QVERIFY(!haddress.isNull());

            AVLogInfo << "ipaddress: " << haddress.toString();

            QHostInfo info = QHostInfo::fromName(hname);
            QVERIFY(info.error() == QHostInfo::NoError);
            QVERIFY(!info.addresses().isEmpty());
            QVERIFY(info.addresses().contains(haddress));
        }
        else
        {
            AVLogError << "testAVHostName: cannot resolve hostname on this machine";
        }
    }

///////////////////////////////////////////////////////////////////////////////

    void testDateTime () {
        QDate date (2003,7,28);
        QTime time (23,59,59);
        AVDateTime datetime (date,time);
        QVERIFY( AVPrintDateTime (datetime) == "20030728235959" );
        time.setHMS (0,0,1);
        datetime.setTime (time);
        QVERIFY( AVPrintDateTime (datetime) == "20030728000001" );
        QVERIFY( AVPrintDateTime (AVParseDateTime ("20030728235959"))
                         == "20030728235959" );
        QVERIFY( AVPrintDateTime (AVParseDateTime ("20030728"))
                         == "20030728000000" );
    }

///////////////////////////////////////////////////////////////////////////////

    void testIsModeAValid () {
        QVERIFY( isModeAValid(   0) );
        QVERIFY( isModeAValid(1234) );
        QVERIFY( isModeAValid(7654) );
        QVERIFY( isModeAValid(7777) );

        QVERIFY( !isModeAValid(1813) );
        QVERIFY( !isModeAValid(   8) );
        QVERIFY( !isModeAValid(9999) );
        QVERIFY( !isModeAValid(  81) );
    }

///////////////////////////////////////////////////////////////////////////////

    void testExtractBitsFromUInt32 () {
        const quint32 testvalue = 0xffffffffL;
        for (uint index=0; index < 32; ++index)
        {
            for(uint count=1; count < 32-index; ++count)
            {
                quint32 awaited_value = (2 << (count-1)) - 1;
                QCOMPARE(
                    awaited_value, extractBitsFromUInt32(testvalue, index, count));
            }
        }
    }

///////////////////////////////////////////////////////////////////////////////

    void testAVStringSplit () {
        QStringList result;
        QString input_simple = "abc;d;ef;;g;";

        result = AVStringSplit(';', input_simple, true);
        QCOMPARE( (uint)result.size(), (uint)6);
        QVERIFY(QString("abc") == result[0]);
        QVERIFY(QString("d")   == result[1]);
        QVERIFY(QString("ef")  == result[2]);
        QVERIFY(result[3].isNull());
        QVERIFY(QString("g")   == result[4]);
        QVERIFY(result[5].isNull());

        result = AVStringSplit(';', input_simple, false);
        QCOMPARE( (uint)result.size(), (uint)4);
        QVERIFY(QString("abc") == result[0]);
        QVERIFY(QString("d")   == result[1]);
        QVERIFY(QString("ef")  == result[2]);
        QVERIFY(QString("g")   == result[3]);

        input_simple = "abc;d;ef;;g";
        result = AVStringSplit(';', input_simple, true);
        QCOMPARE( (uint)result.size(), (uint)5);
        QVERIFY(QString("abc") == result[0]);
        QVERIFY(QString("d")   == result[1]);
        QVERIFY(QString("ef")  == result[2]);
        QVERIFY(result[3].isNull());
        QVERIFY(QString("g")   == result[4]);

        result = AVStringSplit(';', input_simple, false);
        QCOMPARE( (uint)result.size(), (uint)4);
        QVERIFY(QString("abc") == result[0]);
        QVERIFY(QString("d")   == result[1]);
        QVERIFY(QString("ef")  == result[2]);
        QVERIFY(QString("g")   == result[3]);

        QString input_quoted = "abc;\"d;ef\";;g;";
        result = AVStringSplit(';', input_quoted, true);
        QCOMPARE( (uint)result.size(), (uint)5);
        QVERIFY(QString("abc")  == result[0]);
        QVERIFY(QString("d;ef") == result[1]);
        QVERIFY(result[2].isNull());
        QVERIFY(QString("g")    == result[3]);
        QVERIFY(result[4].isNull());

        result = AVStringSplit(';', input_quoted, false);
        QCOMPARE( (uint)result.size(), (uint)3);
        QVERIFY(QString("abc")  == result[0]);
        QVERIFY(QString("d;ef") == result[1]);
        QVERIFY(QString("g")    == result[2]);

        result = AVStringSplit(';', input_quoted, true, '\"', true);
        QCOMPARE( (uint)result.size(), (uint)5);
        QVERIFY(QString("abc")  == result[0]);
        QString r = "\"d;ef";   // worksround for weird MSVC6
        r += "\"";              // compiler
        QVERIFY(r == result[1]);
        QVERIFY(result[2].isNull());
        QVERIFY(QString("g")    == result[3]);
        QVERIFY(result[4].isNull());
    }

///////////////////////////////////////////////////////////////////////////////

    void testSerializeDeserializeUINT8()
    {
        QByteArray ba;
        QDataStream w_stream(&ba, QIODevice::WriteOnly );
        QDataStream r_stream(&ba, QIODevice::ReadOnly );

        quint8 ui8 = 200;
        serializeUINT8(w_stream, ui8);
        QCOMPARE(ba.size(), 1);
        QCOMPARE(ba.at(0), (char)0xc8);
        quint8 ui8_target;
        deserializeUINT8(r_stream, ui8_target);
        QCOMPARE(ui8_target, ui8);
    }

///////////////////////////////////////////////////////////////////////////////

    void testSerializeDeserializeUINT16()
    {
        QByteArray ba;
        QDataStream w_stream(&ba, QIODevice::WriteOnly );
        QDataStream r_stream(&ba, QIODevice::ReadOnly );

        quint16 ui16 = 50000;
        serializeUINT16LH(w_stream, ui16);
        QCOMPARE(ba.size(), 2);
        QCOMPARE(ba.at(0), (char)0x50);
        QCOMPARE(ba.at(1), (char)0xc3);
        quint16 ui16_target;
        deserializeUINT16LH(r_stream, ui16_target);
        QCOMPARE(ui16_target, ui16);
    }

///////////////////////////////////////////////////////////////////////////////

    void testSerializeDeserializeINT16()
    {
        QByteArray ba;
        QDataStream w_stream(&ba, QIODevice::WriteOnly );
        QDataStream r_stream(&ba, QIODevice::ReadOnly );

        qint16 i16 = -30000;
        serializeINT16LH(w_stream, i16);
        QCOMPARE(ba.size(), 2);
        QCOMPARE(ba.at(0), (char)0xd0);
        QCOMPARE(ba.at(1), (char)0x8a);
        qint16 i16_target;
        deserializeINT16LH(r_stream, i16_target);
        QCOMPARE(i16_target, i16);
    }

///////////////////////////////////////////////////////////////////////////////

    void testSerializeDeserializeUINT32()
    {
        QByteArray ba;
        QDataStream w_stream(&ba, QIODevice::WriteOnly );
        QDataStream r_stream(&ba, QIODevice::ReadOnly );

        quint32 ui32 = 1164413202;
        serializeUINT32LH(w_stream, ui32);
        QCOMPARE(ba.size(), 4);
        QCOMPARE(ba.at(0), (char)0x12);
        QCOMPARE(ba.at(1), (char)0x89);
        QCOMPARE(ba.at(2), (char)0x67);
        QCOMPARE(ba.at(3), (char)0x45);
        quint32 ui32_target;
        deserializeUINT32LH(r_stream, ui32_target);
        QCOMPARE(ui32_target, ui32);
    }

///////////////////////////////////////////////////////////////////////////////

    void testAVBreakStr()
    {
        LOGGER_ROOT.Write(LOG_DEBUG, "AVMiscTest::testAVBreakStr");

        // Test degenerate cases
        QCOMPARE(AVBreakString(QString::null, 0), QStringList());
        QCOMPARE(AVBreakString(" ", 0), QStringList(" "));
        QCOMPARE(AVBreakString(" ", 5), QStringList(" "));
        QCOMPARE(AVBreakString("", 5), QStringList(""));

        // Test interesting cases
        const QString str1 = "1234567890 ";
        QCOMPARE(AVBreakString(str1, 5), (QStringList)(QStringList() << "12345" << "67890"));
        QCOMPARE(AVBreakString(str1, 9), (QStringList)(QStringList() << "123456789" << "0 "));
        QCOMPARE(AVBreakString(str1, 11), QStringList(str1));
        QCOMPARE(AVBreakString(str1, 20), QStringList(str1));
        QCOMPARE(AVBreakString(str1, 0, true), QStringList(str1));
        QCOMPARE(AVBreakString(str1, 11, true), QStringList(str1));

        const QString str2 = "12345 7890";
        QCOMPARE(AVBreakString(str2, 4), (QStringList)(QStringList() << "1234" << "5" << "7890"));
        QCOMPARE(AVBreakString(str2, 5), (QStringList)(QStringList() << "12345" << "7890"));
        QCOMPARE(AVBreakString(str2, 6), (QStringList)(QStringList() << "12345" << "7890"));
        QCOMPARE(AVBreakString(str2, 7), (QStringList)(QStringList() << "12345" << "7890"));
        QCOMPARE(AVBreakString(str2, 10), QStringList(str2));

        QCOMPARE(AVBreakString(str2, 4, true), (QStringList)(QStringList() << "12345" << "7890"));
        QCOMPARE(AVBreakString(str2, 5, true), (QStringList)(QStringList() << "12345" << "7890"));

        // Test regexp
        QRegExp break_re(";|/|\\|| ");
        const QString str3 = "12345/78901|34567;90123 ";

        QCOMPARE(AVBreakString(str3, 5, false, break_re),
                 (QStringList)(QStringList() << "12345" << "78901" << "34567" << "90123"));

        QCOMPARE(AVBreakString(str3, 4, false, break_re),
                (QStringList)(QStringList() << "1234" << "5" << "7890" << "1"
                << "3456" << "7" << "9012" << "3 "));

        QCOMPARE(AVBreakString(str3, 4, true, break_re),
                (QStringList)(QStringList() << "12345" << "78901" << "34567" << "90123"));
    }

    ///////////////////////////////////////////////////////////////////////////////

    QString testResolveHostHelper(const QString& itemToResolve)
    {
        QHostAddress ha;
        if (!AVResolveHost(itemToResolve, ha)) return QString::null;
        return ha.toString();
    }

    ///////////////////////////////////////////////////////////////////////////////

    void testResolveHost()
    {
        QCOMPARE(testResolveHostHelper("localhost"), QString("127.0.0.1"));
        QCOMPARE(testResolveHostHelper("1.2.3.4"), QString("1.2.3.4"));
        QCOMPARE(testResolveHostHelper("1.2.3.4.5"), QString());

        QCOMPARE(testResolveHostHelper("::1"), QString("::1"));
        QCOMPARE(testResolveHostHelper("1111:2222:3333:4444:5555:6666:7777:8888"), QString("1111:2222:3333:4444:5555:6666:7777:8888"));
        QCOMPARE(testResolveHostHelper("1111:2222:3333:4444:5555:6666:7777:8888:9999"), QString());
    }

    ///////////////////////////////////////////////////////////////////////////////

//    //! Test for resolving multiple entries in /etc/hosts
//    /*!
//     * To run this test put the following lines into /etc/hosts:
//     *
//     * 192.186.0.1 spidi blumi
//     * 192.186.0.1 pferdi
//     *
//     *
//     * Don't forget to stop the nscd daemon to disable name service cache
//     *
//     */
//    void testResolveHostMultipleEntries ()
//    {
//        QHostAddress hostaddress;
//
//        QVERIFY(AVResolveHost("spidi", hostaddress));
//        QCOMPARE(hostaddress, QHostAddress("192.186.0.1"));
//
//        QVERIFY(AVResolveHost("blumi", hostaddress));
//        QCOMPARE(hostaddress, QHostAddress("192.186.0.1"));
//
//        QVERIFY(AVResolveHost("pferdi", hostaddress));
//        QCOMPARE(hostaddress, QHostAddress("192.186.0.1"));
//
//        AVLogMonitor log_monitor(AVLogger);
//        AVLOG_MONITOR_START(log_monitor);
//
//        QVERIFY(!AVResolveHost("hausi", hostaddress));
//
//        log_monitor.filterOut(QString("AVResolveHost: lookup failed with error 1"), 1, 1,
//                                     AVLog::LOG__ERROR);
//        QVERIFY(log_monitor.stop());
//    }


// QT4PORT TODO ///////////////////////////////////////////////////////////////////////////////
// QT4PORT TODO
// QT4PORT TODO     //! Tests appendMemArray(QMemArray<Type>*, const QMemArray<Type>*, unsigned).
// QT4PORT TODO     void testAppendMemArray()
// QT4PORT TODO     {
// QT4PORT TODO         const char FILL_SIGN = 'A';
// QT4PORT TODO         unsigned old_dest_size = 0;
// QT4PORT TODO         QByteArray destination;
// QT4PORT TODO         QByteArray source;
// QT4PORT TODO
// QT4PORT TODO         source.fill(FILL_SIGN, 500);
// QT4PORT TODO         destination+=source;
// QT4PORT TODO         QVERIFY(destination == source);
// QT4PORT TODO         QCOMPARE(source.count(), destination.count());
// QT4PORT TODO         old_dest_size = destination.count();
// QT4PORT TODO
// QT4PORT TODO         source.fill(FILL_SIGN, 1000);
// QT4PORT TODO         destination+=source;
// QT4PORT TODO         QCOMPARE(source.count() + old_dest_size, destination.count());
// QT4PORT TODO     }
// QT4PORT TODO
// QT4PORT TODO ///////////////////////////////////////////////////////////////////////////////
// QT4PORT TODO
// QT4PORT TODO     //! Tests appendMemArray(QMemArray<Type>*, const QMemArray<Type>*, unsigned).
// QT4PORT TODO     void testAppendMemArray2()
// QT4PORT TODO     {
// QT4PORT TODO         const char FILL_SIGN = 'A';
// QT4PORT TODO         unsigned expected_dest_length = 0;
// QT4PORT TODO         QByteArray destination;
// QT4PORT TODO         QByteArray source;
// QT4PORT TODO
// QT4PORT TODO         source.fill(FILL_SIGN, 300);
// QT4PORT TODO         appendMemArray(&destination, &source, 0);
// QT4PORT TODO         QVERIFY(destination == source);
// QT4PORT TODO         QCOMPARE(source.count(), destination.count());
// QT4PORT TODO         expected_dest_length = destination.count();
// QT4PORT TODO
// QT4PORT TODO         appendMemArray(&destination, &source, 200);
// QT4PORT TODO         expected_dest_length += 100;
// QT4PORT TODO         QCOMPARE(expected_dest_length, destination.count());
// QT4PORT TODO
// QT4PORT TODO         appendMemArray(&destination, &source, 299);
// QT4PORT TODO         expected_dest_length += 1;
// QT4PORT TODO         QCOMPARE(expected_dest_length, destination.count());
// QT4PORT TODO     }
// QT4PORT TODO
// QT4PORT TODO ///////////////////////////////////////////////////////////////////////////////
// QT4PORT TODO
// QT4PORT TODO     //! Tests appendMemArray(QMemArray<Type>*, const QMemArray<Type>*,
// QT4PORT TODO                              unsigned, unsigned).
// QT4PORT TODO     void testAppendMemArray3()
// QT4PORT TODO     {
// QT4PORT TODO         const char FILL_SIGN = 'A';
// QT4PORT TODO         unsigned expected_dest_length = 0;
// QT4PORT TODO         QByteArray destination;
// QT4PORT TODO         QByteArray source;
// QT4PORT TODO
// QT4PORT TODO         source.fill(FILL_SIGN, 300);
// QT4PORT TODO         appendMemArray(&destination, &source, 0, source.count());
// QT4PORT TODO         QVERIFY(destination == source);
// QT4PORT TODO         QCOMPARE(source.count(), destination.count());
// QT4PORT TODO         expected_dest_length = destination.count();
// QT4PORT TODO
// QT4PORT TODO         appendMemArray(&destination, &source, 0, 100);
// QT4PORT TODO         expected_dest_length += 100;
// QT4PORT TODO         QCOMPARE(expected_dest_length, destination.count());
// QT4PORT TODO
// QT4PORT TODO         appendMemArray(&destination, &source, 200, 100);
// QT4PORT TODO         expected_dest_length += 100;
// QT4PORT TODO         QCOMPARE(expected_dest_length, destination.count());
// QT4PORT TODO
// QT4PORT TODO         appendMemArray(&destination, &source, 299, 1);
// QT4PORT TODO         expected_dest_length += 1;
// QT4PORT TODO         QCOMPARE(expected_dest_length, destination.count());
// QT4PORT TODO     }
// QT4PORT TODO
// QT4PORT TODO ///////////////////////////////////////////////////////////////////////////////
// QT4PORT TODO
// QT4PORT TODO     //! Tests appendMemArray(QMemArray<Type>*, const QMemArray<Type>*,
// QT4PORT TODO                              unsigned, unsigned).
// QT4PORT TODO     /**
// QT4PORT TODO      * This is a more semantic test than the previous one.
// QT4PORT TODO      */
// QT4PORT TODO     void testAppendMemArray4()
// QT4PORT TODO     {
// QT4PORT TODO         //--------------------------------------------------------------------------
// QT4PORT TODO         // create source array with ascending values
// QT4PORT TODO         QVector<unsigned> destination;
// QT4PORT TODO         QVector<unsigned> source(20);
// QT4PORT TODO         const unsigned values_to_copy = 5;
// QT4PORT TODO
// QT4PORT TODO         for (unsigned pos = 0; pos < source.count(); pos++)
// QT4PORT TODO             source.at(pos) = pos;
// QT4PORT TODO
// QT4PORT TODO         //--------------------------------------------------------------------------
// QT4PORT TODO         // append source[5] to source[10] to destination
// QT4PORT TODO         unsigned num = 5;
// QT4PORT TODO         appendMemArray(&destination, &source, num, values_to_copy);
// QT4PORT TODO         QCOMPARE(destination.count(), values_to_copy);
// QT4PORT TODO
// QT4PORT TODO         for (unsigned pos = 0; pos < destination.count(); pos++)
// QT4PORT TODO             QCOMPARE(destination.at(pos), num++);
// QT4PORT TODO
// QT4PORT TODO         QCOMPARE(num, 10u);
// QT4PORT TODO
// QT4PORT TODO         //--------------------------------------------------------------------------
// QT4PORT TODO         // append source[15] to source[20] to destination
// QT4PORT TODO         num += 5;
// QT4PORT TODO         appendMemArray(&destination, &source, num, values_to_copy);
// QT4PORT TODO         QCOMPARE(destination.count(), values_to_copy * 2);
// QT4PORT TODO
// QT4PORT TODO         for (unsigned pos = 5; pos < destination.count(); pos++)
// QT4PORT TODO             QCOMPARE(destination.at(pos), num++);
// QT4PORT TODO
// QT4PORT TODO         QCOMPARE(num, 20u);
// QT4PORT TODO     }
// QT4PORT TODO
// QT4PORT TODO ///////////////////////////////////////////////////////////////////////////////
// QT4PORT TODO
// QT4PORT TODO     //! Tests moveArrayData(QMemArray<Type>*, unsigned).
// QT4PORT TODO     void testMoveArrayDataTest()
// QT4PORT TODO     {
// QT4PORT TODO         const char FILL_SIGN = 'A';
// QT4PORT TODO         unsigned array_size = 50;
// QT4PORT TODO         QByteArray array;
// QT4PORT TODO         QVERIFY(array.fill(FILL_SIGN, array_size));
// QT4PORT TODO
// QT4PORT TODO         moveArrayData(&array, 10);
// QT4PORT TODO         array_size -= 10;
// QT4PORT TODO         QCOMPARE(array.count(), array_size);
// QT4PORT TODO
// QT4PORT TODO         moveArrayData(&array, 0);
// QT4PORT TODO         QCOMPARE(array.count(), array_size);
// QT4PORT TODO
// QT4PORT TODO         moveArrayData(&array, array_size);
// QT4PORT TODO         QVERIFY(array.isEmpty());
// QT4PORT TODO     }
// QT4PORT TODO
// QT4PORT TODO ///////////////////////////////////////////////////////////////////////////////
// QT4PORT TODO
// QT4PORT TODO     //! Tests moveArrayData(QMemArray<Type>*, unsigned).
// QT4PORT TODO     /**
// QT4PORT TODO      * This is a more semantic test than the previous one.
// QT4PORT TODO      */
// QT4PORT TODO     void testMoveArrayDataTest2()
// QT4PORT TODO     {
// QT4PORT TODO         QVector<unsigned> array(50);
// QT4PORT TODO
// QT4PORT TODO         for (unsigned count = 0; count < array.count(); count++)
// QT4PORT TODO             array.at(count) = count;
// QT4PORT TODO
// QT4PORT TODO         unsigned num = 10;
// QT4PORT TODO         moveArrayData(&array, num);
// QT4PORT TODO
// QT4PORT TODO         for (unsigned count = 0; count < array.count(); count++)
// QT4PORT TODO             QCOMPARE(array.at(count), num++);
// QT4PORT TODO
// QT4PORT TODO         num = 48;
// QT4PORT TODO         moveArrayData(&array, num - 10);
// QT4PORT TODO         QCOMPARE(array.count(), 2u);
// QT4PORT TODO
// QT4PORT TODO         for (unsigned count = 0; count < array.count(); count++)
// QT4PORT TODO             QCOMPARE(array.at(count), num++);
// QT4PORT TODO     }

///////////////////////////////////////////////////////////////////////////////

    void testEscape()
    {
        QString str(
                "\\"
                "n"
                "\\"
                "\n"
                "n"
                "\n"
                "\\"
                "\\");
        QStringList from, to;
        from.append("\\"); to.append("\\\\");
        from.append("\n"); to.append("\\n");
        QString escaped = AVEscape(str, from, to);
        QCOMPARE(escaped, QString(
                "\\\\"
                "n"
                "\\\\"
                "\\n"
                "n"
                "\\n"
                "\\\\"
                "\\\\"));

        // Verify backward conversion
        QCOMPARE(AVEscape(escaped, to, from), str);
    }

///////////////////////////////////////////////////////////////////////////////

    void testRunEventLoop()
    {
        AVLogInfo << "---- testRunEventLoop launched ----";

        for (uint i = 0; i < 100; ++i)
        {
            QDateTime start = AVDateTime::currentDateTimeUtc();

            qint64 past = runEventLoop(100);

            QDateTime end = AVDateTime::currentDateTimeUtc();

            qint64 span = start.msecsTo(end);

            QVERIFY(qAbs(past-span)<20);

            if(qAbs(past-100)>20)
            {
                AVLogWarning << "testRunEventLoop: running the event loop lasted "
                        "considerably different that intended: "<<past<<" instead of 100";
            }
        }

        AVLogInfo << "---- testRunEventLoop ended ----";
    }

///////////////////////////////////////////////////////////////////////////////

    void testFlagsMacros()
    {
        int flag1 = 0x0001;
        int flag2 = 0x0002;
        int var   = 0x0000;

        setflag (var, flag1);
        qDebug ("var=%d", var);
        QCOMPARE(var, flag1);
        QVERIFY( isflagset(var, flag1)              == true );
        QVERIFY( isflagset(var, flag2)              == false );
        QVERIFY( isflagset(var, flag1+flag2)        == false );
        QVERIFY( isflagset(var, flag1, false)       == true );
        QVERIFY( isflagset(var, flag2, false)       == false );
        QVERIFY( isflagset(var, flag1+flag2, false) == true );

        setflag (var, flag2);
        qDebug ("var=%d", var);
        QCOMPARE(var, flag1+flag2);
        QVERIFY( isflagset(var, flag1)              == true );
        QVERIFY( isflagset(var, flag2)              == true );
        QVERIFY( isflagset(var, flag1+flag2)        == true );
        QVERIFY( isflagset(var, flag1, false)       == true );
        QVERIFY( isflagset(var, flag2, false)       == true );
        QVERIFY( isflagset(var, flag1+flag2, false) == true );

        clearflag (var, flag1);
        qDebug ("var=%d", var);
        QCOMPARE(var, flag2);
        QVERIFY( isflagset(var, flag1)              == false );
        QVERIFY( isflagset(var, flag2)              == true );
        QVERIFY( isflagset(var, flag1+flag2)        == false );
        QVERIFY( isflagset(var, flag1, false)       == false );
        QVERIFY( isflagset(var, flag2, false)       == true );
        QVERIFY( isflagset(var, flag1+flag2, false) == true );
    }

    ///////////////////////////////////////////////////////////////////////////////

    void testListMinMax()
    {
        AVLogInfo << "---- testListMinMax launched ----";

        QList<double> list;
        list << 2.0 << -1.3 << 100.0 << 40.0;

        int index_of_max=1000;
        int index_of_min=1000;

        QCOMPARE(AVmax(list, index_of_max), 100.0);
        QCOMPARE(index_of_max, 2);
        QCOMPARE(AVmax(list), 100.0);

        QCOMPARE(AVmin(list, index_of_min), -1.3);
        QCOMPARE(index_of_min, 1);
        QCOMPARE(AVmin(list), -1.3);

        AVLogInfo << "---- testListMinMax ended ----";
    }

    ///////////////////////////////////////////////////////////////////////////////

    void testSetMinMax()
    {
        AVLogInfo << "---- testSetMinMax launched ----";

        QSet<int> set;
        set << 2 << -13 << 101 << 41;

        QCOMPARE(AVmax(set), 101);

        QCOMPARE(AVmin(set), -13);

        AVLogInfo << "---- testSetMinMax ended ----";
    }

    ///////////////////////////////////////////////////////////////////////////////

    void testAVsprintf()
    {
        AVLogInfo << "---- testAVsprintf launched ----";

        AVLogInfo << "locale: "<< std::locale("").name().c_str();
        AVLogInfo << "Qt locale: "<<QLocale::system().name();
        AVLogInfo << "Qt decimal point: "<<QLocale::system().decimalPoint();

        double val = 12.3;

        QString str;
        AVsprintf(str, "value: %.2f", val);

        QCOMPARE(str, QString("value: 12.30"));

        AVLogInfo << "---- testAVsprintf ended ----";
    }

    ///////////////////////////////////////////////////////////////////////////////

    void testQHash_std_shared_ptr()
    {
        class payload {};

        {
            std::shared_ptr<payload> ptr;
            QCOMPARE(qHash(ptr), qHash(ptr.get()));
        }

        {
            const std::shared_ptr<const payload> ptr2 = std::make_shared<payload>();
            QCOMPARE(qHash(ptr2), qHash(ptr2.get()));
        }

    }

};

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVMiscTest,"avlib/unittests/config")
#include "avmisctest.moc"


// End of file
