if(NOT CMAKE_CROSSCOMPILING)
  # add_executable(testringbuf
  #   ../bittybuzz/bbzfloat.h ../bittybuzz/bbzfloat.c
  #   ../bittybuzz/bbztype.h ../bittybuzz/bbztype.c
  #   ../bittybuzz/bbzringbuf.h ../bittybuzz/bbzringbuf.c
  #   ../bittybuzz/bbzobjringbuf.h
  #   testringbuf.c)

  add_executable(testfloat testfloat.c)
  target_link_libraries(testfloat bittybuzz)

  add_executable(testheap testheap.c)
  target_link_libraries(testheap bittybuzz)
  
else(NOT CMAKE_CROSSCOMPILING)
  
  if("${BBZ_ROBOT}" STREQUAL "kilobot")
    
    kilobot_add_executable(testfloat testfloat.c)
    kilobot_target_link_libraries(testfloat bittybuzz)

    kilobot_add_executable(testheap testheap.c)
    kilobot_target_link_libraries(testheap bittybuzz)
    
  endif("${BBZ_ROBOT}" STREQUAL "kilobot")
  
endif(NOT CMAKE_CROSSCOMPILING)
