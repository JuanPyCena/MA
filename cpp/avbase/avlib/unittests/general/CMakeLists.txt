#-----------------------------------------------------------------------------

include(${AV_BUILD_SYSTEM_DIR}/cmake/CMakeLists_base.txt)

#-----------------------------------------------------------------------------
include_directories(.)
include_directories(${PROJECT_SOURCE_DIR}/avunittesthelperlib)

#-----------------------------------------------------------------------------

SET(DEPENDING_LIBRARIES avlib avunittesthelperlib)



av_add_qtestlib_unittests(
    avalloctest
    TEST_GROUP_NAME avlib
    DEPENDING_LIBRARIES  avlib avunittesthelperlib)
#-----------------------------------------------------------------------------
