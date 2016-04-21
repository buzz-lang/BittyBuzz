project(bittybuzz)
cmake_minimum_required(VERSION 2.8.12)

#
# Add these directories to the paths
#
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
link_directories(${CMAKE_BINARY_DIR}/bittybuzz)

#
# Prepare config.h file, making sure parameters have been set
#
if("${HEAP_SIZE}" STREQUAL "")
  message(FATAL_ERROR "You must specify a value for HEAP_SIZE (e.g., cmake -DHEAP_SIZE=1024 ...)")
endif("${HEAP_SIZE}" STREQUAL "")
if("${HEAP_ELEMS_PER_TSEG}" STREQUAL "")
  message(FATAL_ERROR "You must specify a value for HEAP_ELEMS_PER_TSEG (e.g., cmake -DHEAP_ELEMS_PER_TSEG=5 ...)")
endif("${HEAP_ELEMS_PER_TSEG}" STREQUAL "")
configure_file(bittybuzz/config.h.in ${CMAKE_BINARY_DIR}/bittybuzz/config.h @ONLY)

#
# Descend into the subdirectories
#
add_subdirectory(bittybuzz)
#add_subdirectory(kilolib)
add_subdirectory(testing)
