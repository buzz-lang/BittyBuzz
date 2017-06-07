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