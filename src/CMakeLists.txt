project(bittybuzz)
cmake_minimum_required(VERSION 3.3.2)

# Add new build types
message("* Adding build types...")
SET(CMAKE_CXX_FLAGS_KILODEBUG "${CMAKE_CXX_FLAGS_DEBUG}"
        CACHE STRING "Flags used by the AVR-C++ compiler to debug kilobots."
        FORCE)
SET(CMAKE_C_FLAGS_KILODEBUG "${CMAKE_C_FLAGS_DEBUG}"
        CACHE STRING "Flags used by the AVR-C compiler to debug kilobots."
        FORCE)
SET(CMAKE_EXE_LINKER_FLAGS_KILODEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG}"
        CACHE STRING "Flags used for linking binaries to debug kilobots."
        FORCE)
SET(CMAKE_SHARED_LINKER_FLAGS_KILODEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}"
        CACHE STRING "Flags used by the shared libraries linker to debug kilobots."
        FORCE)
MARK_AS_ADVANCED(
        CMAKE_CXX_FLAGS_KILODEBUG
        CMAKE_C_FLAGS_KILODEBUG
        CMAKE_EXE_LINKER_FLAGS_KILODEBUG
        CMAKE_SHARED_LINKER_FLAGS_KILODEBUG)

#
# Global options and variables
#
set(INCLUDE_FILES_CMAKE ${CMAKE_SOURCE_DIR}/cmake)


#
# Add directories to the paths
#
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
link_directories(${CMAKE_BINARY_DIR}/bittybuzz)

#
# Prepare config.h file, making sure parameters have been set
#
include(${INCLUDE_FILES_CMAKE}/BittyBuzzConfig.cmake)

if (BBZ_USE_FLOAT)
    add_definitions(-DBBZ_USE_FLOAT=1)
endif ()

#
# Host compiler values
#
set(HOST_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-comment -fshort-enums" CACHE STRING "GCC flags for the host compiler.")
find_program(HOST_C_COMPILER gcc)
# Set the current compiler as "HOST" if we didn't set it to anything yet.
set(CURRENT_COMPILER "HOST" CACHE STRING "Which compiler we are using.")

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
    add_subdirectory(kilobot)
endif()

if (NOT CMAKE_CROSSCOMPILING)
    include (CTest)
    add_subdirectory(testing)
endif()

add_subdirectory(doc)
