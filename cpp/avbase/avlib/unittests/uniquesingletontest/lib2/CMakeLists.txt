

include(${AV_BUILD_SYSTEM_DIR}/cmake/CMakeLists_base.txt)

set(module uniquesingletontest_lib2)

#-----------------------------------------------------------------------------

include_directories(../lib1)

#-----------------------------------------------------------------------------

set(headers
    testsingletonuser2.h
)

#-----------------------------------------------------------------------------

set(sources
    testsingletonuser2.cpp
)

#-----------------------------------------------------------------------------

add_avlibrary(${module} ${headers} ${sources})
target_link_libraries(${module} ${QT5_LIBRARIES} avlib uniquesingletontest_lib1)

#! eof
