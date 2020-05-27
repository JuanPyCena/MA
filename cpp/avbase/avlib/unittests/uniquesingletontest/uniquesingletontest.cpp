//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2015
//
// Module:    TESTS - AVCOMMON Unit Tests
//
//-----------------------------------------------------------------------------

/*! \file
    \brief
 */

// local includes
#include "testsingletonuser1.h"
#include "testsingletonuser2.h"
#include "testsingleton.h"

// Qt includes

// avlib includes


// AVCOMMON includes
#include <QTest>
#include <avunittestmain.h>

// local includes



///////////////////////////////////////////////////////////////////////////////

class UniqueSingletonTest : public QObject
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

    void testSingleton()
    {
        TestSingleton* singleton = TestSingleton::getInstance();

        singleton->print("main");

        TestSingletonUser1 user1;

        TestSingletonUser2 user2;


    }



};

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(UniqueSingletonTest,"avlib/unittests/config")
#include "uniquesingletontest.moc"




// End of file
