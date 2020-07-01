//
// Created by felix on 6/2/20.
//

///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON
// QT-Version: QT5
// Copyright:  AviBit data processing GmbH, 2001-2018
//
// Module:     UnitTests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \brief   Pseudo Main for manual testing
 */

#include <QObject>
#include <QTest>
#include <avunittest.h>
#include <QApplication>

#include "testhelper/avimmtester.h"

class TstAVIMMMVN : public QObject
{
Q_OBJECT

public:
    TstAVIMMMVN() {}

public slots:
    void initTestCase() {};
    void cleanupTestCase() {};
    void init() {};
    void cleanup() {};

private slots:
    void test_MVN_pdf();
    void test_MVN_logpdf();
};

///////////////////////////////////////////////////////////////////////////////

void TstAVIMMMVN::test_MVN_pdf()
{
    Vector m_zero(6,1);
    m_zero << 0,0,0,0,0,0;
    
    Vector m_non_zero(6,1);
    m_non_zero << 0,0,1,0,0,1;
    
    double ref_zero = 4.017963066353*exp(-14);
    double ref_non_zero = 4.81299318*exp(-12);
    
    Vector error(6,1);
    error << 1,2,3,4,5,6;
    
    Matrix S(6,6);
    S <<    1,0,0,0,0,0,
            0,3,0,0,0,0,
            0,0,1,0,9,0,
            0,0,0,1,0,0,
            0,0,0,0,7,0,
            0,0,0,0,0,1;
    
    const Vector mean_zero      = m_zero;
    const Vector mean_non_zero  = m_non_zero;
    const Matrix sigma          = S;
    const Vector x              = error;
    
    Mvn mvn_zero(mean_zero, sigma);
    Mvn mvn_non_zero(mean_non_zero, sigma);
    double pdf_zero = mvn_zero.pdf(x);
    double pdf_non_zero = mvn_non_zero.pdf(x);
    
    std::cout.precision(50);
    
    QVERIFY((pdf_zero - ref_zero) < 1*exp(-5));
    QVERIFY((pdf_non_zero - ref_non_zero) < 1*exp(-5));
}

///////////////////////////////////////////////////////////////////////////////

void TstAVIMMMVN::test_MVN_logpdf()
{
    Vector m_zero(6,1);
    m_zero << 0,0,0,0,0,0;
    
    Vector m_non_zero(6,1);
    m_non_zero << 0,0,1,0,0,1;
    
    double ref_zero = -30.84541622761355839088537;
    double ref_non_zero = -26.05970194189927013894703;
    
    Vector error(6,1);
    error << 1,2,3,4,5,6;
    
    Matrix S(6,6);
    S <<    1,0,0,0,0,0,
            0,3,0,0,0,0,
            0,0,1,0,9,0,
            0,0,0,1,0,0,
            0,0,0,0,7,0,
            0,0,0,0,0,1;
    
    const Vector mean_zero      = m_zero;
    const Vector mean_non_zero  = m_non_zero;
    const Matrix sigma          = S;
    const Vector x              = error;
    
    Mvn mvn_zero(mean_zero, sigma);
    Mvn mvn_non_zero(mean_non_zero, sigma);
    double pdf_zero = mvn_zero.logpdf(x);
    double pdf_non_zero = mvn_non_zero.logpdf(x);
    
    QVERIFY(pdf_zero == ref_zero);
    QVERIFY(pdf_non_zero == ref_non_zero);
}

AV_QTEST_MAIN(TstAVIMMMVN)
#include "tstavimmmvn.moc"
