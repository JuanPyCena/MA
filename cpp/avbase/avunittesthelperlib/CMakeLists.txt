#! --------------------------------------------------------------------------
#!
#! Package:   AVCOMMON - Avibit Common Libraries
#! Copyright: AviBit data processing GmbH, 2001-2010
#! Author:    Gerhard Scheikl, g.scheikl@avibit.com
#!
#! Module:    AVUNITTESTHELPERLIB - AVCOMMON Unit Test helper library
#!
#! --------------------------------------------------------------------------

set(module avunittesthelperlib)

#-----------------------------------------------------------------------------

set(headers
    avunittesthelpers.h
    avunittestmain.h
    mockqnetworkaccessmanager.h
)

set(sources
    avunittesthelpers.cpp
    avunittestmain.cpp
    mockqnetworkaccessmanager.cpp
)

#-----------------------------------------------------------------------------
# Library

add_executable(${module} ${headers} ${sources})

#! eof
