cmake_minimum_required (VERSION 2.6)
project (camera_fake)

find_package( OpenCV REQUIRED )

set(CMAKE_CXX_FLAGS "-std=c++11 -O2")


add_executable (filter_image src/main.cpp)
target_link_libraries (filter_image tiobj tisys ${OpenCV_LIBS})

install(TARGETS filter_image DESTINATION bin)
