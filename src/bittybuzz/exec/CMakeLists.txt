file(GLOB BBZ_SOURCES "*.c")

if(NOT CMAKE_CROSSCOMPILING)
    foreach (bbz_exec_src ${BBZ_SOURCES})
        get_filename_component(bbz_excutable ${bbz_exec_src} NAME_WE)
        add_executable(${bbz_excutable} ${bbz_exec_src})
        target_link_libraries(${bbz_excutable} bittybuzz ${TESTING_EXTRA_LIBS})
    endforeach ()
endif(NOT CMAKE_CROSSCOMPILING)