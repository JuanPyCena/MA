#! --------------------------------------------------------------------------
#!
#! Package:   AVCOMMON - Avibit Common Libraries
#! Copyright: AviBit data processing GmbH, 2001-2011
#!
#! Module:    AVLIBDEBRECATED - Avibit Library Deprecated classes
#!                              for compatibility with ported code from QT3
#!
#! --------------------------------------------------------------------------

set(module avlibdeprecated)

#-----------------------------------------------------------------------------

include_directories(.)
include_directories(${PROJECT_SOURCE_DIR}/avlib)

#-----------------------------------------------------------------------------

set(headers
    avlist.h
    avmiscdeprecated.h
    avpairlist.h
    avptrbuffer.h
    avptrhash.h
    avptrhashint.h
    avptrhashqstring.h
    avptrhashvoid.h
    avptrlist.h
    avvaluebuffer.h
    avsocketdevice.h
    avsocketdevicebase.h
    avobserver.h
    avobservableevent.h
    avrawsaver.h
    avmultimap.h
    avdatetimedeprecated.h
    avibitlogo.h
    avprintabledocument.h
    avprintabledocumentfooter.h
    avprintabledocumentheader.h
    avprintabletabledocument.h
    avprinter.h
    avprintertablecolumnstyle.h
    avqt3compat.h
    avtextwriter.h
    avarchiveplayer.h
    avset.h
)

#-----------------------------------------------------------------------------

set(sources
    avmiscdeprecated.cpp
    avsocketdevice.cpp
    avobserver.cpp
    avobservableevent.cpp
    avrawsaver.cpp
    avmultimap.cpp
    empty.cpp
    avprinter.cpp
    avprintabletabledocument.cpp
    avprintabledocumentheader.cpp
    avdatetimedeprecated.cpp
    avprintabledocument.cpp
    avprintertablecolumnstyle.cpp
    avprintabledocumentfooter.cpp
    avqt3compat.cpp
    avtextwriter.cpp
    avarchiveplayer.cpp
)

#-----------------------------------------------------------------------------

set(ui_files
)

#-----------------------------------------------------------------------------

add_executable(${module} ${headers} ${sources})

#! eof
