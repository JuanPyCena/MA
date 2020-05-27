///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Alex Wemmer
    \brief   Tests some of the methods in avipaddress.h
*/


// Qt includes
#include <QString>
#include <QTest>
#include <avunittestmain.h>

// AVLib includes
#include "avipaddress.h"

////////////////////////////////////////////////////////////////////////////////

class AVIpAddressTest : public QObject
{
    Q_OBJECT

public:
    AVIpAddressTest()
    {
    }

private slots:

    void init()
    {
    }

    void cleanup()
    {
    }

    void testValidIpAddress()
    {
        QVERIFY(AVIPAddress::isValid("1.2.3.4"));
        QVERIFY(!AVIPAddress::isValid("1.2.3.4.5"));
        QVERIFY(!AVIPAddress::isValid("1.2.3.256"));

        QVERIFY(AVIPAddress::isValid("1:2:3:4:5:6:7:8"));
        QVERIFY(!AVIPAddress::isValid("1:2:3:4:5:6:7:8:9"));
        QVERIFY(!AVIPAddress::isValid("1:2:3:4:5:6:7:efgh"));
    }

    void testMulticastAddress()
    {
        QVERIFY(AVIPAddress::isMulticast("239.193.0.1"));
        QVERIFY(!AVIPAddress::isMulticast("1.2.3.4"));
        QVERIFY(!AVIPAddress::isMulticast("1.2.3.4.5"));
        QVERIFY(!AVIPAddress::isMulticast("240.0.0.1"));

        QVERIFY(AVIPAddress::isMulticast("ff00:2:3:4:5:6:7:8"));
        QVERIFY(!AVIPAddress::isMulticast("1:2:3:4:5:6:7:8"));
        QVERIFY(!AVIPAddress::isMulticast("ff00:2:3:4:5:6:7:8:9"));
        QVERIFY(!AVIPAddress::isMulticast("ff00:2:3:4:5:6:7:efgh"));
    }
};

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVIpAddressTest,"avlib/unittests/config")
#include "avipaddresstest.moc"


// End of file
