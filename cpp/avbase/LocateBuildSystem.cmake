
# This template file is copied into every module und included from the top-level CMakeLists.txt
# It contains the boilerplate code to initially locate the correct build system.

# *ATTENTION* If you need to change this file, ensure that the template in the build-system is updated.

file(READ "${CMAKE_CURRENT_LIST_DIR}/required_packages.cmake" _required_packages)
string(STRIP ${_required_packages} _required_packages)
string(REGEX MATCH "av_find_avibit_package\\(build-system[ ]+([^ ]+)\\)" _match ${_required_packages})
set(_build_system_version ${CMAKE_MATCH_1})
if("${AV_OVERRIDE_PACKAGES_VERSIONS}" MATCHES "build-system:([^;]+)")
    set(_build_system_version ${CMAKE_MATCH_1})
endif()

# use own variable instead of PACKAGE_FIND_VERSION to allow alphanumeric versions
# and call find_package() with version wildcard 0
set(AV_PACKAGE_FIND_VERSION ${_build_system_version})
set(_build_system_version 0)
message("** INFO: Searching for build-system in version \"${AV_PACKAGE_FIND_VERSION}\"")

if (UNIX)
    find_package(build-system ${_build_system_version} REQUIRED PATHS ${CMAKE_BINARY_DIR}/../installation /opt/avibit64/packages /opt/avibit/packages)
else (UNIX)
    find_package(build-system ${_build_system_version} REQUIRED PATHS ${CMAKE_BINARY_DIR}/../installation c:/avibit64/packages   c:/avibit/packages)
endif (UNIX)
