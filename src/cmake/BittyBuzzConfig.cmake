function (config_value value default)
    if("${${value}}" STREQUAL "")
        message(STATUS "Using default value for ${value} (-D${value}=${default})")
        set(${value} ${default} PARENT_SCOPE)
    endif()
endfunction()

config_value(BBZHEAP_SIZE 1024)
config_value(BBZHEAP_ELEMS_PER_TSEG 5)
config_value(BBZSTACK_SIZE 256)
config_value(BBZVSTIG_CAP 3)
config_value(BBZNEIGHBORS_CAP 4)
config_value(RESERVED_ACTREC_MAX 32)

# Set the XTREME memory optimization to false if it hasn't been set yet.
option(BBZ_XTREME_MEMORY "Whether to enable high memory-optimization." OFF)