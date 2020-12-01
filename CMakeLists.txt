cmake_minimum_required(VERSION 3.9)

# set the project name and version
project( stereo_estimation VERSION 1.0)

###############
# packages #
###############

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

find_package(OpenMP)

###############
# executables #
###############

add_executable( OpenCV_naive_stereo src/main.cpp  src/utils.cpp src/stereomatching.cpp src/filters.cpp)
target_link_libraries( OpenCV_naive_stereo ${OpenCV_LIBS} )

if(OpenMP_CXX_FOUND)
    target_link_libraries(OpenCV_naive_stereo OpenMP::OpenMP_CXX)
endif()
