cmake_minimum_required(VERSION 3.10)
project(OpenCVExample)

set(CMAKE_CXX_STANDARD 23)
find_package(PkgConfig REQUIRED)

pkg_check_modules(OpenCV REQUIRED opencv4)

add_executable(MyApp main.cpp)

target_include_directories(MyApp PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(MyApp PRIVATE ${OpenCV_LIBRARIES})
