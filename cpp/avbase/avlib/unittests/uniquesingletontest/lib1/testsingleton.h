//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH,2001-2010
//
// Module:    TESTS - AVCOMMON Unit Tests
//
//-----------------------------------------------------------------------------

/*! \file
    \brief   TestSingleton header
 */

#ifndef TESTSINGLETON_INCLUDED
#define TESTSINGLETON_INCLUDED

// local includes
#include "uniquesingletontest_lib1_export.h"

// Qt includes

// avlib includes
#include "avsingleton.h"

// forward declarations

//-----------------------------------------------------------------------------
//! Test singleton
/*!
*/
class UNIQUESINGLETONTEST_LIB1_EXPORT TestSingleton : public AVSingleton<TestSingleton>
{
    //! friend declaration for function level test case
    //friend class TstTestSingleton;

public:
    TestSingleton();
    ~TestSingleton() override;

    void print(const QString& msg);


};

#endif // TESTSINGLETON_INCLUDED

// End of file
