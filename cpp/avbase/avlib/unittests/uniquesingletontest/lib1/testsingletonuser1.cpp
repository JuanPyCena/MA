//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2015
//
// Module:    TESTS - AVCOMMON Unit Tests
//
//-----------------------------------------------------------------------------

/*! \file
    \brief   TestSingletonUser1 implementation
 */

// local includes
#include "testsingletonuser1.h"
#include "testsingleton.h"

// Qt includes

// avlib includes



//-----------------------------------------------------------------------------

TestSingletonUser1::TestSingletonUser1()
{
    TestSingleton::getInstance()->print("TestSingeltonUser1");
}

//-----------------------------------------------------------------------------

TestSingletonUser1::~TestSingletonUser1()
{
}

// End of file
