///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*!
  \file
  \author  DI Alexander Wemmer, a.wemmer@avibit.com
  \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
  \brief   Unit test for the AVStringPairList class
*/


#include "stdio.h"
#include <QTest>
#include <avunittestmain.h>

#include <QString>
#include <QFile>


#include "avlog.h"
#include "avstringpairlist.h"


///////////////////////////////////////////////////////////////////////////////

class AVStringPairListUnitTest : public QObject
{
    Q_OBJECT



protected:

    QString test_data_file_name;
    QString test_data_file_name2;
    QString test_data_file_name3;
    QFile* test_data_file;
    QStringList* test_data_list;

    AVStringPairList *av_spl;

public:
    AVStringPairListUnitTest() :
        test_data_file(0), test_data_list(0), av_spl(0) {}


private slots:


    void init()
    {
        AVLogger->setMinLevel(AVLog::LOG__FATAL);

        test_data_file_name = "avstringpairlist.unittest.testdata";
        test_data_file_name2 = "avstringpairlist.unittest.testdata.2";
        test_data_file_name3 = "avstringpairlist.unittest.testdata.3";

        test_data_file = new QFile(test_data_file_name);
        QVERIFY(test_data_file);

        test_data_list = new QStringList();
        QVERIFY(test_data_list);

        test_data_list->append("a:1");
        test_data_list->append("b:2");
        test_data_list->append("c:3");
        test_data_list->append("d:4");
        test_data_list->sort();

        QVERIFY(test_data_file->open(QIODevice::WriteOnly | QIODevice::Truncate));
        QTextStream outstream(test_data_file);

        // insert comment line - should be ignored
        outstream << "#ignoreme\n" << "#\n";

        QStringList::Iterator iter = test_data_list->begin();
        for( ; iter != test_data_list->end(); ++iter)
        {
            outstream << qPrintable(*iter) << endl;
        }

        // insert comment line - should be ignored
        outstream << "#ignoreme\n" << "#\n";

        test_data_file->flush();
        test_data_file->close();

        av_spl = new AVStringPairList(test_data_file_name, ":");
        QVERIFY(av_spl);
    }

    //----------------------------------------

    void cleanup()
    {
        delete av_spl;
        delete test_data_file;
        delete test_data_list;

        QFile(test_data_file_name).remove();
        QFile(test_data_file_name2).remove();
        QFile(test_data_file_name3).remove();
    }

    //----------------------------------------

    void testLoadGetClearSaveList()
    {
        QStringList stringlist;

        // no entries on start?
        QVERIFY(av_spl->getNrPairs() == 0);
        av_spl->getPairs(stringlist);
        QVERIFY(stringlist.count() == 0);

        // loading entries from file
        QVERIFY(av_spl->loadPairs());
        QVERIFY(av_spl->getNrPairs() == (uint)test_data_list->count());

        // loaded correct?
        av_spl->getPairs(stringlist);
        QVERIFY(stringlist.count() == test_data_list->count());
        stringlist.sort();
        QVERIFY(stringlist == *test_data_list);

        // set new datafile correct?
        QVERIFY(av_spl->setFileName(test_data_file_name2));
        av_spl->getPairs(stringlist);
        QVERIFY(stringlist.count() == test_data_list->count());
        stringlist.sort();
        QVERIFY(stringlist == *test_data_list);

        // save data to new file
        av_spl->savePairs();
        av_spl->getPairs(stringlist);
        QVERIFY(stringlist.count() == test_data_list->count());
        stringlist.sort();
        QVERIFY(stringlist == *test_data_list);

        // clear data
        av_spl->clearPairs();
        QVERIFY(av_spl->getNrPairs() == 0);
        av_spl->getPairs(stringlist);
        QVERIFY(stringlist.count() == 0);

        // load from new file
        QVERIFY(av_spl->loadPairs());
        QVERIFY(av_spl->getNrPairs() == (uint)test_data_list->count());

        // are data still the same?
        av_spl->getPairs(stringlist);
        QVERIFY(stringlist.count() == test_data_list->count());
        stringlist.sort();
        QVERIFY(stringlist == *test_data_list);

        // clear data and save the zero data to file
        av_spl->clearPairs();
        av_spl->savePairs();
        QVERIFY(av_spl->getNrPairs() == 0);
        av_spl->getPairs(stringlist);
        QVERIFY(stringlist.count() == 0);

        // load the zero data from file and check
        QVERIFY(av_spl->loadPairs());
        QVERIFY(av_spl->getNrPairs() == 0);
        av_spl->getPairs(stringlist);
        QVERIFY(stringlist.count() == 0);

        // set again old datafile, load data and check
        QVERIFY(av_spl->setFileName(test_data_file_name));
        QVERIFY(av_spl->loadPairs());
        QVERIFY(av_spl->getNrPairs() == (uint)test_data_list->count());
        av_spl->getPairs(stringlist);
        QVERIFY(stringlist.count() == test_data_list->count());
        stringlist.sort();
        QVERIFY(stringlist == *test_data_list);

        // set empty datafilenmae
        QVERIFY(! av_spl->setFileName(""));
        av_spl->getPairs(stringlist);
        QVERIFY(stringlist.count() == test_data_list->count());
        stringlist.sort();
        QVERIFY(stringlist == *test_data_list);
    }

    //----------------------------------------

    void testPairRemove()
    {
        av_spl->loadPairs();

        QString left;
        QString right;
        QString res;

        left = "a";
        right = "1";
        QVERIFY(av_spl->removePairByLeft(left));
        QVERIFY(av_spl->getNrPairs() == (uint)test_data_list->count() - 1);
        res = av_spl->getRightByLeft(left);
        QVERIFY(res == "");
        res = av_spl->getLeftByRight(right);
        QVERIFY(res == "");

        //-----

        left = "b";
        right = "2";
        QVERIFY(av_spl->removePairByRight(right));
        QVERIFY(av_spl->getNrPairs() == (uint)test_data_list->count() - 2);
        res = av_spl->getRightByLeft(left);
        QVERIFY(res == "");
        res = av_spl->getLeftByRight(right);
        QVERIFY(res == "");

        //-----

        left = "a";
        right = "1";
        QVERIFY(! av_spl->removePairByLeft(left));
        QVERIFY(! av_spl->removePairByRight(right));

        left = "b";
        right = "2";
        QVERIFY(! av_spl->removePairByLeft(left));
        QVERIFY(! av_spl->removePairByRight(right));

        left = "";
        right = "";
        QVERIFY(! av_spl->removePairByLeft(left));
        QVERIFY(! av_spl->removePairByRight(right));

        QVERIFY(av_spl->getNrPairs() == (uint)test_data_list->count() - 2);

        //-----

        left = "c";
        QVERIFY(av_spl->removePairByLeft(left));
        QVERIFY(av_spl->getNrPairs() == 1);

        QStringList stringlist;
        av_spl->getPairs(stringlist);
        QVERIFY(stringlist.count() == 1);
    }

    //----------------------------------------

    void testPairSet()
    {
        av_spl->loadPairs();

        QString left;
        QString right;
        QString res;

        QStringList stringlist;

        left = "a";
        right = "11";
        QVERIFY(av_spl->addPair(left, right));
        QVERIFY(av_spl->getNrPairs() == (uint)test_data_list->count());
        res = av_spl->getRightByLeft(left);
        QVERIFY(res == right);
        res = av_spl->getLeftByRight(right);
        QVERIFY(res == left);

        //-----

        left = "x";
        right = "99";
        QVERIFY(av_spl->addPair(left, right));
        QVERIFY(av_spl->getNrPairs() == (uint)test_data_list->count() + 1);

        //-----

        left = "";
        right = "";
        QVERIFY(!av_spl->addPair(left, right));
        QVERIFY(av_spl->getNrPairs() == (uint)test_data_list->count() + 1);
    }

    //----------------------------------------

    void testPairSeparator()
    {
        av_spl->loadPairs();

        QString left;
        QString right;
        QStringList stringlist;

        QVERIFY(av_spl->setFileName(test_data_file_name3));

        // set separtor to existing char
        QVERIFY(!av_spl->setSeparator("a"));

        // set another new separtor
        QVERIFY(av_spl->setSeparator("##"));

        // save data to new file
        av_spl->savePairs();
        QVERIFY(av_spl->getNrPairs() == (uint)test_data_list->count());
        av_spl->getPairs(stringlist);
        QVERIFY(stringlist.count() == test_data_list->count());

        // clear data
        av_spl->clearPairs();
        QVERIFY(av_spl->getNrPairs() == 0);
        av_spl->getPairs(stringlist);
        QVERIFY(stringlist.count() == 0);

        // loading entries from file
        QVERIFY(av_spl->loadPairs());
        QVERIFY(av_spl->getNrPairs() == (uint)test_data_list->count());

        // loaded correct?
        QVERIFY(av_spl->getNrPairs() == (uint)test_data_list->count());
        av_spl->getPairs(stringlist);
        QVERIFY(stringlist.count() == test_data_list->count());

        // clear data
        av_spl->clearPairs();
        QVERIFY(av_spl->getNrPairs() == 0);
        av_spl->getPairs(stringlist);
        QVERIFY(stringlist.count() == 0);

        QVERIFY(av_spl->setSeparator(":"));

        // add a pair
        left = "x";
        right = "99";
        QVERIFY(av_spl->addPair(left, right));
        QVERIFY(av_spl->getNrPairs() == 1);

        // loading entries from file
        QVERIFY(!av_spl->loadPairs());

        QVERIFY(av_spl->getNrPairs() == 1);
    }

    //----------------------------------------

    void testRegexpMatchSearch()
    {
        QString test_left1 = "B.*";
        QString test_right1 = "GAW";

        QString test_left2 = "B42";
        QString test_right2 = "GAE";

        QString left_entry_to_match = "B42";
        QString matching_left_entry;
        QString found_right_entry;

        // fill in some values

        QVERIFY(av_spl->addPair("something", "else"));
        QVERIFY(av_spl->getNrPairs() == 1);
        QVERIFY(av_spl->addPair(".just*", "another"));
        QVERIFY(av_spl->getNrPairs() == 2);

        // test unique search with one match

        QVERIFY(av_spl->addPair(test_left1, test_right1));
        QVERIFY(av_spl->getNrPairs() == 3);

        QVERIFY(
            av_spl->findMatchingLeftEntryByRegExp(
                left_entry_to_match, true, matching_left_entry, found_right_entry));

        QVERIFY(matching_left_entry == test_left1);
        QVERIFY(found_right_entry == test_right1);

        // test non-unique search with one match

        QVERIFY(
            av_spl->findMatchingLeftEntryByRegExp(
                left_entry_to_match, false, matching_left_entry, found_right_entry));

        QVERIFY(matching_left_entry == test_left1);
        QVERIFY(found_right_entry == test_right1);

        // test unique search with multi match

        QVERIFY(av_spl->addPair(test_left2, test_right2));
        QVERIFY(av_spl->getNrPairs() == 4);

        QVERIFY(!av_spl->findMatchingLeftEntryByRegExp(
                           left_entry_to_match, true, matching_left_entry, found_right_entry));

        QVERIFY(matching_left_entry.isEmpty());
        QVERIFY(found_right_entry.isEmpty());

        // test non-unique search with multi match

        QVERIFY(
            av_spl->findMatchingLeftEntryByRegExp(
                left_entry_to_match, false, matching_left_entry, found_right_entry));

        QVERIFY(matching_left_entry == test_left1 || matching_left_entry == test_left2);

        if (matching_left_entry == test_left1)
        {
            QVERIFY(found_right_entry == test_right1);
        }
        else if (matching_left_entry == test_left2)
        {
            QVERIFY(found_right_entry == test_right2);
        }
    }

    //----------------------------------------

    void printQStringList(QStringList* list)
    {
        qDebug("========== QStringList ==========");
        QStringList::Iterator iter = list->begin();
        for( ; iter != list->end(); ++iter)
        {
            qDebug("|%s|", qPrintable(*iter));
        }
        qDebug("=================================");
    }

    //----------------------------------------
};

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVStringPairListUnitTest,"avlib/unittests/config")
#include "avstringpairlistunittest.moc"


// End of file
