# ==========================================
# =          FUNCTION DEFINITIONS          =
# ==========================================

# Generates a hex file given a C and a Buzz source.
# bst_source can be left empty in order not to use any Buzz Symbols Table.
function(generate_hex c_source bzz_source)
    get_filename_component(BZZ_BASENAME ${bzz_source} NAME_WE)
    set(GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/gen/${BZZ_BASENAME}")
    set(BZZ_BASEPATH "${GEN_DIR}/${BZZ_BASENAME}")
    set(GENSYMS_FILE "${GEN_DIR}/bbzsymbols.h")
    set(LOG_FILE "${GEN_DIR}/log.txt")

    set(BASM_FILE ${BZZ_BASEPATH}.basm)
    set(BO_FILE   ${BZZ_BASEPATH}.bo)
    set(BDB_FILE  ${BZZ_BASEPATH}.bdb)
    set(BBO_FILE  ${BZZ_BASEPATH}.bbo)
    set(ELF_FILE  ${BZZ_BASEPATH}.elf)
    set(HEX_FILE  ${BZZ_BASEPATH}.hex)
    set(MAP_FILE  ${BZZ_BASEPATH}.map)

    # .bzz -> .basm, ...
    if (NOT ${ARGN} STREQUAL "")
        add_custom_command(OUTPUT ${HEX_FILE} ${GEN_DIR}
                BYPRODUCTS ${BASM_FILE} ${BO_FILE} ${BDB_FILE} ${BBO_FILE} ${ELF_FILE} ${MAP_FILE} ${GENSYMS_FILE} ${LOG_FILE}
                COMMAND ${COMPILER_SCRIPT} -b ${bzz_source} -B ${ARGN} ${c_source}
                DEPENDS ${bzz_source} ${c_source} ${ARGN} bbzkilobot bittybuzz bo2bbo kilo_bcodegen)
    else()
        add_custom_command(OUTPUT ${HEX_FILE} ${GEN_DIR}
                BYPRODUCTS ${BASM_FILE} ${BO_FILE} ${BDB_FILE} ${BBO_FILE} ${ELF_FILE} ${MAP_FILE} ${GENSYMS_FILE} ${LOG_FILE}
                COMMAND ${COMPILER_SCRIPT} -b ${bzz_source} ${c_source}
                DEPENDS ${bzz_source} ${c_source} bbzkilobot bittybuzz bo2bbo kilo_bcodegen)
    endif()

    add_custom_target(${BZZ_BASENAME} ALL DEPENDS ${HEX_FILE} ${COMPILER_SCRIPT})
    set_target_properties(${BZZ_BASENAME} PROPERTIES OUTPUT_NAME "${HEX_FILE}")
    add_dependencies(${BZZ_BASENAME} bbzkilobot bittybuzz bo2bbo kilo_bcodegen)
endfunction()

# Generates all hex files.
function(generate_all_hex)
    file(GLOB BUZZ_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "buzz_scripts/*.bzz")
    file(GLOB BST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "buzz_scripts/*.bst")
    foreach(buzz_source ${BUZZ_SOURCES})
        get_filename_component(basename ${buzz_source} NAME_WE)
        if (buzz_scripts/${basename}.bst IN_LIST BST_SOURCES)
            generate_hex(${CMAKE_CURRENT_SOURCE_DIR}/${basename}.c ${CMAKE_CURRENT_SOURCE_DIR}/buzz_scripts/${basename}.bzz ${CMAKE_CURRENT_SOURCE_DIR}/buzz_scripts/${basename}.bst)
        else()
            generate_hex(${CMAKE_CURRENT_SOURCE_DIR}/${basename}.c ${CMAKE_CURRENT_SOURCE_DIR}/buzz_scripts/${basename}.bzz)
        endif()
    endforeach()
endfunction()


# ==========================================
# =              CMAKE SCRIPT              =
# ==========================================

set(COMPILER_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/compile.sh)
configure_file(compile.sh ${COMPILER_SCRIPT} @ONLY)

generate_all_hex()