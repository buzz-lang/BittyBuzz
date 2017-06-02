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


# ==========================================
# =          FUNCTION DEFINITIONS          =
# ==========================================

# Generates a BittyBuzz object file.
# bst_source can be left empty in order not to use any Buzz Symbols Table.
function(generate_bbo _TARGET bzz_outdir bzz_source bst_source)
  get_filename_component(BZZ_BASENAME ${bzz_source} NAME_WE)
  set(BZZ_BASEPATH "${bzz_outdir}/${BZZ_BASENAME}")

  set(BASM_FILE ${BZZ_BASEPATH}.basm)
  set(BO_FILE   ${BZZ_BASEPATH}.bo)
  set(BDB_FILE  ${BZZ_BASEPATH}.bdb)
  set(BBO_FILE  ${BZZ_BASEPATH}.bbo)

  # .bzz -> .basm
  if (NOT ${bst_source} STREQUAL "")
    add_custom_command(OUTPUT ${BASM_FILE}
            COMMAND ${BZZPAR} ${bzz_source} ${BASM_FILE} ${bst_source})
  else()
    add_custom_command(OUTPUT ${BASM_FILE}
            COMMAND ${BZZPAR} ${bzz_source} ${BASM_FILE})
  endif()

  # .basm -> .bo ; .basm -> .bdb
  add_custom_command(OUTPUT ${BO_FILE} ${BDB_FILE}
          COMMAND ${BZZASM} ${BASM_FILE} ${BO_FILE} ${BDB_FILE}
          DEPENDS ${BASM_FILE})

  # .bo -> .bbo
  add_custom_command(OUTPUT ${BBO_FILE}
          COMMAND "$<TARGET_FILE:bo2bbo>" ${BO_FILE} ${BBO_FILE}
          DEPENDS ${BO_FILE})

  # Add the main target
  add_custom_target(${_TARGET} DEPENDS ${BBO_FILE} "$<TARGET_FILE:bo2bbo>")
endfunction()

# ==========================================
# =          FUNCTION DEFINITIONS          =
# ==========================================

# Generates a Buzz object file.
# bst_source can be left empty in order not to use any Buzz Symbols Table.
function(generate_bo _TARGET bzz_outdir bzz_source bst_source)
  get_filename_component(BZZ_BASENAME ${bzz_source} NAME_WE)
  set(BZZ_BASEPATH "${bzz_outdir}/${BZZ_BASENAME}")

  set(BASM_FILE ${BZZ_BASEPATH}.basm)
  set(BO_FILE   ${BZZ_BASEPATH}.bo)
  set(BDB_FILE  ${BZZ_BASEPATH}.bdb)

  # .bzz -> .basm
  if (NOT ${bst_source} STREQUAL "")
    add_custom_command(OUTPUT ${BASM_FILE}
            COMMAND ${BZZPAR} ${bzz_source} ${BASM_FILE} ${bst_source})
  else()
    add_custom_command(OUTPUT ${BASM_FILE}
            COMMAND ${BZZPAR} ${bzz_source} ${BASM_FILE})
  endif()

  # .basm -> .bo ; .basm -> .bdb
  add_custom_command(OUTPUT ${BO_FILE} ${BDB_FILE}
          COMMAND ${BZZASM} ${BASM_FILE} ${BO_FILE} ${BDB_FILE}
          DEPENDS ${BASM_FILE})

  # Add the main target
  add_custom_target(${_TARGET} DEPENDS ${BO_FILE} "$<TARGET_FILE:bo2bbo>")
endfunction()



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
