//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2015
//
// Module:    TESTS - AVCOMMON Unit Tests
//
//-----------------------------------------------------------------------------

/*! \file
    \brief   TestSingletonUser1 header
 */

#ifndef TESTSINGLETONUSER1_INCLUDED
#define TESTSINGLETONUSER1_INCLUDED

// local includes
#include "uniquesingletontest_lib1_export.h"

// Qt includes
#include <QtGlobal>

// avlib includes

// forward declarations

//-----------------------------------------------------------------------------
//! Test class using the TestSingleton
/*!
*/
class UNIQUESINGLETONTEST_LIB1_EXPORT TestSingletonUser1
{


public:
    TestSingletonUser1();
    virtual ~TestSingletonUser1();

private:
    Q_DISABLE_COPY(TestSingletonUser1)

};

#endif // TESTSINGLETONUSER1_INCLUDED

// End of file
