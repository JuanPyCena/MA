///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    TESTS - AVCOMMON Unit Tests
//
/////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Andreas Schuller, a.schuller@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Test classes for testing template instantiations (cxx compiler)
*/


//Avlib

#include "avsingleton.h"

//local includes
#include "avsingletontestclasses.h"



TestClassUsedAsSingleton* TestClassSingletonInfo::getAddrOfSingleton()
{
    return TestClassUsedAsSingleton::getInstance();
}

int TestClassUsedAsSingleton::counter = 0;


//End of File
