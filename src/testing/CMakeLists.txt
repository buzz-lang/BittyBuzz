
# Enable automated testing when not crosscompiling
if (ENABLE_AUTOMATED_TESTING)

  if (NOT CMAKE_CROSSCOMPILING)
    include(CTest)
    find_package(Boost COMPONENTS unit_test_framework)

    if (${Boost_FOUND})
      set(BBZ_USE_AUTOMATED_TESTS 1)
      include_directories(${Boost_INCLUDE_DIRS})
      
      set(TESTING_EXTRA_LIBS ${TESTING_EXTRA_LIBS} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES})
      # TODO Use file globs
      set_source_files_properties(testfloat.c  PROPERTIES LANGUAGE "CXX")
      set_source_files_properties(testheap.c   PROPERTIES LANGUAGE "CXX")
      set_source_files_properties(testdarray.c PROPERTIES LANGUAGE "CXX")
    endif (${Boost_FOUND})
  endif (NOT CMAKE_CROSSCOMPILING)

endif (ENABLE_AUTOMATED_TESTING)


if(NOT CMAKE_CROSSCOMPILING)
  # add_executable(testringbuf
  #   ../bittybuzz/bbzfloat.h ../bittybuzz/bbzfloat.c
  #   ../bittybuzz/bbztype.h ../bittybuzz/bbztype.c
  #   ../bittybuzz/bbzringbuf.h ../bittybuzz/bbzringbuf.c
  #   ../bittybuzz/bbzobjringbuf.h
  #   testringbuf.c)

  add_executable(testfloat testfloat.c)
  target_link_libraries(testfloat bittybuzz ${TESTING_EXTRA_LIBS})

  add_executable(testheap testheap.c)
  target_link_libraries(testheap bittybuzz ${TESTING_EXTRA_LIBS})

  add_executable(testdarray testdarray.c)
  target_link_libraries(testdarray bittybuzz ${TESTING_EXTRA_LIBS})
  
else(NOT CMAKE_CROSSCOMPILING)
  
  if("${BBZ_ROBOT}" STREQUAL "kilobot")
    
    kilobot_add_executable(testfloat testfloat.c)
    kilobot_target_link_libraries(testfloat bittybuzz)

    kilobot_add_executable(testheap testheap.c)
    kilobot_target_link_libraries(testheap bittybuzz)

    kilobot_add_executable(testdarray testdarray.c)
    kilobot_target_link_libraries(testdarray bittybuzz)
    
  endif("${BBZ_ROBOT}" STREQUAL "kilobot")
  
endif(NOT CMAKE_CROSSCOMPILING)


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testingconfig.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/testingconfig.h)