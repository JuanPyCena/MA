//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2015
//
// Module:    TESTS - AVCOMMON Unit Tests
//
//-----------------------------------------------------------------------------

/*! \file
    \brief   TestSingletonUser2 implementation
 */

// local includes
#include "testsingletonuser2.h"
#include "testsingleton.h"

// Qt includes

// avlib includes



//-----------------------------------------------------------------------------

TestSingletonUser2::TestSingletonUser2()
{
    TestSingleton::getInstance()->print("TestSingeltonUser2");
}

//-----------------------------------------------------------------------------

TestSingletonUser2::~TestSingletonUser2()
{
}

// End of file
