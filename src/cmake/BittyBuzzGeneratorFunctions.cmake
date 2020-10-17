find_program(CAT "cat")
find_program(PRINTF "printf")

# Generates a BittyBuzz object file. (produce a target that will generate the .bbo when needed)
# bst_source is optional. You may specify a nonexistent file (such as the
# empty string "") in order not to use any BST file.
function(generate_bbo _TARGET bzz_outdir bzz_source bst_source)
    get_filename_component(BZZ_BASENAME ${bzz_source} NAME_WE)
    set(BZZ_BASEPATH "${bzz_outdir}/${BZZ_BASENAME}")

    set(BST_FILE  ${BZZ_BASEPATH}.bst)
    set(BASM_FILE ${BZZ_BASEPATH}.basm)
    set(BO_FILE   ${BZZ_BASEPATH}.bo)
    set(BDB_FILE  ${BZZ_BASEPATH}.bdb)
    set(BBO_FILE  ${BZZ_BASEPATH}.bbo)

    # .bzz -> .basm
    # file(READ   "${BBZ_BASE_BST_FILE}" BBZ_BASE_BST)
    # file(READ   "${bst_source}" USER_BST)
    # file(WRITE  "${BST_FILE}" "${BBZ_BASE_BST}")
    # file(APPEND "${BST_FILE}" "${USER_BST}")
    add_custom_command(OUTPUT ${BST_FILE}
        COMMAND ${PRINTF} "\"\"" | ${CAT} ${BBZ_BASE_BST_FILE} > ${BST_FILE}
        COMMAND ${PRINTF} "\"\"" | ${CAT} ${bst_source} >> ${BST_FILE}
        DEPENDS ${PRINTF} ${CAT} ${BBZ_BASE_BST_FILE} ${bst_source})

    add_custom_command(OUTPUT ${BASM_FILE}
        COMMAND ${BZZPAR} ${bzz_source} ${BASM_FILE} ${BST_FILE}
        DEPENDS ${BZZPAR} ${bzz_source} ${BST_FILE})

    # .basm -> .bo ; .basm -> .bdb
    add_custom_command(OUTPUT ${BO_FILE} ${BDB_FILE}
            COMMAND ${BZZASM} ${BASM_FILE} ${BO_FILE} ${BDB_FILE}
            DEPENDS ${BZZASM} ${BASM_FILE})

    # .bo -> .bbo
    add_custom_command(OUTPUT ${BBO_FILE}
            COMMAND "$<TARGET_FILE:bo2bbo>" ${BO_FILE} ${BBO_FILE}
            DEPENDS ${BO_FILE})

    # Add the main target
    add_custom_target(${_TARGET} DEPENDS ${BBO_FILE} "$<TARGET_FILE:bo2bbo>")
endfunction()


# Generates a Buzz object file.
# bst_source can be left empty in order not to use any Buzz Symbols Table.
function(generate_bo _TARGET bzz_outdir bzz_source bst_source)
    get_filename_component(BZZ_BASENAME ${bzz_source} NAME_WE)
    set(BZZ_BASEPATH "${bzz_outdir}/${BZZ_BASENAME}")

    set(BASM_FILE ${BZZ_BASEPATH}.basm)
    set(BO_FILE   ${BZZ_BASEPATH}.bo)
    set(BDB_FILE  ${BZZ_BASEPATH}.bdb)

    # .bzz -> .basm
    if (NOT ${bst_source} STREQUAL "")
        add_custom_command(OUTPUT ${BASM_FILE}
            COMMAND ${BZZPAR} ${bzz_source} ${BASM_FILE} ${bst_source})
    else()
        add_custom_command(OUTPUT ${BASM_FILE}
            COMMAND ${BZZPAR} ${bzz_source} ${BASM_FILE})
    endif()

    # .basm -> .bo ; .basm -> .bdb
    add_custom_command(OUTPUT ${BO_FILE} ${BDB_FILE}
            COMMAND ${BZZASM} ${BASM_FILE} ${BO_FILE} ${BDB_FILE}
            DEPENDS ${BASM_FILE})

    # Add the main target
    add_custom_target(${_TARGET} DEPENDS ${BO_FILE} "$<TARGET_FILE:bo2bbo>")
endfunction()