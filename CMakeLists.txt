cmake_minimum_required(VERSION 3.20)
project(MyECSv)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-pthread -fconcepts")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

add_subdirectory(source)
add_subdirectory(GoogleTests/googletest)

include_directories(GoogleTests/googletest/googlemock/include/)
include_directories(GoogleTests/googletest/googletest/include/)

target_link_libraries(${PROJECT_NAME} gtest gtest_main)
target_link_libraries(${PROJECT_NAME} gmock gmock_main)

find_package(fmt)
target_link_libraries(${PROJECT_NAME} fmt)
