cmake_minimum_required(VERSION 3.16)
project(cpp)

set(CMAKE_CXX_STANDARD 14)
set(EIGEN3_INCLUDE_DIR "external/Eigen3")
set(CONFIG_READ_LIB "external/config_read_lib/lightconf")

include_directories(.)
include_directories(${EIGEN3_INCLUDE_DIR})
include_directories(${LIBCONFIG})
include_directories(${CONFIG_READ_LIB})

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