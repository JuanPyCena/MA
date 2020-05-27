#-----------------------------------------------------------------------------


#-----------------------------------------------------------------------------
include_directories(.)
include_directories(${PROJECT_SOURCE_DIR}/avlib)
include_directories(${PROJECT_SOURCE_DIR}/avcalculationlib)
include_directories(${PROJECT_SOURCE_DIR}/avunittesthelperlib)


# TODO create macro in build-system for this (see BS-229)
file(RELATIVE_PATH _relative_directory ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_LIST_DIR})
install(DIRECTORY config DESTINATION ${_relative_directory} COMPONENT tests)

#-----------------------------------------------------------------------------
#! eof
