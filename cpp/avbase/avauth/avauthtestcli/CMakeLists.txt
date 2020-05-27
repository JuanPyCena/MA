#! --------------------------------------------------------------------------
#!
#! Package:   AVCOMMON - AviBit Common Libraries
#! Copyright: AviBit data processing GmbH, 2001-2010
#! Author:    Christian Muschick, c.muschick@avibit.com
#!
#! Module:    AVAUTHTESTCLI - command line avauth test utility
#!
#! --------------------------------------------------------------------------

# RCM doesn't match our usual pattern
include_directories(${PROJECT_SOURCE_DIR}/avauth)

#-----------------------------------------------------------------------------

set(headers
)

set(sources
    avauthtestcli.cpp
)

add_executable(avauthtestcli avauthtestcli.cpp)

#! eof
