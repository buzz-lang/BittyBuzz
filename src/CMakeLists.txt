project(bittybuzz)
cmake_minimum_required(VERSION 3.3.2)

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

#
# Host compiler values
#
set(HOST_C_FLAGS "${CMAKE_C_FLAGS} -Wall" CACHE STRING "GCC flags for the host compiler.")
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
# Descend into the subdirectories
#
add_subdirectory(bittybuzz)
add_subdirectory(testing)
if(CMAKE_CROSSCOMPILING)
    add_subdirectory(kilobot)
endif()
add_subdirectory(doc)
