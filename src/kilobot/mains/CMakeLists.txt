include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
link_directories(${CMAKE_CURRENT_BINARY_DIR})

configure_file(compile.sh compile.sh @ONLY)
execute_process(COMMAND chmod +x ${CMAKE_CURRENT_BINARY_DIR}/compile.sh)

set(COMPILER_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/compile.sh)

# Generates a BittyBuzz object file.
# bst_source can be left empty in order not to use any Buzz Symbols Table.
function(generate_hex c_source bzz_source)
    get_filename_component(BZZ_BASENAME ${bzz_source} NAME_WE)
    set(GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/gen")
    set(BZZ_BASEPATH "${GEN_DIR}/${BZZ_BASENAME}")
    set(GENSYMS_FILE "${GEN_DIR}/bbzkilosymgen.h")

    set(BASM_FILE ${BZZ_BASEPATH}.basm)
    set(BO_FILE   ${BZZ_BASEPATH}.bo)
    set(BDB_FILE  ${BZZ_BASEPATH}.bdb)
    set(BBO_FILE  ${BZZ_BASEPATH}.bbo)
    set(ELF_FILE  ${BZZ_BASEPATH}.elf)
    set(HEX_FILE  ${BZZ_BASEPATH}.hex)
    set(MAP_FILE  ${BZZ_BASEPATH}.map)

    # .bzz -> .basm, ...
    if (NOT ${ARGN} STREQUAL "")
        add_custom_command(OUTPUT ${BASM_FILE} ${BO_FILE} ${BDB_FILE} ${BBO_FILE} ${ELF_FILE} ${HEX_FILE} ${MAP_FILE} ${GENSYMS_FILE}
                COMMAND ${COMPILER_SCRIPT} -b ${bzz_source} -B ${ARGN} ${c_source})
    else()
        add_custom_command(OUTPUT ${BASM_FILE} ${BO_FILE} ${BDB_FILE} ${BBO_FILE} ${ELF_FILE} ${HEX_FILE} ${MAP_FILE} ${GENSYMS_FILE}
                COMMAND ${COMPILER_SCRIPT} -b ${bzz_source} ${c_source})
    endif()

    add_custom_target(${BZZ_BASENAME} DEPENDS ${HEX_FILE} ${COMPILER_SCRIPT})
    set_target_properties(${BZZ_BASENAME} PROPERTIES OUTPUT_NAME "${HEX_FILE}")
endfunction()

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

generate_all_hex()