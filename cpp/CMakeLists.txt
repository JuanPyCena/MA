cmake_minimum_required(VERSION 3.16)
project(cpp)

set(CMAKE_CXX_STANDARD 14)
set(EIGEN3_INCLUDE_DIR "C:\\ProgramData\\Eigen3")
include_directories(.)
include_directories(${EIGEN3_INCLUDE_DIR})

add_executable(cpp
        main.cpp filterlib/imm_kalman_filter.cpp filterlib/imm_kalman_filter.h filterlib/imm_estimator.cpp filterlib/imm_estimator.h filterlib/imm_filter_base.h utils/makros.h filterlib/imm_extended_kalman_filter.cpp filterlib/imm_extended_kalman_filter.h)
