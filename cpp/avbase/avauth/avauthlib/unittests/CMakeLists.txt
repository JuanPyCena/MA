#-----------------------------------------------------------------------------

include(${AV_BUILD_SYSTEM_DIR}/cmake/CMakeLists_base.txt)

add_labels_to_tests_in_current_directory(MODULE_AVAUTH)


av_add_qtestlib_unittests(
    avauthenticator
    TEST_GROUP_NAME avauth
    DEPENDING_LIBRARIES avunittesthelperlib avauthlib
)
