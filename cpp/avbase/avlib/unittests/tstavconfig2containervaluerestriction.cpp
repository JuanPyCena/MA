///////////////////////////////////////////////////////////////////////////////
//
// Package:   ADMAX - Avibit AMAN/DMAN Libraries
// Copyright: AviBit data processing GmbH, 2001-2014
// QT-Version: QT5
//
// Module:    TESTS - AVCOMMON Tests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Matthias Fuchs, m.fuchs@avibit.com
    \author  QT4-PORT: Matthias Fuchs, m.fuchs@avibit.com
    \brief   Tests for AVConfig2ContainerValueRestriction
 */

// Qt5 includes
#include <QTest>
#include <avunittestmain.h>

// avlib includes
#include "avconfig2restrictions.h"

class TstAVConfig2ContainerValueRestriction : public QObject
{
    Q_OBJECT

private slots:
    void test_serialisation();
    void test_serialisation_of_wrong_value();

    void test_clone();

    void test_no_values_fullfil_restrictions();
    void test_valid_values_fulfill_restrictions();
    void test_a_single_invalid_value_leads_to_error();
};

///////////////////////////////////////////////////////////////////////////////

void TstAVConfig2ContainerValueRestriction::test_serialisation()
{
    const QString serialised1 = "Valid container values: 1, 7, 5";

    AVConfig2ContainerValueRestriction<QList<int> > restriction1(QList<int>() << 1 << 7 << 5);
    AVConfig2ContainerValueRestriction<QList<int> > restriction2(QList<int>() << 2 << 8 << 5);

    QVERIFY(!(restriction1 == restriction2));
    QVERIFY(restriction1.toString() != restriction2.toString());

    QVERIFY(restriction1.toString() == serialised1);
    QVERIFY(restriction2.fromString(serialised1));
    QVERIFY(restriction1 == restriction2);
}

///////////////////////////////////////////////////////////////////////////////

void TstAVConfig2ContainerValueRestriction::test_serialisation_of_wrong_value()
{
    AVConfig2ContainerValueRestriction<QList<int> > restriction(QList<int>() << 1);
    QVERIFY(!restriction.fromString("Valid container values: 1, A, 5"));
}

///////////////////////////////////////////////////////////////////////////////

void TstAVConfig2ContainerValueRestriction::test_clone()
{
    AVConfig2ContainerValueRestriction<QList<int> > restriction1(QList<int>() << 1 << 7 << 5);
    AVConfig2ContainerValueRestriction<QList<int> > *restriction2 = restriction1.clone();

    QVERIFY(restriction2 != 0);
    QVERIFY(restriction2 != &restriction1);
    QVERIFY(*restriction2 == restriction1);

    delete restriction2;
}

///////////////////////////////////////////////////////////////////////////////

void TstAVConfig2ContainerValueRestriction::test_no_values_fullfil_restrictions()
{
    const QString list = "[]";
    AVConfig2ContainerValueRestriction<QList<int> > restriction(QList<int>() << 1 << 7 << 5);

    const QString error_message = restriction.errorMessage("List", list);
    QVERIFY(error_message.isEmpty());
}


///////////////////////////////////////////////////////////////////////////////

void TstAVConfig2ContainerValueRestriction::test_valid_values_fulfill_restrictions()
{
    const QString list = "[7; 1]";
    AVConfig2ContainerValueRestriction<QList<int> > restriction(QList<int>() << 1 << 7 << 5);

    const QString error_message = restriction.errorMessage("List", list);
    QVERIFY(error_message.isEmpty());
}

///////////////////////////////////////////////////////////////////////////////

void TstAVConfig2ContainerValueRestriction::test_a_single_invalid_value_leads_to_error()
{
    const QString list = "[7; 1; 3]";
    AVConfig2ContainerValueRestriction<QList<int> > restriction(QList<int>() << 1 << 7 << 5);

    const QString error_message = restriction.errorMessage("List", list);
    QVERIFY(!error_message.isEmpty());
}

///////////////////////////////////////////////////////////////////////////////

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(TstAVConfig2ContainerValueRestriction,"avlib/unittests/config")
#include "tstavconfig2containervaluerestriction.moc"

// end of file
