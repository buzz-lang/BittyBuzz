# ==========================================
# =          FUNCTION DEFINITIONS          =
# ==========================================

# Generates a hex file given a C and a Buzz source.
# bst_source can be left empty in order not to use any Buzz Symbols Table.
function(generate_hex c_source bzz_source)
    get_filename_component(BZZ_BASENAME ${bzz_source} NAME_WE)
    set(GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/${BZZ_BASENAME}")
    make_directory(${GEN_DIR})
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
    set(DBG_FILE  ${BZZ_BASEPATH}.dbg)
    set(ASM_FILE  ${BZZ_BASEPATH}.lst)
    set(ELFDBG_FILE  ${BZZ_BASEPATH}.elfdbg)

    set(ELF_TARGET ${BZZ_BASENAME}-${BBZ_ROBOT}.elf)
    set(BIN_TARGET ${BZZ_BASENAME}-${BBZ_ROBOT}.bin)
    set(BIN_FILE  ${BZZ_BASEPATH}.bin)

    generate_bbo(${BZZ_BASENAME}_bbo ${GEN_DIR} ${bzz_source} ${ARGN})

    # Generate the symbols.h file
    add_custom_command(OUTPUT ${GENSYMS_FILE}
        COMMAND ./crazyflie_bcodegen ${BO_FILE} ${GENSYMS_FILE}
        DEPENDS crazyflie_bcodegen ${BO_FILE} ${BZZ_BASENAME}_bbo
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bittybuzz/exec)

    # We have to use 'bbzcrazyflie_objects' instead of the usual library file because of an issue with the linker that prevent the script from initializing
    add_executable(${ELF_TARGET} EXCLUDE_FROM_ALL ${c_source} ${GENSYMS_FILE} "$<TARGET_OBJECTS:bbzcrazyflie_objects>" )
    set_target_properties(${ELF_TARGET}
        PROPERTIES
        COMPILE_FLAGS "${CFLAGS} -DRID=$(RESULT)"
        LINK_FLAGS "${LDSCRIPT} ${LDFLAGS} -Wl,-Map=${MAP_FILE} ${LDLIBS}")

    # Display the Size of the generated ELF file
    add_custom_command(TARGET ${ELF_TARGET} POST_BUILD
        COMMAND ${SIZE} ${ELF_TARGET}
        COMMENT "Calculating ELF file size:")

    # Remove the generated symbols.h file afterward (necessary)
    add_custom_command(TARGET ${ELF_TARGET} POST_BUILD
        COMMAND rm ${GENSYMS_FILE}
        COMMENT "Removing Generated symbols")

    # Generate .lst file (the dissassembled code)
    add_custom_command(TARGET ${ELF_TARGET} POST_BUILD
        COMMAND ${OBJDUMP} -d -S ${ELF_TARGET} > ${ASM_FILE}
        DEPENDS ${ELF_TARGET}
        COMMENT "Generating text assembly file")
    # .elf -> .hex
    add_custom_command(TARGET ${ELF_TARGET} POST_BUILD
        BYPRODUCTS ${HEX_FILE}
        COMMAND ${OBJCOPY} -Oihex ${ELF_TARGET} ${HEX_FILE}
        COMMAND ${SIZE} ${HEX_FILE}
        COMMENT "Caclulating HEX file size:"
        DEPENDS ${ELF_TARGET})
    # .elf -> .bin (Generate a .bin file by WJ) 
    add_custom_command(TARGET ${ELF_TARGET} POST_BUILD
        BYPRODUCTS ${BIN_FILE}
        COMMAND ${OBJCOPY} -Obinary ${ELF_TARGET} ${BIN_FILE}
        COMMENT "Generating BIN file: \n  >> ${BIN_FILE}"
        DEPENDS ${ELF_TARGET})
    
    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${HEX_FILE};${ASM_FILE};${MAP_FILE};${BIN_FILE}")

    add_custom_target(${BZZ_BASENAME} ALL DEPENDS ${ELF_TARGET})
    set_target_properties(${BZZ_BASENAME} PROPERTIES OUTPUT_NAME "${ELF_TARGET}" INCLUDE_DIRECTORIES "${INCLUDE_DIRECTORIES}")
    add_dependencies(${BZZ_BASENAME} bbzcrazyflie_objects bittybuzz)
    include_directories(${BZZ_BASENAME} ${GEN_DIR})
    add_dependencies(behaviors ${BZZ_BASENAME})
    crazyflie_target_link_libraries(${BZZ_BASENAME} bittybuzz)

    add_custom_target(upload_${BZZ_BASENAME}
        LD_LIBRARY_PATH=/usr/local/lib/ ${UPLOADER} erase && LD_LIBRARY_PATH=/usr/local/lib/ ${UPLOADER} --reset --format ihex write ${HEX_FILE}
        DEPENDS ${HEX_FILE} ${ELF_TARGET} ${UPLOADER}
        COMMENT "Uploading ${HEX_FILE} to crazyflie")
endfunction()

# Generates all hex files.
function(generate_all_hex)
    file(GLOB BUZZ_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "buzz_scripts/*.bzz")
    file(GLOB BST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "buzz_scripts/*.bst")
    #include_directories(/usr/lib/avr/include)
    add_custom_target(behaviors ALL)
    foreach(buzz_source ${BUZZ_SOURCES})
        get_filename_component(basename ${buzz_source} NAME_WE)
        add_executable(${basename}_tmp EXCLUDE_FROM_ALL ${CMAKE_CURRENT_SOURCE_DIR}/${basename}.c)
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

generate_all_hex()
