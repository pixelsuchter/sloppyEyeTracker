cmake_minimum_required(VERSION 3.0)

project(EyeTrackingCpp)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(EyeTrackingCpp main.cpp)

target_link_libraries(EyeTrackingCpp ${OpenCV_LIBS})
