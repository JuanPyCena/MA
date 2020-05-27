#!
#! Package:   AVAUTH - AviBit Authentication Libraries
#! Copyright: AviBit data processing GmbH, 2001-2010
#! Author:    C. Skluzacek, christian.skluzacek@adbsafegate.com
#!
#! Module:    AVAUTH - AviBit Authentication Library
#!
#! --------------------------------------------------------------------------

set(module avauthlib)

#-----------------------------------------------------------------------------

include_directories(${PROJECT_SOURCE_DIR}/avauth)

#-----------------------------------------------------------------------------

set(headers
    avauthenticator.h
    avauthenticatorconfig.h
    avconvert.h
    avidtoken.h
    avjsonutils.h
    avopeniddata.h
    avservicesmetadata.h
    avtokendata.h
)

#-----------------------------------------------------------------------------

set(sources
    avauthenticator.cpp
    avauthenticatorconfig.cpp
    avjsonutils.cpp
    avopeniddata.cpp
    avservicesmetadata.cpp
    avtokendata.cpp
)

#-----------------------------------------------------------------------------
# add module-wide QT_USE_QSTRINGBUILDER definition (reference: http://doc.qt.io/qt-5/qstring.html)
# so that '+' will automatically be performed as the QStringBuilder '%' everywhere.

add_definitions(-DQT_USE_QSTRINGBUILDER)

# RCM
# for token verification step
# find_package(OpenSSL REQUIRED)
#-----------------------------------------------------------------------------

add_executable(${module} ${headers} ${sources} ${ui_sources})

#! eof
