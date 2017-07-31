#  Copyright (c) 2017 Alireza Kheirkhahang
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.7)
project(interposition_hpx)

SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
link_directories(${CMAKE_BINARY_DIR}/lib)
include_directories(${PROJECT_SOURCE_DIR})
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_POSITION_INDEPENDENT_CODE ON)

IF(UNIX)
    IF(CMAKE_COMPILER_IS_GNUCC)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
    ENDIF(CMAKE_COMPILER_IS_GNUCC)
ENDIF(UNIX)

# Find and Include HPX
IF(HPX_ROOT AND NOT HPX_DIR)
    SET(HPX_DIR "${HPX_ROOT}/lib/cmake/HPX")
ENDIF()

# Instruct cmake to find the HPX settings, it looks for HPXConfig.cmake in ${HPX_DIR}
find_package(HPX)

include_directories(${HPX_INCLUDE_DIR})
link_directories(${HPX_LIBRARY_DIR})

IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    include_directories(/home/alireza/lib/pxfs-debug/include/)
    link_directories(/home/alireza/lib/pxfs-debug/lib)
ELSEIF(CMAKE_BUILD_TYPE STREQUAL "Release")
    include_directories(/home/alireza/lib/pxfs-release/include/)
    link_directories(/home/alireza/lib/pxfs-release/lib)
ENDIF()

# Adjust auxiliary build settings for HPX to work properly
IF(NOT MSVC)
    set(HPX_RPATH ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}${HPX_RPATH})
ELSE()
    # enable solution folders for MSVC
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    IF(HPX_NO_INSTALL)
        set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/$(Configuration)" CACHE PATH
                "Prefix prepended to install directories." FORCE)
    ENDIF()
ENDIF()

add_hpx_executable(hello_world
        SOURCES hello_world.cpp
        FOLDER
        COMPONENT_DEPENDENCIES iostreams base_file)

add_subdirectory(components)
add_subdirectory(tests)
add_subdirectory(interposition)


add_hpx_executable(launch_process_test
        SOURCES launch_process.cpp
        DEPENDENCIES iostreams_component process_component
        launch_process_test_server_component
        HPX_PREFIX ${HPX_BUILD_PREFIX}
        FOLDER)

add_hpx_executable(launched_process_test
        SOURCES launched_process.cpp
        HPX_PREFIX ${HPX_BUILD_PREFIX}
        FOLDER
        COMPONENT_DEPENDENCIES launch_process_test_server)

add_hpx_executable(launcher
        SOURCES launcher.cpp
        HPX_PREFIX ${HPX_BUILD_PREFIX}
        COMPONENT_DEPENDENCIES iostreams base_file process launch_process_test_server
        FOLDER)