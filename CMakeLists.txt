project(naval-warfare)
cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}")

# uses the FindGear2D.cmake to search for Gear2D
find_package(Gear2D REQUIRED)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/../components)
message(STATUS ${CMAKE_INSTALL_PREFIX})

# add the Gear2D include directories and link directories.
# they are calculated by FindGear2D.cmake
include_directories(${Gear2D_INCLUDE_DIR} ${SDL_INCLUDE_DIR})
link_directories(${Gear2D_LINK_DIR})

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  add_definitions(-DLOGTRACE)
endif(${CMAKE_BUILD_TYPE} STREQUAL "Debug")

add_subdirectory(src)
