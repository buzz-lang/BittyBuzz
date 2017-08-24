# ====================================
# = DOXYGEN DOCUMENTATION GENERATION =
# ====================================

# -----------------
# -   Functions   -
# -----------------

# Sets the varaibles required by the doc generation.
function(set_doc_variables)
    set(DOC_TRG "doc" PARENT_SCOPE)
    set(IN_DOXYFILE  "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in" PARENT_SCOPE)
    set(DOXYFILE     "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"    PARENT_SCOPE)
endfunction()

# Configures the Doxyfile.
function(configure_doxyfile)
    configure_file(${IN_DOXYFILE} ${DOXYFILE})
endfunction()

# Adds the Doxygen target.
function(add_doc_target)
    add_custom_target(${DOC_TRG})
    add_custom_command(
        TARGET      ${DOC_TRG}
        DEPENDS     ${DOXYFILE} ${BBZ_SOURCES}
        COMMAND     ${DOXYGEN}
        ARGS        ${DOXYFILE}
    )
endfunction()

# -----------------
# - CMake script  -
# -----------------

find_program(DOXYGEN doxygen)

if (DOXYGEN)
    set_doc_variables()
    configure_doxyfile()
    add_doc_target()
endif()