#-----------------------------------------------------------------------------

include(${AV_BUILD_SYSTEM_DIR}/cmake/CMakeLists_base.txt)

# disable c style warnings
if(CMAKE_COMPILER_IS_GNUCXX)
    string(REPLACE "-Wold-style-cast" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
endif()

add_labels_to_tests_in_current_directory(MODULE_AVLIB)

#-----------------------------------------------------------------------------
include_directories(.)

#-----------------------------------------------------------------------------

add_subdirectory(general)
add_subdirectory(avtimereference)
add_subdirectory(avconsole)
add_subdirectory(uniquesingletontest)
add_subdirectory(benchmarks)

av_add_qtestlib_unittests(
    avconfig2exportercstyle
    avtwowayhash
    avvehicle
    avconfig2test
    avconfigtest
    avcstringtest
    avdatastreamtest
    avdataframinglengthtest
    avdatetimetest
    avdomvariablestest
    avenvstringtest
    avexpressionstest
    avfactorytest
    avfromtostringtest
    avlogtest
    avmacros
    avmatrixtest
    avmisctest
    avresolvehosttest
    avipaddresstest
    avprofiling
    avpathtest
    avserverlisttest
    avsetsunittest
    avsharetest
    avsharedmemory
    avstringpairlistunittest
    avthreadsingletontest
    avthreadtest
    avtimer
    avtimereference
# SWE-4588
#    avtimingtests
    avuniquenumbergeneratortest
    qdatastreamtest
    qscriptenginetest
    avcrctest
    avexecstandbychecker
    avlogwriter
    avprocessstatedesc
    avconfig2containervaluerestriction
    avconfig2sharedconfigholder
    avsingletontest
    avenvironment
    avthreadobjectmanager
    avtypesafesignalspy
    # avstdinreaderthread  #enable after cmake >3.13 https://gitlab.kitware.com/cmake/cmake/issues/18591
    avdataframingdelimiters
    avversionreader
    avpropertycomputablegadgettest
    avpropertycomputableqobjecttest
    avhashstringperformance
    avprocesswatchdog
    TEST_GROUP_NAME avlib
    HELPER_LIBRARY_NAME avlibunittesthelperlib
    HELPER_CODE_FILES helperlib/avsingletontestclasses.h
                      helperlib/avsingletontestclasses.cpp
                      helperlib/avconsoleunittestconnection.h
                      helperlib/avconsoleunittestconnection.cpp
    DEPENDING_LIBRARIES  Qt5::Script avlib avunittesthelperlib
)

#-----------------------------------------------------------------------------

av_install_subdirectory(config COMPONENT tests)

#! eof
