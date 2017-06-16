use_host_compiler()

# Add executables
file(GLOB BBZ_SOURCES "*.c")
foreach (bbz_exec_src ${BBZ_SOURCES})
    get_filename_component(bbz_excutable ${bbz_exec_src} NAME_WE)
    add_executable(${bbz_excutable} ${bbz_exec_src} ../bbzfloat.c)
endforeach ()
