use_native_compiler()

file(GLOB BBZ_HEADERS "*.h")
file(GLOB BBZ_SOURCES "*.c")
list(APPEND BBZ_SOURCES ${BBZ_HEADERS})

if(NOT CMAKE_CROSSCOMPILING)
  add_library(bittybuzz STATIC ${BBZ_SOURCES})
else(NOT CMAKE_CROSSCOMPILING)
  if("${BBZ_ROBOT}" STREQUAL "kilobot")
    kilobot_add_library(bittybuzz STATIC ${BBZ_SOURCES})
  endif("${BBZ_ROBOT}" STREQUAL "kilobot")
endif(NOT CMAKE_CROSSCOMPILING)

add_subdirectory(exec)