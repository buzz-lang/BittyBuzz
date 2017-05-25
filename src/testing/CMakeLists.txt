if (CMAKE_CROSSCOMPILING)
    # Add the tests that work on the kilobots
    add_subdirectory(mcu_test)
else()
    # Add the tests that work on PC
    add_subdirectory(pc_test)
endif()