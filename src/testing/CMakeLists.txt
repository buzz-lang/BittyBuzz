# ==========================================
# =          FUNCTION DEFINITIONS          =
# ==========================================

# Adds all executables in the testing directory and adds them as
# test executables.
function(add_tests)
    set(test_sources
        testdarray.c
        testfloat.c
        testheap.c
        testmsgs.c
        testringbuf.c
        testvm.c
    )

    if (NOT BBZ_DISABLE_MESSAGES)
        if (NOT BBZ_DISABLE_NEIGHBORS)
            list(APPEND test_sources testneighbors.c)
        endif ()
        if (NOT BBZ_DISABLE_VSTIGS)
            list(APPEND test_sources testvstig.c)
        endif ()
        if (NOT BBZ_DISABLE_SWARMS)
            list(APPEND test_sources testswarm.c)
        endif ()
    endif ()

    foreach(test_source ${test_sources})
        get_filename_component(test_executable ${test_source} NAME_WE)
        add_executable(${test_executable} ${test_source})
        target_link_libraries(${test_executable} bittybuzz ${TESTING_EXTRA_LIBS})
        add_dependencies(test_executables ${test_executable})
        add_dependencies(${test_executable} test_resources)
        add_test(NAME ${test_executable}
                 COMMAND "${test_executable}" )
    endforeach()
endfunction()


# ==========================================
# =              CMAKE SCRIPT              =
# ==========================================
use_native_compiler()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3")

add_custom_target(test_resources)
add_custom_target(test_executables ALL)

add_tests()
add_subdirectory(resources)
