///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Kevin Krammer, k.krammer@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Testinf correct parsing, sorting and accessing of a serverlist
*/



#include "avmisc.h"
#include <QTest>
#include <avunittestmain.h>
#include <avserverlist.h>


///////////////////////////////////////////////////////////////////////////////

class AVServerListTest : public QObject
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

    void testParsing()
    {
        AVServerList* serverList;

        serverList = new (LOG_HERE) AVServerList("localhost", 1111);
        QVERIFY(serverList != 0);
        QVERIFY( serverList->count() == 1 );
        delete serverList;

        serverList = new (LOG_HERE) AVServerList("localhost#2222", 1111);
        QVERIFY(serverList != 0);
        QVERIFY( serverList->count() == 1 );
        QVERIFY( serverList->getPort() == 2222 );
        delete serverList;

        serverList = new (LOG_HERE) AVServerList("localhost;3", 1111);
        QVERIFY(serverList != 0);
        QVERIFY( serverList->count() == 1 );
        QVERIFY( serverList->getPort() == 1111 );
        QVERIFY( serverList->getPriority() == 3 );
        delete serverList;

        serverList = new (LOG_HERE) AVServerList("localhost#2222;3", 1111);
        QVERIFY(serverList != 0);
        QVERIFY( serverList->count() == 1 );
        QVERIFY( serverList->getPort() == 2222 );
        QVERIFY( serverList->getPriority() == 3 );
        delete serverList;

        serverList = new (LOG_HERE) AVServerList("localhost/server2", 1111);
        QVERIFY(serverList != 0);
        QVERIFY( serverList->count() == 2 );
        delete serverList;

        serverList = new (LOG_HERE) AVServerList("localhost;3/server2", 1111);
        QVERIFY(serverList != 0);
        QVERIFY( serverList->count() == 2 );
        delete serverList;

        serverList = new (LOG_HERE) AVServerList("localhost;3/server2;3", 1111);
        QVERIFY(serverList != 0);
        QVERIFY( serverList->count() == 2 );
        delete serverList;

        serverList = new (LOG_HERE) AVServerList("localhost#;3/server2;3", 1111);
        QVERIFY(serverList != 0);
        QVERIFY( serverList->count() == 1 );
        delete serverList;

        serverList = new (LOG_HERE) AVServerList("localhost#9999;3/server2;3", 1111);
        QVERIFY(serverList != 0);
        QVERIFY( serverList->count() == 2 );
        delete serverList;

        const char* servers = "server1;1/server2#2222;2/server3/server4;/server5;3/server6#;4";
        serverList = new (LOG_HERE) AVServerList(servers, 1111);
        QVERIFY(serverList != 0);
        QVERIFY( serverList->count() == 4 );

        uint count = 0;
        while (serverList->getPriority() != -1)
        {
            QVERIFY( serverList->getPort() != -1 );
            QVERIFY( serverList->getServer() != QString::null );

            if (serverList->getServer() == "server2")
                QVERIFY( serverList->getPort() == 2222 );
            else
                QVERIFY( serverList->getPort() == 1111 );

            if (serverList->getServer() == "server3")
                QVERIFY( serverList->getPriority() == 1 );

            serverList->next();
            count++;
        }

        QVERIFY( count == serverList->count() );

        count = 0;
        serverList->first();
        while (serverList->getPort() != -1)
        {
            QVERIFY( serverList->getPriority() != -1 );
            QVERIFY( serverList->getServer() != QString::null );
            serverList->next();
            count++;
        }

        QVERIFY( count == serverList->count() );

        count = 0;
        serverList->first();
        while (serverList->getServer() != QString::null)
        {
            QVERIFY( serverList->getPriority() != -1 );
            QVERIFY( serverList->getPort() != -1 );
            serverList->next();
            count++;
        }

        QVERIFY( count == serverList->count() );
        delete serverList;
    }

    void testSorting()
    {
        const char* servers = "server1;1/server2#2222;2/server3/server4;/server5;3/"
                              "server6#;4/server6;2/server7;1";
        AVServerList serverList(servers, 1111);
        serverList.first();
        QVERIFY( serverList.getServer() == "server5" );
        QVERIFY( serverList.getPriority() == serverList.maxPriority());
        QVERIFY( serverList.getPriority() == 3 );

        int lastPrio = serverList.maxPriority();
        uint prio1 = 0;
        uint prio2 = 0;
        uint prio3 = 0;
        uint other = 0;
        while (serverList.getPort() != -1)
        {
            QVERIFY( serverList.getPriority() <= lastPrio );
            switch (serverList.getPriority())
            {
                case 1: prio1++;
                    break;
                case 2: prio2++;
                    break;
                case 3: prio3++;
                    break;
                default: other++;
                    break;
            }
            serverList.next();
        }

        QVERIFY( prio1 == 3 );
        QVERIFY( prio2 == 2 );
        QVERIFY( prio3 == 1 );
        QVERIFY( other == 0 );

        lastPrio = serverList.maxPriority();
        serverList.first();
        while (serverList.getPort() != -1)
        {
            QVERIFY( serverList.getPriority() <= lastPrio );
            serverList.nextAndRandom();
        }

        serverList.firstAndRandom();
        QVERIFY( serverList.getServer() == "server5" );
    }

};

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVServerListTest,"avlib/unittests/config")
#include "avserverlisttest.moc"


// End of file
