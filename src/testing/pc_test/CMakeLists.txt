# ==========================================
# =          FUNCTION DEFINITIONS          =
# ==========================================

# Adds all executables in the testing directory,
# taking the robot type into account.
function(add_test_executables)
    file(GLOB test_sources "*.c")

    foreach(test_source ${test_sources})
        get_filename_component(test_excutable ${test_source} NAME_WE)
        add_executable(${test_excutable} ${test_source})
        target_link_libraries(${test_excutable} bittybuzz ${TESTING_EXTRA_LIBS})
    endforeach()
endfunction()


# Copies all files under ./ressources/ to the binary directory
# so that they can be found by the test executables.
function (copy_test_ressources)
    # Find files under the directory.
    file(GLOB files "ressources/*")

    # Copy those files to a new ressource directory.
    file(COPY ${files} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/ressources")
endfunction()



# ==========================================
# =              CMAKE SCRIPT              =
# ==========================================

add_test_executables()
copy_test_ressources()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})