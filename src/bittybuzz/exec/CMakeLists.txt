use_host_compiler()

# Add executables
set(BBZ_SOURCES
        bo2bbo.c
        kilo_bcodegen.c
        zooids_bcodegen.c
        crazyflie_bcodegen.c
)
foreach (bbz_exec_src ${BBZ_SOURCES})
    get_filename_component(bbz_excutable ${bbz_exec_src} NAME_WE)
    add_executable(${bbz_excutable} ${bbz_exec_src} ../bbzfloat.c)
endforeach ()
