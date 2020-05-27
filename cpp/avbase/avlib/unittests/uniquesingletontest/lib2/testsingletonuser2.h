//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2015
//
// Module:    TESTS - AVCOMMON Unit Tests
//
//-----------------------------------------------------------------------------

/*! \file
    \brief   TestSingletonUser2 header
 */

#ifndef TESTSINGLETONUSER2_INCLUDED
#define TESTSINGLETONUSER2_INCLUDED

// local includes
#include "uniquesingletontest_lib2_export.h"

// Qt includes
#include <QtGlobal>

// avlib includes

// forward declarations

//-----------------------------------------------------------------------------
//! Test class using the TestSingleton
/*!
*/
class UNIQUESINGLETONTEST_LIB2_EXPORT TestSingletonUser2
{


public:
    TestSingletonUser2();
    virtual ~TestSingletonUser2();

private:
    Q_DISABLE_COPY(TestSingletonUser2)

};

#endif // TESTSINGLETONUSER2_INCLUDED

// End of file
