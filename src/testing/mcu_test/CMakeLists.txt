# ==========================================
# =          FUNCTION DEFINITIONS          =
# ==========================================

function(add_bcode_generator)
    find_program(CC gcc)
    set(BYTECODE_GEN_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/util/bcodegen.c)
    set(BYTECODE_GEN ${CMAKE_CURRENT_BINARY_DIR}/bcodegen${CMAKE_EXECUTABLE_SUFFIX})
    set(BYTECODE_GEN ${BYTECODE_GEN} PARENT_SCOPE)

    add_custom_command(OUTPUT ${BYTECODE_GEN}
        COMMAND ${CC} -o ${BYTECODE_GEN} ${BYTECODE_GEN_SOURCE}
    )
endfunction()

# Adds all executables in the testing directory,
# taking the robot type into account.
# Also generates the header file containing the BittyBuzz bytecode.
function(add_test_executables)
    file(GLOB test_sources "*.c")

    foreach(test_source ${test_sources})
        get_filename_component(test_executable ${test_source} NAME_WE)

        # Add the bytecode header
        set(bcode_infile ${CMAKE_CURRENT_SOURCE_DIR}/ressources/${test_executable}.bo)
        set(bcode_header ${CMAKE_CURRENT_BINARY_DIR}/${test_executable}.h)
        add_custom_command(OUTPUT ${bcode_header}
            COMMAND ${BYTECODE_GEN} ${bcode_infile} ${bcode_header}
            DEPENDS ${BYTECODE_GEN} ${bcode_infile}
        )

        # Add the test executable
        if ("${BBZ_ROBOT}" STREQUAL "kilobot")
            kilobot_add_executable(${test_executable} ${test_source} ${bcode_header})
            kilobot_target_link_libraries(${test_executable} bittybuzz)
        endif()

    endforeach()
    # Let the executables find the headers produced by the generator
    include_directories(${CMAKE_CURRENT_BINARY_DIR})
endfunction()

# ==========================================
# =              CMAKE SCRIPT              =
# ==========================================

add_bcode_generator()
add_test_executables()