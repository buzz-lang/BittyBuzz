project(bittybuzz)
cmake_minimum_required(VERSION 3.5.1)

#
# Global options and variables
#
set(INCLUDE_FILES_CMAKE ${CMAKE_SOURCE_DIR}/cmake)

#
# Add new Build Types
#
include(${INCLUDE_FILES_CMAKE}/BittyBuzzBuildTypes.cmake)

#
# Add directories to the paths
#
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
link_directories(${CMAKE_BINARY_DIR}/bittybuzz)

#
# Prepare config.h file, making sure parameters have been set
#
include(${INCLUDE_FILES_CMAKE}/BittyBuzzConfig.cmake)

#
# Host compiler values
#
set(HOST_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-comment -fshort-enums" CACHE STRING "GCC flags for the host compiler.")
find_program(HOST_C_COMPILER gcc)

#
# Add compiler-swapping macros
#
include(${INCLUDE_FILES_CMAKE}/BittyBuzzCompilerMacros.cmake)

#
# Add .bo and .bbo generator functions
#
include(${INCLUDE_FILES_CMAKE}/BittyBuzzGeneratorFunctions.cmake)

#
# Find Buzz programs
#
include(${INCLUDE_FILES_CMAKE}/BittyBuzzFindBuzzPrgms.cmake)

#
# Find Argos
#
#include(${INCLUDE_FILES_CMAKE}/BittyBuzzBuildChecks.cmake)

#
# Descend into the subdirectories
#
add_subdirectory(bittybuzz)

if (CMAKE_CROSSCOMPILING)
        add_subdirectory(${BBZ_ROBOT})
endif()

if (NOT CMAKE_CROSSCOMPILING)
    include (CTest)
    add_subdirectory(testing)
endif()

add_subdirectory(doc)
