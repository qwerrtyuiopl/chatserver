cmake_minimum_required(VERSION 3.5)
project(ChatSystem)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin")


include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/include/server")
include_directories("${PROJECT_SOURCE_DIR}/include/client")
include_directories("${PROJECT_SOURCE_DIR}/include/json")
include_directories("${PROJECT_SOURCE_DIR}/include/public")
include_directories("${PROJECT_SOURCE_DIR}/include/thread")
add_subdirectory(src)
add_subdirectory(test)