#! --------------------------------------------------------------------------
#!
#! Package:   AVCOMMON - AviBit Common Libraries
#! Copyright: AviBit data processing GmbH, 2001-2010
#! Author:    Dr. Thomas Leitner, t.leitner@avibit.com
#!
#! Module:    AVMSGLIB - AviBit Messaging Library
#!
#! --------------------------------------------------------------------------

include(${AV_BUILD_SYSTEM_DIR}/cmake/CMakeLists_base.txt)

set(module avimmlib)

#-----------------------------------------------------------------------------

add_subdirectory(unittests)

#-----------------------------------------------------------------------------

set(headers
        filterlib/avimmestimator.h
        filterlib/avimmextendedkalmanfilter.h
        filterlib/avimmfilterbase.h
        filterlib/avimmkalmanfilter.h
        utils/avimmconfig.h
        utils/avimmmakros.h
        utils/avimmtypedefs.h
        utils/avimmconfigparser.h
)

#-----------------------------------------------------------------------------

set(sources
        filterlib/avimmestimator.cpp
        filterlib/avimmextendedkalmanfilter.cpp
        filterlib/avimmkalmanfilter.cpp
        utils/avimmconfig.cpp
        )


add_avlibrary(${module} ${headers} ${sources})
target_link_libraries(${module} ${QT5_LIBRARIES})
target_include_directories(${module} SYSTEM PUBLIC
        $<BUILD_INTERFACE:${AVCOMMON_SOURCE_DIR}/3rdparty/eigen3>
        $<INSTALL_INTERFACE:include/avcommon/src5/3rdparty/eigen3>
        )
target_link_libraries(${module} ${QT_LIBRARIES} avlib)
#! eof
