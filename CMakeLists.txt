cmake_minimum_required (VERSION 3.11)
project(420-colours CXX)

set (COLOURS_VERSION_MAJOR 420)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pthread")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g3")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")


set (EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set (LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

find_package(fmt REQUIRED)

add_executable(420-colours
  main.cpp
)

target_link_libraries(420-colours
  SDL2
  GL
  GLEW
  fmt
)