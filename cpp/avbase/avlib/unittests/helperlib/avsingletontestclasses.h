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

#if !defined(AVSINGLETONTESTCLASSES_H)
#define AVSINGLETONTESTCLASSES_H

#include "avlog.h"
#include "avsingleton.h"
#include "avlibunittesthelperlib_export.h"

class AVLIBUNITTESTHELPERLIB_EXPORT TestClassUsedAsSingleton : public AVSingleton<TestClassUsedAsSingleton>
{
public:

    TestClassUsedAsSingleton()
    {
        ++counter;
        qDebug("Ctor of TestClassUsedAsSingleton called: instance counter (%d)", counter);
    }

    ~TestClassUsedAsSingleton() override {}

private:

    static int counter;

};

class AVLIBUNITTESTHELPERLIB_EXPORT TestClassSingletonInfo
{
public:
   static TestClassUsedAsSingleton* getAddrOfSingleton();

};

#endif

// End of file
