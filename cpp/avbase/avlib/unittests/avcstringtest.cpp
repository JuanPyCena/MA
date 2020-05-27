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
    \author  Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief
*/


// QT includes

// AVCOMMON includes
#include <avcstring.h>
#include <QTest>
#include <avunittestmain.h>
#include <avdatastream.h>

// local includes
#include <type_traits>

//uncomment following lines to test deprecation
//#undef QT_WARNING_DISABLE_DEPRECATED
//#define QT_WARNING_DISABLE_DEPRECATED

///////////////////////////////////////////////////////////////////////////////

class AVCStringTest : public QObject
{
    Q_OBJECT



private:


private slots:

    void init()
    {
    }

    void cleanup()
    {
    }

    void testAVCString()
    {
        AVLogInfo << "----testAVCString launched----";
        QT_WARNING_PUSH
        QT_WARNING_DISABLE_DEPRECATED

        const char* test_string = "Bla bla bla 1234!";

        AVCString s1(test_string); //deprecated

        QVERIFY(test_string == s1);

        AVCString s2;
        s2=test_string;//deprecated

        QVERIFY(test_string == s2);

        QT_WARNING_POP
        AVLogInfo << "----testAVCString ended----";
    }

    void testAVCStringSerialization()
    {
        AVLogInfo << "----testAVCStringSerialization launched----";

        auto test_string = QByteArrayLiteral("Bla bla bla 1234!");

        AVCString s1(test_string);

        QByteArray buffer;

        QDataStream write_stream(&buffer, QIODevice::WriteOnly | QIODevice::Unbuffered);

        write_stream << s1;

        QDataStream read_stream(&buffer, QIODevice::ReadOnly | QIODevice::Unbuffered);

        AVCString s2;
        read_stream >> s2;

        QVERIFY(test_string == s2);
        QVERIFY(s1 == s2);

        // Using QDataStream the serialized value is not terminated with \0
        QVERIFY(!buffer.endsWith('\0'));

        QByteArray buffer2;

        AVDataStream write_stream2(&buffer2, QIODevice::WriteOnly | QIODevice::Unbuffered);

        write_stream2 << s1;

        AVDataStream read_stream2(&buffer2, QIODevice::ReadOnly | QIODevice::Unbuffered);

        AVCString s3;
        read_stream2 >> s3;

        QVERIFY(s3 == test_string);
        QVERIFY(s1 == s3);

        // Using AVDataStream the serialized value is terminated with \0
        // (compatibility with QCString)
        QVERIFY(buffer2.endsWith('\0'));


        AVLogInfo << "----testAVCStringSerialization ended----";
    }

    void testAVCStringSerializationNull()
    {
        AVLogInfo << "----testAVCStringSerializationNull launched----";

        QByteArray buffer;

        AVDataStream write_stream(&buffer, QIODevice::WriteOnly);

        AVCString null_string;
        write_stream << null_string;

        AVDataStream read_stream(&buffer, QIODevice::ReadOnly);

        AVCString s1;
        read_stream >> s1;

        QVERIFY(null_string == s1);
        QVERIFY(s1.isNull());

        // Only the qint32 of size must be in the buffer
        QVERIFY(buffer.size()==4);

        AVDataStream str(&buffer, QIODevice::ReadOnly);
        qint32 size;
        str>>size;
        QCOMPARE(size, 0);

        AVLogInfo << "----testAVCStringSerializationNull ended----";
    }

    void testAVCStringSerializationEmpty()
    {
        AVLogInfo << "----testAVCStringSerializationEmpty launched----";

        QByteArray buffer;

        AVDataStream write_stream(&buffer, QIODevice::WriteOnly);

        AVCString empty_string(QByteArrayLiteral(""));
        write_stream << empty_string;

        AVDataStream read_stream(&buffer, QIODevice::ReadOnly);

        AVCString s1;
        read_stream >> s1;

        QVERIFY(empty_string == s1);

        // The qint32 of size and the terminalization byte must be in the buffer
        QCOMPARE(buffer.size(), 5);
        QVERIFY(buffer.endsWith('\0'));
        buffer.chop(1);

        AVDataStream str(&buffer, QIODevice::ReadOnly);
        qint32 size;
        str>>size;
        QCOMPARE(size, 1);

        AVLogInfo << "----testAVCStringSerializationEmpty ended----";
    }

    void testAVCStringFromLiteral()
    {
        QT_WARNING_PUSH
        QT_WARNING_DISABLE_DEPRECATED
        AVLogInfo << "----testAVCStringFromLiteral launched----";

#if 0 //test compile error
        AVCString string = "asdf"; //error
        QVERIFY(string == "asdf");
#else
        AVCString string;
#endif
        string = "Bla"; //deprecated
        QVERIFY(string == "Bla");

        string = QByteArrayLiteral("asfd");
        QVERIFY(string == "asfd");
        QT_WARNING_POP

        AVLogInfo << "----testAVCStringFromLiteral ended----";
    }
};

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVCStringTest,"avlib/unittests/config")
#include "avcstringtest.moc"


// End of file
