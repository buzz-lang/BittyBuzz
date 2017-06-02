project(bittybuzz)
cmake_minimum_required(VERSION 3.3.2)

#
# Global options
#

# Nothing to see here. Read along.


#
# Add these directories to the paths
#
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
link_directories(${CMAKE_BINARY_DIR}/bittybuzz)

#
# Prepare config.h file, making sure parameters have been set
#
function (config_value value default)
    if("${${value}}" STREQUAL "")
        message(STATUS "Using default value for ${value} (-D${value}=${default})")
        set(${value} ${default} PARENT_SCOPE)
    endif()
endfunction()

config_value(BBZHEAP_SIZE 1024)
config_value(BBZHEAP_ELEMS_PER_TSEG 5)
config_value(BBZSTACK_SIZE 256)
config_value(RESERVED_ACTREC_MAX 32)

configure_file(bittybuzz/config.h.in ${CMAKE_BINARY_DIR}/bittybuzz/config.h @ONLY)

#
# Host compiler values
#
set(HOST_C_FLAGS "${CMAKE_C_FLAGS} -Wall" CACHE STRING "GCC flags for the host compiler.")
find_program(HOST_C_COMPILER gcc)
# Set the current compiler as "HOST" if we didn't set it to anything yet.
set(CURRENT_COMPILER "HOST" CACHE STRING "Which compiler we are using.")

#
# Utility macros to quickly swap between compilers
#

# Swaps to the host's compiler in order to generate
# utility executables and libraries.
macro(use_host_compiler)
    if (${CURRENT_COMPILER} STREQUAL "NATIVE")
        # Save current native flags
        set(NATIVE_C_FLAGS ${CMAKE_C_FLAGS} CACHE STRING "GCC flags for the native compiler." FORCE)

        # Change compiler
        set(CMAKE_SYSTEM_NAME ${CMAKE_HOST_SYSTEM_NAME})
        set(CMAKE_SYSTEM_PROCESSOR ${CMAKE_HOST_SYSTEM_PROCESSOR})
        set(CMAKE_C_COMPILER ${HOST_C_COMPILER})
        set(CMAKE_C_FLAGS ${HOST_C_FLAGS})
        set(CURRENT_COMPILER "HOST" CACHE STRING "Which compiler we are using." FORCE)
    endif()
endmacro()

# Swaps to the compiler for the system we are programming for.
macro(use_native_compiler)
    if (CMAKE_CROSSCOMPILING AND ${CURRENT_COMPILER} STREQUAL "HOST")
        # Save current host flags
        set(HOST_C_FLAGS ${CMAKE_C_FLAGS} CACHE STRING "GCC flags for the host compiler." FORCE)

        # Change compiler
        set(CMAKE_SYSTEM_NAME ${NATIVE_SYSTEM_NAME})
        set(CMAKE_SYSTEM_PROCESSOR ${NATIVE_SYSTEM_PROCESSOR})
        set(CMAKE_C_COMPILER ${NATIVE_C_COMPILER})
        set(CMAKE_C_FLAGS ${NATIVE_C_FLAGS})
        set(CURRENT_COMPILER "NATIVE" CACHE STRING "Which compiler we are using." FORCE)
    endif()
endmacro()

#
# Find Buzz programs
#

function(buzz_find_program varOut prgm)
    set(path ${varOut}_PATH)
    find_program(${path} ${prgm})
    if ("${${path}}" STREQUAL "${path}-NOTFOUND")
        message(SEND_ERROR "Buzz program \"${prgm}\" could not be found. "
                "Compile and install instructions can be found at "
                "https://github.com/MISTLab/Buzz")
        set(${varOut} "${varOut}-NOTFOUND" CACHE STRING "Path to a Buzz program.")
    else()
        set(${varOut} "${${path}}" CACHE STRING "Path to a Buzz program.")
    endif()
endfunction()

buzz_find_program(BZZC   bzzc)
buzz_find_program(BZZPAR bzzparse)
buzz_find_program(BZZASM bzzasm)
buzz_find_program(BZZRUN bzzrun)

#
# Descend into the subdirectories
#
add_subdirectory(bittybuzz)
add_subdirectory(testing)
if(CMAKE_CROSSCOMPILING)
    add_subdirectory(kilobot)
endif()
add_subdirectory(doc)
