cmake_minimum_required(VERSION 3.22)
project(daBERs)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(fmt-8.1.1)

add_library(daBERs-obj OBJECT
        src/tag.cpp
        src/buffer_check.cpp
        src/exception.cpp
        src/length.cpp)
target_include_directories(daBERs-obj PUBLIC include)
target_link_libraries(daBERs-obj PRIVATE fmt::fmt-header-only)
set_target_properties(daBERs-obj PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(daBERs STATIC $<TARGET_OBJECTS:daBERs-obj>)
target_link_libraries(daBERs PUBLIC daBERs-obj)

#The DocTest main doesn't actually use anything in our static library,
#  so auto registration doesn't work because the linker won't pull
#  anything in.  See https://github.com/doctest/doctest/issues/21.
#  So we work around it by having an object library we use to both
#  build the test exe and the static library.
add_executable(daBERs_tests test_main.cpp $<TARGET_OBJECTS:daBERs-obj>)
target_link_libraries(daBERs_tests PUBLIC daBERs-obj)

enable_testing()
add_test(NAME daBERs_tests COMMAND daBERs_tests)