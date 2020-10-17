use_native_compiler()

set(BBZ_HEADERS
        bbzdarray.h
        bbzenums.h
        bbzfloat.h
        bbzheap.h
        bbzinclude.h
        bbzinmsg.h
        bbzmsg.h
        bbzneighbors.h
#        bbzobjringbuf.h
        bbzoutmsg.h
        bbzringbuf.h
        bbzstrids.h
        bbzswarm.h
        bbztable.h
        bbztype.h
        bbzutil.h
        bbzvm.h
        bbzvstig.h
        config.h
)
set(BBZ_SOURCES
        bbzdarray.c
        bbzfloat.c
        bbzheap.c
        bbzinmsg.c
        bbzmsg.c
        bbzneighbors.c
        bbzoutmsg.c
        bbzringbuf.c
        bbzswarm.c
        bbztable.c
        bbztype.c
        bbzutil.c
        bbzvm.c
        bbzvstig.c
)
list(APPEND BBZ_SOURCES ${BBZ_HEADERS})

if(NOT CMAKE_CROSSCOMPILING)
    add_library(bittybuzz STATIC ${BBZ_SOURCES})
    add_library(bittybuzz_dl SHARED ${BBZ_SOURCES})
    set_target_properties(bittybuzz_dl PROPERTIES OUTPUT_NAME bittybuzz)
else(NOT CMAKE_CROSSCOMPILING)
    set(BBZCROSSCOMPILING 1)
    if("${BBZ_ROBOT}" STREQUAL "kilobot")
        kilobot_add_library(bittybuzz STATIC ${BBZ_SOURCES})
    endif("${BBZ_ROBOT}" STREQUAL "kilobot")
    if("${BBZ_ROBOT}" STREQUAL "zooids")
        zooids_add_library(bittybuzz STATIC ${BBZ_SOURCES})
    endif("${BBZ_ROBOT}" STREQUAL "zooids")
    if("${BBZ_ROBOT}" STREQUAL "crazyflie")
        crazyflie_add_library(bittybuzz STATIC ${BBZ_SOURCES})
    endif("${BBZ_ROBOT}" STREQUAL "crazyflie")
endif(NOT CMAKE_CROSSCOMPILING)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

add_subdirectory(exec)
add_subdirectory(util)
