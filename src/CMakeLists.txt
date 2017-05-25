project(bittybuzz)
cmake_minimum_required(VERSION 2.8.12)

#
# Global options
#

option(ENABLE_AUTOMATED_TESTING
  "Enables the use of the Boost Unit Test Framework, if it is found.
   Only considered when compiling for PC."
  ON)

#
# Add these directories to the paths
#
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
link_directories(${CMAKE_BINARY_DIR}/bittybuzz)

#
# Prepare config.h file, making sure parameters have been set
#
if("${HEAP_SIZE}" STREQUAL "")
  message(STATUS "Using default value for HEAP_SIZE (-DHEAP_SIZE=1024)")
  set(HEAP_SIZE 1024)
endif("${HEAP_SIZE}" STREQUAL "")
if("${HEAP_ELEMS_PER_TSEG}" STREQUAL "")
  message(STATUS "Using default value for HEAP_ELEMS_PER_TSEG (-DHEAP_ELEMS_PER_TSEG=5)")
  set(HEAP_ELEMS_PER_TSEG 5)
endif("${HEAP_ELEMS_PER_TSEG}" STREQUAL "")
if("${STACK_SIZE}" STREQUAL "")
  message(STATUS "Using default value for STACK_SIZE (-DSTACK_SIZE=256)")
  set(STACK_SIZE 256)
endif("${STACK_SIZE}" STREQUAL "")
if("${MAX_ACTREC_COUNT}" STREQUAL "")
  message(STATUS "Using default value for MAX_ACTREC_COUNT (-DMAX_ACTREC_COUNT=32)")
  set(MAX_ACTREC_COUNT 32)
endif("${MAX_ACTREC_COUNT}" STREQUAL "")
configure_file(bittybuzz/config.h.in ${CMAKE_BINARY_DIR}/bittybuzz/config.h @ONLY)

#
# GCC flags
#
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

#
# Descend into the subdirectories
#
add_subdirectory(bittybuzz)
# TODO Make some tests work on the kilobot?
add_subdirectory(testing)
add_subdirectory(doc)
