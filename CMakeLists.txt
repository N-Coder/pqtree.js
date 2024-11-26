# this is mostly for development to help your IDE understand this project's C++ code
# the actual build is done directly through emcc in build.sh

cmake_minimum_required(VERSION 3.8)
project(PCTreeJS CXX)
set(CMAKE_CXX_STANDARD 20)

find_package(PCTree REQUIRED)

add_library(PCTreeJS wasm/glue.cpp wasm/layout.cpp)
target_include_directories(PCTreeJS PRIVATE ${PCTree_INCLUDE_DIRS})
target_link_libraries(PCTreeJS PCTree)
