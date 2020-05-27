#-----------------------------------------------------------------------------

include(${AV_BUILD_SYSTEM_DIR}/cmake/CMakeLists_base.txt)

add_labels_to_tests_in_current_directory(MODULE_AVLIB BENCHMARK)

#-----------------------------------------------------------------------------
include_directories(.)

#-----------------------------------------------------------------------------

av_add_qtestlib_unittests(
    avconfig2benchmarktest

    DEPENDING_LIBRARIES  Qt5::Script avlib avunittesthelperlib
)

# TODO create macro in build-system for this (see BS-229)
file(RELATIVE_PATH _relative_directory ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_LIST_DIR})
install(DIRECTORY config DESTINATION ${_relative_directory} COMPONENT tests)

#-----------------------------------------------------------------------------
#! eof
