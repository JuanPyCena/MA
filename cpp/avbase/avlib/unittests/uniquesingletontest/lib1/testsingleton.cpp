//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH,2001-2010
//
// Module:    TESTS - AVCOMMON Unit Tests
//
//-----------------------------------------------------------------------------

/*! \file
    \brief   TestSingleton implementation
 */

// local includes
#include "testsingleton.h"

// Qt includes

// avlib includes

//-----------------------------------------------------------------------------

TestSingleton::TestSingleton()
{
    AVLogInfo << "TestSingleton::TestSingleton: ";
}

//-----------------------------------------------------------------------------

TestSingleton::~TestSingleton()
{
}

//-----------------------------------------------------------------------------

void TestSingleton::print(const QString& msg)
{
    AVLogInfo << "TestSingleton::print: ("<<this<<") "<<msg;
}

//-----------------------------------------------------------------------------

// End of file
