#!
#! Package:   AVAUTHTESTCLIENT - AviBit Authentication Test Client Application
#! Copyright: AviBit data processing GmbH, 2001-2010
#! Author:    C. Skluzacek, christian.skluzacek@adbsafegate.com
#!
#! Module:    AVAUTHTESTCLIENT - AviBit Authentication Test Client Application
#!
#! --------------------------------------------------------------------------

set(module avauthtestgui)

#-----------------------------------------------------------------------------

include_directories(${PROJECT_SOURCE_DIR}/avauth)

#-----------------------------------------------------------------------------

set(headers
    application.h
    avlogmodel.h
    mainwindow.h
)

#-----------------------------------------------------------------------------

set(sources
    application.cpp
    avauthtestclient.cpp
    avlogmodel.cpp
    mainwindow.cpp
    mainwindow.ui
)

#-----------------------------------------------------------------------------
# add module-wide QT_USE_QSTRINGBUILDER definition (reference: http://doc.qt.io/qt-5/qstring.html)
# so that '+' will automatically be performed as the QStringBuilder '%' everywhere.

add_definitions(-DQT_USE_QSTRINGBUILDER)

#-----------------------------------------------------------------------------

add_executable(${module} ${headers} ${sources} ${ui_sources})

#! eof
