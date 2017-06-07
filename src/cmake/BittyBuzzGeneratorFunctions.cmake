# Generates a BittyBuzz object file.
# bst_source can be left empty in order not to use any Buzz Symbols Table.
function(generate_bbo _TARGET bzz_outdir bzz_source bst_source)
  get_filename_component(BZZ_BASENAME ${bzz_source} NAME_WE)
  set(BZZ_BASEPATH "${bzz_outdir}/${BZZ_BASENAME}")

  set(BASM_FILE ${BZZ_BASEPATH}.basm)
  set(BO_FILE   ${BZZ_BASEPATH}.bo)
  set(BDB_FILE  ${BZZ_BASEPATH}.bdb)
  set(BBO_FILE  ${BZZ_BASEPATH}.bbo)

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