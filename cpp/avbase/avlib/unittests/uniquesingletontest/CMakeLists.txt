#-----------------------------------------------------------------------------

include(${AV_BUILD_SYSTEM_DIR}/cmake/CMakeLists_base.txt)


#-----------------------------------------------------------------------------

include_directories(lib1)
include_directories(lib2)
include_directories(${PROJECT_SOURCE_DIR}/avunittesthelperlib)

#-----------------------------------------------------------------------------

add_subdirectory(lib1)
add_subdirectory(lib2)

av_add_qtestlib_unittests(
    uniquesingletontest
    TEST_GROUP_NAME avlib
    DEPENDING_LIBRARIES avlib avunittesthelperlib uniquesingletontest_lib1 uniquesingletontest_lib2
)

#-----------------------------------------------------------------------------
#! eof
