# this is mostly for development to help your IDE understand this project's C++ code
# the actual build is done directly through emcc in build.sh

cmake_minimum_required(VERSION 3.8)
project(PCTreeJS CXX)
set(CMAKE_CXX_STANDARD 20)

find_package(PCTree REQUIRED)

set(SOURCE_FILES wasm/layout.cpp wasm/drawing.cpp)

add_library(PCTreeJS ${SOURCE_FILES} wasm/glue.cpp)
target_include_directories(PCTreeJS PUBLIC ${PCTree_INCLUDE_DIRS})
target_link_libraries(PCTreeJS PCTree)

add_executable(test_drawing ${SOURCE_FILES} wasm/test_drawing.cpp)
target_include_directories(test_drawing PUBLIC ${PCTree_INCLUDE_DIRS})
target_link_libraries(test_drawing PCTree)

add_executable(test_serialize ${SOURCE_FILES} wasm/test_serialize.cpp)
target_include_directories(test_serialize PUBLIC ${PCTree_INCLUDE_DIRS})
target_link_libraries(test_serialize PCTree)
