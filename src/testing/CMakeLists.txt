# ==========================================
# =          FUNCTION DEFINITIONS          =
# ==========================================

# Adds all executables in the testing directory,
# taking the robot type into account.
function(add_test_executables)
    file(GLOB test_sources "*.c")

    foreach(test_source ${test_sources})
        get_filename_component(test_executable ${test_source} NAME_WE)
        add_executable(${test_executable} ${test_source})
        target_link_libraries(${test_executable} bittybuzz ${TESTING_EXTRA_LIBS})
        add_dependencies(test_executables ${test_executable})
    endforeach()
endfunction()


# ==========================================
# =              CMAKE SCRIPT              =
# ==========================================
use_native_compiler()

add_custom_target(test_resources)
add_custom_target(test_executables ALL DEPENDS test_resources)

add_test_executables()
add_subdirectory(resources)