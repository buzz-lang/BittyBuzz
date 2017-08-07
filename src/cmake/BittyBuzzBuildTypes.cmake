if (CMAKE_BUILD_TYPE MATCHES "Debug")
    set(DEBUG ON)
endif()

# Add new build types
message(STATUS "Adding build types...")
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
SET(CMAKE_CXX_FLAGS_XTREMEDEBUG "${CMAKE_CXX_FLAGS_DEBUG}"
        CACHE STRING "Flags used by the C++ compiler to debug with extreme memory restrictions."
        FORCE)
SET(CMAKE_C_FLAGS_XTREMEDEBUG "${CMAKE_C_FLAGS_DEBUG}"
        CACHE STRING "Flags used by the C compiler to debug with extreme memory restrictions."
        FORCE)
SET(CMAKE_EXE_LINKER_FLAGS_XTREMEDEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG}"
        CACHE STRING "Flags used for linking binaries to debug with extreme memory restrictions."
        FORCE)
SET(CMAKE_SHARED_LINKER_FLAGS_XTREMEDEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}"
        CACHE STRING "Flags used by the shared libraries linker to debug with extreme memory restrictions."
        FORCE)
MARK_AS_ADVANCED(
        CMAKE_CXX_FLAGS_KILODEBUG
        CMAKE_C_FLAGS_KILODEBUG
        CMAKE_EXE_LINKER_FLAGS_KILODEBUG
        CMAKE_SHARED_LINKER_FLAGS_KILODEBUG
        CMAKE_CXX_FLAGS_XTREMEDEBUG
        CMAKE_C_FLAGS_XTREMEDEBUG
        CMAKE_EXE_LINKER_FLAGS_XTREMEDEBUG
        CMAKE_SHARED_LINKER_FLAGS_XTREMEDEBUG)