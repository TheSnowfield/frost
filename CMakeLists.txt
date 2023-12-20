cmake_minimum_required(VERSION 3.10)

project(frost)
set(CMAKE_C_STANDARD 11)

# all useful directories
set(FROST_ADDITIONAL_INC_DIR "")
set(FROST_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)
set(FROST_LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# search source files
file(GLOB_RECURSE FROST_SRC src/*.c)
  list(APPEND FROST_SRC ${FROST_LIB_SRC})

# add include directories
include_directories(
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/src
  ${FROST_ADDITIONAL_INC_DIR}
)

# ci build
if(NOT DEFINED CI_BUILD)
  execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  add_definitions(-DFROST_VERSION_MAJOR=dirty)
  add_definitions(-DFROST_VERSION_MINOR=${GIT_BRANCH})
  add_definitions(-DFROST_VERSION_PATCH=${GIT_HASH})

elseif(DEFINED CI_BUILD)
  add_definitions(-DFROST_VERSION_MAJOR=${CI_VERSION_MAJOR})
  add_definitions(-DFROST_VERSION_MINOR=${CI_VERSION_MINOR})
  add_definitions(-DFROST_VERSION_PATCH=${CI_VERSION_PATCH})
endif()

# build config
if(NOT DEFINED CONFIG OR CONFIG STREQUAL "debug")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
elseif()
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
endif()

if(BUILD STREQUAL "test")
  include(${FROST_TEST_DIR}/CMakeLists.txt)
elseif(BUILD STREQUAL "lib")
  include(${FROST_LIBRARY_DIR}/CMakeLists.txt)
else()
  message(FATAL_ERROR "Unknown build type, please specify `-DBUILD=test|lib`")
endif()
