///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*!
  \file
  \author  DI Alexander Wemmer, a.wemmer@avibit.com
  \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
  \brief   Unit test for AVSet class
*/


#include "stdio.h"
#include <QTest>
#include <avunittestmain.h>

#include <QString>
#include <QStringList>
#include <QFile>
#include <QLinkedList>


#include "avlog.h"
#include "avset.h"


///////////////////////////////////////////////////////////////////////////////

class AVSetsUnitTest : public QObject
{
    Q_OBJECT



protected:

    AVSet<int>* int_set;
    AVSet<QString>* string_set;

public:

    AVSetsUnitTest() : int_set(0), string_set(0) {}

private slots:

    void init()
    {
        AVLogger->setMinLevel(AVLog::LOG__FATAL);

        int_set = new AVSet<int>();
        QVERIFY(int_set);

        string_set = new AVSet<QString>();
        QVERIFY(string_set);
    }

    //----------------------------------------

    void cleanup()
    {
        delete int_set;
        delete string_set;
    }

    //----------------------------------------

    void testIntSet()
    {
        int int1 = 1;
        int int2 = 2;
        int int3 = -3;

        QCOMPARE(int_set->count(), 0u);
        QVERIFY(!int_set->exists(int1));
        QVERIFY(!int_set->exists(int2));
        QVERIFY(!int_set->exists(int3));

        QVERIFY(!int_set->del(int1));
        QVERIFY(!int_set->del(int2));
        QVERIFY(!int_set->del(int3));
        QCOMPARE(int_set->count(), 0u);

        QVERIFY(!int_set->add(int1));
        QCOMPARE(int_set->count(), 1u);
        QVERIFY(!int_set->add(int2));
        QCOMPARE(int_set->count(), 2u);
        QVERIFY(!int_set->add(int3));
        QCOMPARE(int_set->count(), 3u);

        QVERIFY(int_set->exists(int1));
        QVERIFY(int_set->exists(int2));
        QVERIFY(int_set->exists(int3));

        QVERIFY(int_set->add(int1));
        QCOMPARE(int_set->count(), 3u);
        QVERIFY(int_set->add(int2));
        QCOMPARE(int_set->count(), 3u);
        QVERIFY(int_set->add(int3));
        QCOMPARE(int_set->count(), 3u);

        QVERIFY(int_set->del(int1));
        QCOMPARE(int_set->count(), 2u);
        QVERIFY(int_set->del(int2));
        QCOMPARE(int_set->count(), 1u);
        QVERIFY(int_set->del(int3));
        QCOMPARE(int_set->count(), 0u);

        QVERIFY(!int_set->exists(int1));
        QVERIFY(!int_set->exists(int2));
        QVERIFY(!int_set->exists(int3));

        QVERIFY(!int_set->del(int1));
        QVERIFY(!int_set->del(int2));
        QVERIFY(!int_set->del(int3));
        QCOMPARE(int_set->count(), 0u);

        QVERIFY(!int_set->add(int1));
        QVERIFY(!int_set->add(int2));
        QVERIFY(!int_set->add(int3));
        QCOMPARE(int_set->count(), 3u);

        AVSet<int> second = *int_set;
        QVERIFY(second.count() == 3);
        QVERIFY(second == *int_set);
        QVERIFY(second.exists(int1));
        QVERIFY(second.exists(int2));
        QVERIFY(second.exists(int3));

        second.clear();
        QVERIFY(second.count() == 0);
        QVERIFY(second != *int_set);
        QVERIFY(!second.exists(int1));
        QVERIFY(!second.exists(int2));
        QVERIFY(!second.exists(int3));

        second = *int_set;
        QVERIFY(second.count() == 3);
        QVERIFY(second == *int_set);
        QVERIFY(second.exists(int1));
        QVERIFY(second.exists(int2));
        QVERIFY(second.exists(int3));

        QVERIFY(second.del(int2));
        QVERIFY(second != *int_set);
        QVERIFY(!second.exists(int2));
        QVERIFY(int_set->exists(int2));
        QVERIFY(!second.add(int2));
        QVERIFY(second == *int_set);

        QList<int> list;
        list.append(int1);
        list.append(int2);
        list.append(int3);

        second.clear();
        QVERIFY(!second.addFromList(list));
        QVERIFY(second.count() == 3);
        QVERIFY(second == *int_set);
        QVERIFY(second.exists(int1));
        QVERIFY(second.exists(int2));
        QVERIFY(second.exists(int3));
        QVERIFY(second.addFromList(list));
    }

    //----------------------------------------

    void testStringSet()
    {
        QString string1 = "1";
        QString string2 = "2";
        QString string3 = "-3";

        QVERIFY(!string_set->exists(string1));
        QVERIFY(!string_set->exists(string2));
        QVERIFY(!string_set->exists(string3));

        QVERIFY(!string_set->del(string1));
        QVERIFY(!string_set->del(string2));
        QVERIFY(!string_set->del(string3));

        QVERIFY(!string_set->add(string1));
        QVERIFY(!string_set->add(string2));
        QVERIFY(!string_set->add(string3));

        QVERIFY(string_set->exists(string1));
        QVERIFY(string_set->exists(string2));
        QVERIFY(string_set->exists(string3));

        QVERIFY(string_set->add(string1));
        QVERIFY(string_set->add(string2));
        QVERIFY(string_set->add(string3));

        QVERIFY(string_set->del(string1));
        QVERIFY(string_set->del(string2));
        QVERIFY(string_set->del(string3));

        QVERIFY(!string_set->exists(string1));
        QVERIFY(!string_set->exists(string2));
        QVERIFY(!string_set->exists(string3));

        QVERIFY(!string_set->del(string1));
        QVERIFY(!string_set->del(string2));
        QVERIFY(!string_set->del(string3));

        QVERIFY(!string_set->add(string1));
        QVERIFY(!string_set->add(string2));
        QVERIFY(!string_set->add(string3));

        AVSet<QString> second = *string_set;
        QVERIFY(second.count() == 3);
        QVERIFY(second == *string_set);
        QVERIFY(second.exists(string1));
        QVERIFY(second.exists(string2));
        QVERIFY(second.exists(string3));

        second.clear();
        QVERIFY(second.count() == 0);
        QVERIFY(second != *string_set);
        QVERIFY(!second.exists(string1));
        QVERIFY(!second.exists(string2));
        QVERIFY(!second.exists(string3));

        second = *string_set;
        QVERIFY(second.count() == 3);
        QVERIFY(second == *string_set);
        QVERIFY(second.exists(string1));
        QVERIFY(second.exists(string2));
        QVERIFY(second.exists(string3));

        QVERIFY(second.del(string2));
        QVERIFY(second != *string_set);
        QVERIFY(!second.exists(string2));
        QVERIFY(string_set->exists(string2));
        QVERIFY(!second.add(string2));
        QVERIFY(second == *string_set);

        QStringList list;
        list.append(string1);
        list.append(string2);
        list.append(string3);

        second.clear();
        QVERIFY(!second.addFromList(list));
        QVERIFY(second.count() == 3);
        QVERIFY(second == *string_set);
        QVERIFY(second.exists(string1));
        QVERIFY(second.exists(string2));
        QVERIFY(second.exists(string3));
        QVERIFY(second.addFromList(list));
    }

    //----------------------------------------

    void testIterator()
    {
        int_set->add(1);
        int_set->add(2);
        int_set->add(4);
        int_set->add(8);

        QList<int> int_list;
        AVSetConstIterator<int> iter = int_set->begin();
        for(; iter != int_set->end(); ++iter) int_list.append(iter.key());

        QVERIFY(int_list == int_set->getSet());
        QVERIFY(int_list.count() == 4);
        QVERIFY(int_list[0] == 1);
        QVERIFY(int_list[1] == 2);
        QVERIFY(int_list[2] == 4);
        QVERIFY(int_list[3] == 8);
    }

    //----------------------------------------

    //! SWE-4513
    void testSubtract()
    {
        AVSet<int> test_set;
        test_set.addFromList(QList<int>() << 1 << 2 << 3 << 4 << 5);

        AVSet<int> remove_set;
        remove_set.addFromList(QList<int>() << 2 << 4);

        QCOMPARE(test_set.substract(remove_set).getSet(), QList<int>() << 1 << 3 << 5);
    }

    //----------------------------------------
};

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVSetsUnitTest,"avlib/unittests/config")
#include "avsetsunittest.moc"


// End of file
