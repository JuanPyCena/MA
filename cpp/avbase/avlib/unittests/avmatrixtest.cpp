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
  \brief   Unit test for the AVMatrix class
*/


#include "stdio.h"
#include <QTest>
#include <avunittestmain.h>

#include <QString>
#include <QFile>


#include "avlog.h"
#include "avmatrix.h"


#define TEST_INIT_VAL -10
#define TEST_INIT_VAL2 -11

#define TEST_VAL11 2
#define TEST_VAL12 13
#define TEST_VAL21 7
#define TEST_VAL22 99

///////////////////////////////////////////////////////////////////////////////

class AVMatrixUnitTest : public QObject
{
    Q_OBJECT



protected:

    int rows;
    int columns;
    AVMatrix<int> *av_matrix;

public:
    AVMatrixUnitTest() : rows(0), columns(0), av_matrix(0) {}

private slots:


    void init()
    {
        rows = 2;
        columns = 2;
        av_matrix = new AVMatrix<int>(rows, columns, TEST_INIT_VAL);
        QVERIFY(av_matrix);
    }

    //----------------------------------------

    void cleanup()
    {
        delete av_matrix;
        av_matrix = 0;
    }

    //----------------------------------------

    void testMatrixCreate()
    {
        QVERIFY(av_matrix->getRows() == rows);
        QVERIFY(av_matrix->getColumns() == columns);

        QVERIFY(*av_matrix == *av_matrix);
        QVERIFY(!(*av_matrix != *av_matrix));

        QVERIFY(av_matrix->get(0, 0) == TEST_INIT_VAL);
        QVERIFY(av_matrix->get(0, 1) == TEST_INIT_VAL);
        QVERIFY(av_matrix->get(1, 0) == TEST_INIT_VAL);
        QVERIFY(av_matrix->get(1, 1) == TEST_INIT_VAL);

        QVERIFY(av_matrix->getInitVal() == TEST_INIT_VAL);
    }

    //----------------------------------------

    void testMatrixCopy()
    {
        av_matrix->set(0, 0, TEST_VAL11);
        av_matrix->set(0, 1, TEST_VAL12);
        av_matrix->set(1, 0, TEST_VAL21);
        av_matrix->set(1, 1, TEST_VAL22);

        QVERIFY(av_matrix->get(0, 0) == TEST_VAL11);
        QVERIFY(av_matrix->get(0, 1) == TEST_VAL12);
        QVERIFY(av_matrix->get(1, 0) == TEST_VAL21);
        QVERIFY(av_matrix->get(1, 1) == TEST_VAL22);

        // copy constructor

        AVMatrix<int>* copy_matrix = new AVMatrix<int>(*av_matrix);
        QVERIFY(copy_matrix);

        QVERIFY(copy_matrix->getRows() == rows);
        QVERIFY(copy_matrix->getColumns() == columns);

        QVERIFY(*copy_matrix == *copy_matrix);
        QVERIFY(!(*copy_matrix != *copy_matrix));

        QVERIFY(*av_matrix == *copy_matrix);
        QVERIFY(!(*av_matrix != *copy_matrix));

        // copy operator

        delete copy_matrix;
        copy_matrix = new AVMatrix<int>(rows, columns, TEST_INIT_VAL);
        QVERIFY(copy_matrix);
        *copy_matrix = *av_matrix;

        QVERIFY(copy_matrix->getRows() == rows);
        QVERIFY(copy_matrix->getColumns() == columns);

        QVERIFY(*copy_matrix == *copy_matrix);
        QVERIFY(!(*copy_matrix != *copy_matrix));

        QVERIFY(*av_matrix == *copy_matrix);
        QVERIFY(!(*av_matrix != *copy_matrix));

        delete copy_matrix;
    }

    //----------------------------------------

    void testMatrixReset()
    {
        av_matrix->set(0, 0, TEST_VAL11);
        av_matrix->set(0, 1, TEST_VAL12);
        av_matrix->set(1, 0, TEST_VAL21);
        av_matrix->set(1, 1, TEST_VAL22);

        QVERIFY(av_matrix->get(0, 0) == TEST_VAL11);
        QVERIFY(av_matrix->get(0, 1) == TEST_VAL12);
        QVERIFY(av_matrix->get(1, 0) == TEST_VAL21);
        QVERIFY(av_matrix->get(1, 1) == TEST_VAL22);

        av_matrix->reset(3, 3, TEST_INIT_VAL2);

        QVERIFY(av_matrix->get(0, 0) == TEST_INIT_VAL2);
        QVERIFY(av_matrix->get(0, 1) == TEST_INIT_VAL2);
        QVERIFY(av_matrix->get(0, 2) == TEST_INIT_VAL2);
        QVERIFY(av_matrix->get(1, 0) == TEST_INIT_VAL2);
        QVERIFY(av_matrix->get(1, 1) == TEST_INIT_VAL2);
        QVERIFY(av_matrix->get(1, 2) == TEST_INIT_VAL2);
        QVERIFY(av_matrix->get(2, 0) == TEST_INIT_VAL2);
        QVERIFY(av_matrix->get(2, 1) == TEST_INIT_VAL2);
        QVERIFY(av_matrix->get(2, 2) == TEST_INIT_VAL2);

        QVERIFY(av_matrix->getInitVal() == TEST_INIT_VAL2);

        av_matrix->set(0, 0, TEST_VAL11);
        av_matrix->set(0, 1, TEST_VAL12);
        av_matrix->set(0, 2, TEST_VAL12);
        av_matrix->set(1, 0, TEST_VAL21);
        av_matrix->set(1, 1, TEST_VAL22);
        av_matrix->set(1, 2, TEST_VAL12);
        av_matrix->set(2, 0, TEST_VAL21);
        av_matrix->set(2, 1, TEST_VAL22);
        av_matrix->set(2, 2, TEST_VAL12);

        QVERIFY(av_matrix->get(0, 0) == TEST_VAL11);
        QVERIFY(av_matrix->get(0, 1) == TEST_VAL12);
        QVERIFY(av_matrix->get(0, 2) == TEST_VAL12);
        QVERIFY(av_matrix->get(1, 0) == TEST_VAL21);
        QVERIFY(av_matrix->get(1, 1) == TEST_VAL22);
        QVERIFY(av_matrix->get(1, 2) == TEST_VAL12);
        QVERIFY(av_matrix->get(2, 0) == TEST_VAL21);
        QVERIFY(av_matrix->get(2, 1) == TEST_VAL22);
        QVERIFY(av_matrix->get(2, 2) == TEST_VAL12);
    }

    //----------------------------------------

    void testMatrixFromAndToStringList()
    {
        AVMatrix<QString> matrix1(0, 0,0);
        QCOMPARE(matrix1.getRows(), (int)0);
        QCOMPARE(matrix1.getColumns(), (int)0);
        QStringList list1a;
        list1a.push_back("0;1;2;3");
        list1a.push_back("10;11;12;13");
        list1a.push_back("20;21;22;23");
        QVERIFY(matrix1.read(list1a, ";"));
        QCOMPARE(matrix1.getRows(), (int)3);
        QCOMPARE(matrix1.getColumns(), (int)4);
        QCOMPARE(matrix1.get(0, 0),QString("0"));
        QCOMPARE(matrix1.get(0, 1),QString("1"));
        QCOMPARE(matrix1.get(0, 2),QString("2"));
        QCOMPARE(matrix1.get(0, 3),QString("3"));
        QCOMPARE(matrix1.get(1, 0),QString("10"));
        QCOMPARE(matrix1.get(1, 1),QString("11"));
        QCOMPARE(matrix1.get(1, 2),QString("12"));
        QCOMPARE(matrix1.get(1, 3),QString("13"));
        QCOMPARE(matrix1.get(2, 0),QString("20"));
        QCOMPARE(matrix1.get(2, 1),QString("21"));
        QCOMPARE(matrix1.get(2, 2),QString("22"));
        QCOMPARE(matrix1.get(2, 3),QString("23"));
        QStringList list1b;
        QVERIFY(matrix1.write(list1b, ";"));
        QVERIFY(list1a == list1b);

        AVMatrix<int> matrix2(0, 0,0);
        QVERIFY(matrix2.read(list1a, ";"));
        QCOMPARE(matrix2.getRows(), (int)3);
        QCOMPARE(matrix2.getColumns(), (int)4);
        QCOMPARE(matrix2.get(0, 0),0);
        QCOMPARE(matrix2.get(0, 1),1);
        QCOMPARE(matrix2.get(0, 2),2);
        QCOMPARE(matrix2.get(0, 3),3);
        QCOMPARE(matrix2.get(1, 0),10);
        QCOMPARE(matrix2.get(1, 1),11);
        QCOMPARE(matrix2.get(1, 2),12);
        QCOMPARE(matrix2.get(1, 3),13);
        QCOMPARE(matrix2.get(2, 0),20);
        QCOMPARE(matrix2.get(2, 1),21);
        QCOMPARE(matrix2.get(2, 2),22);
        QCOMPARE(matrix2.get(2, 3),23);
        list1b.clear();
        QVERIFY(matrix2.write(list1b, ";"));
        QVERIFY(list1a == list1b);

        AVMatrix<int> matrix3(0, 0,0);
        QVERIFY(matrix3.read(QStringList("1;-1;0;-9999"), ";") == true);
        QCOMPARE(matrix3.get(0, 0),1);
        QCOMPARE(matrix3.get(0, 1),-1);
        QCOMPARE(matrix3.get(0, 2),0);
        QCOMPARE(matrix3.get(0, 3),-9999);
        QVERIFY(matrix3.read(QStringList("  2"),  ";")         == true);
        QCOMPARE(matrix3.get(0, 0),2);
        QVERIFY(matrix3.read(QStringList("  3;"), ";", true)   == true);
        QCOMPARE(matrix3.get(0, 0),3);
        QCOMPARE(matrix3.get(0, 1),0);
        int logLevel = AVLogger->minLevel();
        AVLogger->setMinLevel(AVLog::LOG__ERROR);
        QVERIFY(matrix3.read(QStringList("1.0"),  ";")        == false);
        QVERIFY(matrix3.read(QStringList("1;"),   ";")        == false);
        QVERIFY(matrix3.read(QStringList("x"),    ";")        == false);
        QVERIFY(matrix3.read(QStringList("1x"),   ";")        == false);
        QVERIFY(matrix3.read(QStringList("1-"),   ";")        == false);
        QVERIFY(matrix3.read(QStringList("TEST"), ";")        == false);
        AVLogger->setMinLevel(logLevel);

        AVMatrix<uint> matrix4(0, 0,0);
        QVERIFY(matrix4.read(QStringList("2  "),  ";")         == true);
        QVERIFY(matrix4.read(QStringList("0;1;999999"), ";")   == true);
        QVERIFY(matrix4.read(QStringList("1"),  ";")           == true);
        QVERIFY(matrix4.read(QStringList("1;"), ";", true)     == true);
        QVERIFY(matrix4.read(QStringList("-1"), ";")           == true);
        QCOMPARE(matrix4.get(0, 0),UINT_MAX);
        logLevel = AVLogger->minLevel();
        AVLogger->setMinLevel(AVLog::LOG__ERROR);
        QVERIFY(matrix4.read(QStringList("1.0"),  ";")         == false);
        QVERIFY(matrix4.read(QStringList("1;"),   ";")         == false);
        QVERIFY(matrix4.read(QStringList("x"),    ";")         == false);
        QVERIFY(matrix4.read(QStringList("1x"),   ";")         == false);
        QVERIFY(matrix4.read(QStringList("1-"),   ";")         == false);
        QVERIFY(matrix4.read(QStringList("TEST"), ";")         == false);
        AVLogger->setMinLevel(logLevel);

        AVMatrix<double> matrix5(0, 0,0);
        QVERIFY(matrix5.read(QStringList("1;-1;0.0"),  ";")    == true);
        QVERIFY(matrix5.read(QStringList("-1.0001"),   ";")    == true);
        QVERIFY(matrix5.read(QStringList("99.99;1.0"), ";")    == true);
        logLevel = AVLogger->minLevel();
        AVLogger->setMinLevel(AVLog::LOG__ERROR);
        QVERIFY(matrix5.read(QStringList("1, 0"),   ";")        == false);
        QVERIFY(matrix5.read(QStringList("-1, 0"),  ";")        == false);
        QVERIFY(matrix5.read(QStringList("z"),     ";")        == false);
        QVERIFY(matrix5.read(QStringList("1.0z"),  ";")        == false);
        QVERIFY(matrix5.read(QStringList("-1.0z"), ";")        == false);
        QVERIFY(matrix5.read(QStringList("TEST"),  ";")        == false);
        AVLogger->setMinLevel(logLevel);
    }

    void testMatrixFromAndToString()
    {
        // init matrix1
        AVMatrix<QString> matrix1(3, 3, "");
        QMap<QString, int> column_descriptions, row_descriptions;
        column_descriptions["-"] = 0;
        column_descriptions["abc"] = 1;
        column_descriptions["x\"y"] = 2;
        row_descriptions[" "] = 0;
        row_descriptions["\""] = 1;
        row_descriptions["\"asdf\""] = 2;
        QVERIFY(matrix1.setColumnDescriptions(column_descriptions));
        QVERIFY(matrix1.setRowDescriptions(row_descriptions));

        // copy to matrix2 using from/toString
        AVMatrix<QString> matrix2;
        QVERIFY(AVToString(matrix1) != AVToString(matrix2));
        QVERIFY(AVFromString(AVToString(matrix1), matrix2));
        QVERIFY(AVToString(matrix1) == AVToString(matrix2));

        // change values of matrix1 and copy to matrix2 using from/toString
        matrix1.set(0, 0, "1");
        matrix1.set(0, 1, "1.2");
        matrix1.set(0, 2, "abc");
        matrix1.set(1, 0, "a\"b");
        matrix1.set(1, 2, " ");
        matrix1.set(2, 0, " a");
        matrix1.set(2, 1, "a ");
        matrix1.set(2, 2, "\"asd\"");
        QVERIFY(AVToString(matrix1) != AVToString(matrix2));
        QVERIFY(AVFromString(AVToString(matrix1), matrix2));
        QVERIFY(AVToString(matrix1) == AVToString(matrix2));

        // change row descriptions matrix2 and copy to matrix1 using from/toString
        row_descriptions.clear();
        row_descriptions["0"] = 0;
        row_descriptions["1"] = 1;
        row_descriptions["2"] = 2;
        QVERIFY(matrix2.setRowDescriptions(row_descriptions));
        QVERIFY(AVToString(matrix1) != AVToString(matrix2));
        QVERIFY(AVFromString(AVToString(matrix2), matrix1));
        QVERIFY(AVToString(matrix1) == AVToString(matrix2));

        // change size of matrix1 and copy to matrix2 using from/toString
        matrix1.reset(10, 1, "x");
        matrix1.set(8, 0, "new");
        QVERIFY(AVToString(matrix1) != AVToString(matrix2));
        QVERIFY(AVFromString(AVToString(matrix2), matrix1));
        QVERIFY(AVToString(matrix1) == AVToString(matrix2));

        // init integer-based matrix3
        AVMatrix<int> matrix3(3, 3, 0);
        QVERIFY(matrix1.setColumnDescriptions(column_descriptions));
        QVERIFY(matrix1.setRowDescriptions(row_descriptions));
        matrix3.set(1, 1, 1337);

        // copy to matrix1 using from/toString
        QVERIFY(AVToString(matrix3) != AVToString(matrix1));
        QVERIFY(AVToString(matrix3) != AVToString(matrix2));
        QVERIFY(AVFromString(AVToString(matrix3), matrix1));
        QVERIFY(AVToString(matrix1) == AVToString(matrix3));

        // check empty matrix4
        AVMatrix<float> matrix4;
        QVERIFY(AVToString(matrix4) != AVToString(matrix1));
        QVERIFY(AVFromString(AVToString(matrix4), matrix1));
        QVERIFY(AVToString(matrix1) == AVToString(matrix4));

        // check single-column matrix6
        AVMatrix<float> matrix6(1, 5, 3.4f);
        QVERIFY(AVToString(matrix6) != AVToString(matrix1));
        QVERIFY(AVFromString(AVToString(matrix6), matrix1));
        QVERIFY(AVToString(matrix1) == AVToString(matrix6));

        // check single-row matrix7
        AVMatrix<double> matrix7(5, 1, 3.4);
        QVERIFY(AVToString(matrix7) != AVToString(matrix1));
        QVERIFY(AVFromString(AVToString(matrix7), matrix1));
        QVERIFY(AVToString(matrix1) == AVToString(matrix7));
    }

    void testMatrixWithDescriptions()
    {
        QStringList list1a;
        list1a.push_back("C0;C1;C2;C3");
        list1a.push_back("0;1;2;3");
        list1a.push_back("10;11;12;13");
        list1a.push_back("20;21;22;23");
        QStringList list1b;

        AVMatrix<int> matrix1(0, 0,0);
        QVERIFY(matrix1.read(list1a, ";", false, false, true));
        QCOMPARE(matrix1.getRows(), (int)3);
        QCOMPARE(matrix1.getColumns(), (int)4);
        QCOMPARE(matrix1.get(0, 0),0);
        QCOMPARE(matrix1.get(0, "C0"),0);
        QCOMPARE(matrix1.get(0, 1),1);
        QCOMPARE(matrix1.get(0, "C1"),1);
        QCOMPARE(matrix1.get(0, 2),2);
        QCOMPARE(matrix1.get(0, "C2"),2);
        QCOMPARE(matrix1.get(0, 3),3);
        QCOMPARE(matrix1.get(0, "C3"),3);
        QCOMPARE(matrix1.get(1, 0),10);
        QCOMPARE(matrix1.get(1, "C0"),10);
        QCOMPARE(matrix1.get(1, 1),11);
        QCOMPARE(matrix1.get(1, "C1"),11);
        QCOMPARE(matrix1.get(1, 2),12);
        QCOMPARE(matrix1.get(1, "C2"),12);
        QCOMPARE(matrix1.get(1, 3),13);
        QCOMPARE(matrix1.get(1, "C3"),13);
        QCOMPARE(matrix1.get(2, 0),20);
        QCOMPARE(matrix1.get(2, "C0"),20);
        QCOMPARE(matrix1.get(2, 1),21);
        QCOMPARE(matrix1.get(2, "C1"),21);
        QCOMPARE(matrix1.get(2, 2),22);
        QCOMPARE(matrix1.get(2, "C2"),22);
        QCOMPARE(matrix1.get(2, 3),23);
        QCOMPARE(matrix1.get(2, "C3"),23);
        // not supported yet
        // QVERIFY(matrix1.write(list1b, ";"));
        // QVERIFY(list1a == list1b);

        QStringList list2a;
        list2a.push_back("R0;0;1;2;3");
        list2a.push_back("R1;10;11;12;13");
        list2a.push_back("R2;20;21;22;23");
        QStringList list2b;

        AVMatrix<int> matrix2(0, 0,0);
        QVERIFY(matrix2.read(list2a, ";", false, true, false));
        QCOMPARE(matrix2.getRows(), (int)3);
        QCOMPARE(matrix2.getColumns(), (int)4);
        QCOMPARE(matrix2.get(0, 0),0);
        QCOMPARE(matrix2.get("R0", 0),0);
        QCOMPARE(matrix2.get(0, 1),1);
        QCOMPARE(matrix2.get("R0", 1),1);
        QCOMPARE(matrix2.get(0, 2),2);
        QCOMPARE(matrix2.get("R0", 2),2);
        QCOMPARE(matrix2.get(0, 3),3);
        QCOMPARE(matrix2.get("R0", 3),3);
        QCOMPARE(matrix2.get(1, 0),10);
        QCOMPARE(matrix2.get("R1", 0),10);
        QCOMPARE(matrix2.get(1, 1),11);
        QCOMPARE(matrix2.get("R1", 1),11);
        QCOMPARE(matrix2.get(1, 2),12);
        QCOMPARE(matrix2.get("R1", 2),12);
        QCOMPARE(matrix2.get(1, 3),13);
        QCOMPARE(matrix2.get("R1", 3),13);
        QCOMPARE(matrix2.get(2, 0),20);
        QCOMPARE(matrix2.get("R2", 0),20);
        QCOMPARE(matrix2.get(2, 1),21);
        QCOMPARE(matrix2.get("R2", 1),21);
        QCOMPARE(matrix2.get(2, 2),22);
        QCOMPARE(matrix2.get("R2", 2),22);
        QCOMPARE(matrix2.get(2, 3),23);
        QCOMPARE(matrix2.get("R2", 3),23);
        // not supported yet
        // QVERIFY(matrix2.write(list2b, ";"));
        // QVERIFY(list2a == list2b);

        QStringList list3a;
        list3a.push_back(";C0;C1;C2;C3");
        list3a.push_back("R0;0;1;2;3");
        list3a.push_back("R1;10;11;12;13");
        list3a.push_back("R2;20;21;22;23");
        QStringList list3b;

        AVMatrix<int> matrix3(0, 0,0);
        QVERIFY(matrix3.read(list3a, ";", false, true, true));
        QCOMPARE(matrix3.getRows(), (int)3);
        QCOMPARE(matrix3.getColumns(), (int)4);
        QCOMPARE(matrix3.get(0, 0),0);
        QCOMPARE(matrix3.get("R0", "C0"),0);
        QCOMPARE(matrix3.get(0, 1),1);
        QCOMPARE(matrix3.get("R0", "C1"),1);
        QCOMPARE(matrix3.get(0, 2),2);
        QCOMPARE(matrix3.get("R0", "C2"),2);
        QCOMPARE(matrix3.get(0, 3),3);
        QCOMPARE(matrix3.get("R0", "C3"),3);
        QCOMPARE(matrix3.get(1, 0),10);
        QCOMPARE(matrix3.get("R1", "C0"),10);
        QCOMPARE(matrix3.get(1, 1),11);
        QCOMPARE(matrix3.get("R1", "C1"),11);
        QCOMPARE(matrix3.get(1, 2),12);
        QCOMPARE(matrix3.get("R1", "C2"),12);
        QCOMPARE(matrix3.get(1, 3),13);
        QCOMPARE(matrix3.get("R1", "C3"),13);
        QCOMPARE(matrix3.get(2, 0),20);
        QCOMPARE(matrix3.get("R2", "C0"),20);
        QCOMPARE(matrix3.get(2, 1),21);
        QCOMPARE(matrix3.get("R2", "C1"),21);
        QCOMPARE(matrix3.get(2, 2),22);
        QCOMPARE(matrix3.get("R2", "C2"),22);
        QCOMPARE(matrix3.get(2, 3),23);
        QCOMPARE(matrix3.get("R2", "C3"),23);
        // not supported yet
        // QVERIFY(matrix3.write(list3b, ";"));
        // QVERIFY(list3a == list3b);
    }
};

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVMatrixUnitTest,"avlib/unittests/config")
#include "avmatrixtest.moc"


// End of file
