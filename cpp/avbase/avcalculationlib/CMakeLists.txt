#! --------------------------------------------------------------------------
#!
#! Package:   AVCOMMON - AviBit Common Libraries
#! Copyright: AviBit data processing GmbH, 2001-2010
#! Author:    Dr. Thomas Leitner, t.leitner@avibit.com
#!
#! Module:    AVCALCULATIONLIB - AviBit Calculation Library
#!
#! --------------------------------------------------------------------------

set(module avcalculationlib)

#-----------------------------------------------------------------------------

include_directories(.)
include_directories(${PROJECT_SOURCE_DIR}/avlibdeprecated)
include_directories(${PROJECT_SOURCE_DIR}/avlib)

#-----------------------------------------------------------------------------

add_subdirectory(unittests)

#-----------------------------------------------------------------------------

set(headers
    avtransitionlevel.h
    avrandom.h
    avaviationcalculator.h
    avdgstosystemtrafo.h
    avcoordellipsoid.h
    avcoordinatetransform3dbase.h
    avcoordinatetransformbase.h
    avearthdatum.h
    avgausskruegertrafo.h
    avgausskruegersystemtrafo.h
    avgeoref.h
    avgeoreftrafo.h
    avmeteosattrafo.h
    avplanarcalculator.h
    avsciconst.h
    avsystemtrafo.h
    avsystemtrafocfg.h
    avvectorcoords.h
    avssrtrafo.h
    avstereotrafo.h
    avtrafofactory.h
    avtransformationcalculator.h
    avutmsystemtrafo.h
    avutmtrafo.h
    avweatherradartrafo.h
    averrorellipse.h
    avconvexhull.h
    avrunningmean.h
    avrunningmeantime.h
    avsmrtrafo.h
    avimagetransformation.h
)

#-----------------------------------------------------------------------------

set(sources
    avtransitionlevel.cpp
    avrandom.cpp
    avaviationcalculator.cpp
    avdgstosystemtrafo.cpp
    avcoordinatetransform3dbase.cpp
    avcoordinatetransformbase.cpp
    avearthdatum.cpp
    avgausskruegertrafo.cpp
    avgausskruegersystemtrafo.cpp
    avgeoref.cpp
    avgeoreftrafo.cpp
    avmeteosattrafo.cpp
    avplanarcalculator.cpp
    avsciconst.cpp
    avsystemtrafo.cpp
    avsystemtrafocfg.cpp
    avssrtrafo.cpp
    avstereotrafo.cpp
    avtrafofactory.cpp
    avtransformationcalculator.cpp
    avutmsystemtrafo.cpp
    avutmtrafo.cpp
    avweatherradartrafo.cpp
    averrorellipse.cpp
    avvectorcoords.cpp
    avsmrtrafo.cpp
    avimagetransformation.cpp
)

add_executable(${module} ${headers} ${sources} ${ui_sources})

#-----------------------------------------------------------------------------

#! eof
