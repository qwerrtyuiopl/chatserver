add_executable(json_test json_test.cpp)
add_executable(thread_test thread_test.cpp)
target_include_directories(json_test PRIVATE ${PROJECT_SOURCE_DIR}/include/json)
target_include_directories(thread_test PRIVATE ${PROJECT_SOURCE_DIR}/include/thread)