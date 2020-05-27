

include(${AV_BUILD_SYSTEM_DIR}/cmake/CMakeLists_base.txt)

set(module uniquesingletontest_lib1)

#-----------------------------------------------------------------------------


#-----------------------------------------------------------------------------

set(headers
    testsingletonuser1.h
    testsingleton.h
)

#-----------------------------------------------------------------------------

set(sources
    testsingletonuser1.cpp
    testsingleton.cpp
)

#-----------------------------------------------------------------------------

add_avlibrary(${module} ${headers} ${sources})
target_link_libraries(${module} ${QT5_LIBRARIES} avlib)

#! eof
