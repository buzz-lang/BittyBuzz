set(BBZ_HEADERS
  bbzfloat.h
  bbzheap.h
  bbztable.h
  bbztype.h
  bbzdarray.h)

set(BBZ_SOURCES
  ${BBZ_HEADERS}
  bbzfloat.c
  bbzheap.c
  bbztable.c
  bbztype.c
  bbzdarray.c)

if(NOT CMAKE_CROSSCOMPILING)
  add_library(bittybuzz STATIC ${BBZ_SOURCES})
else(NOT CMAKE_CROSSCOMPILING)
  if("${BBZ_ROBOT}" STREQUAL "kilobot")
    kilobot_add_library(bittybuzz STATIC ${BBZ_SOURCES})
  endif("${BBZ_ROBOT}" STREQUAL "kilobot")
endif(NOT CMAKE_CROSSCOMPILING)