cmake_minimum_required(VERSION 3.14)
project(procedurals)

## ----------- CMAKE -----------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(GoogleTest REQUIRED)
find_package(System REQUIRED)

## ----------- Executable -----------
set(SOURCE_FILES sources/no_source.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} PUBLIC
  system
)

## ----------- Include -----------
target_include_directories(${PROJECT_NAME} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/includes
)

## ----------- Compile Options -----------
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)

## ----------- test -----------
enable_testing()

file(GLOB TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)

add_executable(${PROJECT_NAME}_tests ${TEST_SOURCES})

target_include_directories(${PROJECT_NAME}_tests PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/includes
)

target_link_libraries(${PROJECT_NAME}_tests PRIVATE
  ${PROJECT_NAME}
  gtest_main
)

target_compile_options(${PROJECT_NAME}_tests PRIVATE ${COMMON_FLAGS})

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_tests WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
