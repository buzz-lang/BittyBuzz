# ==========================================
# =          FUNCTION DEFINITIONS          =
# ==========================================

# Copies all files to the binary directory so that
# they can be found by the test executables.
function (copy_test_resources)
    # Find files under the directory.
#    file(GLOB files "*")
    set(files
            1_InstrTest.bo
            1_InstrTest.bbo
            2_IfTest.bzz
            3_test1.bzz
            3_test1.bst
            4_AllFeaturesTest.bzz
            4_AllFeaturesTest.bst
    )

    # Copy those files to a new ressource directory.
    file(COPY ${files} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
endfunction()


# Adds all BittyBuzz Objects as dependencies of the executables.
function (generate_all_bbos)
#    file(GLOB BUZZ_SOURCES "*.bzz")
    set(BUZZ_SOURCES
            2_IfTest.bzz
            3_test1.bzz
            4_AllFeaturesTest.bzz
    )
#    file(GLOB BST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.bst")
    set(BST_SOURCES
            3_test1.bst
            4_AllFeaturesTest.bst
    )

    foreach(buzz_source ${BUZZ_SOURCES})
        get_filename_component(basename ${buzz_source} NAME_WE)

        # Set varaibles
        set(BBO_TARGET ${basename}_BBO)

        # Generate BittyBuzz Object
        set(SOUGHT_BST_FILE "${basename}.bst")
        if (SOUGHT_BST_FILE IN_LIST BST_SOURCES)
            generate_bbo(${BBO_TARGET} ${CMAKE_CURRENT_BINARY_DIR} ${buzz_source} "${CMAKE_CURRENT_BINARY_DIR}/${SOUGHT_BST_FILE}")
        else()
            generate_bbo(${BBO_TARGET} ${CMAKE_CURRENT_BINARY_DIR} ${buzz_source} "")
        endif()
        add_dependencies(test_resources ${BBO_TARGET})
    endforeach()
endfunction()


# ==========================================
# =              CMAKE SCRIPT              =
# ==========================================

copy_test_resources()
generate_all_bbos()
