

include(${AV_BUILD_SYSTEM_DIR}/cmake/CMakeLists_base.txt)

#------------------------------------------------------------------------------

av_add_qtestlib_unittests(
    avconsolecommandtest
    avconsoletest
    TEST_GROUP_NAME avlib
    DEPENDING_LIBRARIES avlib avunittesthelperlib avlibunittesthelperlib
)

#! eof
