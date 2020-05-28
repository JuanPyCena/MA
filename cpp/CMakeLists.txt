cmake_minimum_required(VERSION 3.16)
project(cpp)

set(CMAKE_CXX_STANDARD 14)
set(EIGEN3_INCLUDE_DIR "C:\\ProgramData\\Eigen3")
set(BOOST "C:\\ProgramData\\boost")
set(LIBCONFIG "D:\\programming\\clion\\MasterarbeitCPP\\MA\\cpp\\libconfig")
set(PROJECT_SOURCE_DIR "D:\\programming\\clion\\MasterarbeitCPP\\MA\\cpp\\avbase")

set(CMAKE_CXX_FLAGS "-Wno-attributes")
include_directories(.)
include_directories(${EIGEN3_INCLUDE_DIR})
include_directories(${BOOST})
include_directories(${LIBCONFIG})
find_library(libconfig libconfig/lib)

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
        utils/typedefs.h)
