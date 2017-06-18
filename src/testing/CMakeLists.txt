# ==========================================
# =          FUNCTION DEFINITIONS          =
# ==========================================

# Adds all executables in the testing directory and adds them as
# test executables.
function(add_tests)
    file(GLOB test_sources "*.c")

    foreach(test_source ${test_sources})
        get_filename_component(test_executable ${test_source} NAME_WE)
        add_executable(${test_executable} ${test_source})
        target_link_libraries(${test_executable} bittybuzz ${TESTING_EXTRA_LIBS})
        add_dependencies(test_executables ${test_executable})
        add_test(NAME ${test_executable}
                 COMMAND "${test_executable}" )
    endforeach()
endfunction()


# ==========================================
# =              CMAKE SCRIPT              =
# ==========================================
use_native_compiler()

add_custom_target(test_resources)
add_custom_target(test_executables ALL DEPENDS test_resources)

include (CTest)
add_tests()
add_subdirectory(resources)
