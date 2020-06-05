#-----------------------------------------------------------------------------

include(${AV_BUILD_SYSTEM_DIR}/cmake/CMakeLists_base.txt)

# disable c style warnings
if(CMAKE_COMPILER_IS_GNUCXX)
    string(REPLACE "-Wold-style-cast" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
endif()

#-----------------------------------------------------------------------------

add_labels_to_tests_in_current_directory(MODULE_AVIMMLIB)

#-----------------------------------------------------------------------------
include_directories(${PROJECT_SOURCE_DIR}/unittests)


#-----------------------------------------------------------------------------

av_add_qtestlib_unittests(
        tstimmtestmain
        tstavimmconfigreader
        HELPER_LIBRARY_NAME avimmlibunittesthelperlib
        TEST_GROUP_NAME avimmlib
        HELPER_CODE_FILES testhelper/avimmtester.h
        DEPENDING_LIBRARIES avlib avimmlib avunittesthelperlib
)