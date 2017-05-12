# ==========================================
# =          FUNCTION DEFINITIONS          =
# ==========================================


# Enables automated testing when not crosscompiling
function (configure_automated_testing)
    if (ENABLE_AUTOMATED_TESTING AND NOT CMAKE_CROSSCOMPILING)
        # Find Boost's unit test library
        find_package(Boost COMPONENTS unit_test_framework)
        if (${Boost_FOUND})
            # Include it
            set(BBZ_USE_AUTOMATED_TESTS 1)
            include_directories(${Boost_INCLUDE_DIRS})
            
            # Compile the test files as C++ and link against Boost's library
            set(TESTING_EXTRA_LIBS ${TESTING_EXTRA_LIBS} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES})
            file(GLOB test_sources "*.c")
            foreach(test_source ${test_sources})
                set_source_files_properties(${test_source} PROPERTIES LANGUAGE "CXX")
            endforeach()
        endif (${Boost_FOUND})
    endif()
endfunction()


# Adds all executables in the testing directory,
# taking the robot type into account.
function(add_test_executables robot_name)
    file(GLOB test_sources "*.c")

    if (robot_name)
        set(prefix "${robot_name}_")
    endif()

    foreach(test_source ${test_sources})
        get_filename_component(test_excutable ${test_source} NAME_WE)
        
        if ("${BBZ_ROBOT}" STREQUAL "kilobot")
            add_executable(${test_excutable} ${test_source})
            target_link_libraries(${test_excutable} bittybuzz ${TESTING_EXTRA_LIBS})
        elseif ()
            kilobot_add_executable(${test_excutable} ${test_source})
            kilobot_target_link_libraries(${test_excutable} bittybuzz ${TESTING_EXTRA_LIBS})
        endif()
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

add_test_executables("${BBZ_ROBOT}")
copy_test_ressources()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testingconfig.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/testingconfig.h)