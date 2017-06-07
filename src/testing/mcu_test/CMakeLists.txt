# ==========================================
# =          FUNCTION DEFINITIONS          =
# ==========================================

# Adds all executables in the testing directory,
# taking the robot type into account.
function(add_test_executables)
    file(GLOB test_sources "*.c")
    include_directories(${CMAKE_CURRENT_BINARY_DIR})

    foreach(test_source ${test_sources})
        get_filename_component(test_executable ${test_source} NAME_WE)
        # Add the test executable
        if ("${BBZ_ROBOT}" STREQUAL "kilobot")
            kilobot_add_executable(${test_executable} ${test_source})
            kilobot_target_link_libraries(${test_executable} bittybuzz)
        endif()
        add_dependencies(test_executables ${test_executable})
    endforeach()
endfunction()


# ==========================================
# =              CMAKE SCRIPT              =
# ==========================================

add_test_executables()

add_subdirectory(util)
add_subdirectory(resources)
