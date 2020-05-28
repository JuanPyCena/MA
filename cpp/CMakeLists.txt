cmake_minimum_required(VERSION 3.16)
project(cpp)

set(CMAKE_CXX_STANDARD 14)
set(EIGEN3_INCLUDE_DIR "C:\\ProgramData\\Eigen3")
set(BOOST "C:\\ProgramData\\boost")
set(LIBCONFIG "D:\\programming\\clion\\MasterarbeitCPP\\vcpkg\\packages\\libconfig_x86-windows\\include")
set(CMAKE_TOOLCHAIN_FILE "D:\\programming\\clion\\MasterarbeitCPP\\vcpkg\\packages\\libconfig_x86-windows")

include_directories(.)
include_directories(${EIGEN3_INCLUDE_DIR})
include_directories(${BOOST})
include_directories(${LIBCONFIG})


add_executable(cpp
        main.cpp
        filterlib/imm_kalman_filter.cpp
        filterlib/imm_kalman_filter.h
        filterlib/imm_estimator.cpp
        filterlib/imm_estimator.h
        filterlib/imm_filter_base.h
        filterlib/imm_extended_kalman_filter.cpp
        filterlib/imm_extended_kalman_filter.h
        utils/imm_config.cpp
        utils/imm_config.h
        utils/makros.h
        utils/typedefs.h
        )
target_link_libraries(cpp "D:/programming/clion/MasterarbeitCPP/vcpkg/packages/libconfig_x86-windows/lib/libconfig++.lib")