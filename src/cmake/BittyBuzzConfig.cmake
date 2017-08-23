function (config_value value default)
    if("${${value}}" STREQUAL "")
        message(STATUS "Using default value for ${value} (-D${value}=${default})")
        set(${value} ${default} PARENT_SCOPE)
    endif()
endfunction()

if (CMAKE_CROSSCOMPILING)
    config_value(BBZHEAP_SIZE 1088)
else()
    config_value(BBZHEAP_SIZE 3264)
endif ()
config_value(BBZHEAP_ELEMS_PER_TSEG 5)
config_value(BBZSTACK_SIZE 96)
config_value(BBZVSTIG_CAP 3)
config_value(BBZNEIGHBORS_CAP 15)
config_value(BBZINMSG_QUEUE_CAP 10)
config_value(BBZOUTMSG_QUEUE_CAP 10)
config_value(BBZHEAP_RSV_ACTREC_MAX 28)
config_value(BBZLAMPORT_THRESHOLD 50)
config_value(BBZHEAP_GCMARK_DEPTH 8)
config_value(BBZMSG_IN_PROC_MAX 10)
config_value(BBZNEIGHBORS_CLR_PERIOD 10)
config_value(BBZNEIGHBORS_MARK_TIME 4)

# Set the XTREME memory optimization to false if it hasn't been set yet.
option(BBZ_XTREME_MEMORY "Whether to enable high memory-optimization." OFF)
option(BBZ_USE_PRIORITY_SORT "Whether to use priority sort on out-messages queue." OFF)
option(BBZ_USE_FLOAT "Whether to use float type." OFF)
option(BBZ_DISABLE_NEIGHBORS "Whether to disable usage of neighbors' data structure and messages." ON)
option(BBZ_DISABLE_VSTIGS "Whether to disable usage of virtual stigmergies' data structure and messages." ON)
option(BBZ_DISABLE_SWARMS "Whether to disable usage of swarms' data structure and messages." OFF) # TODO Swarms take too much RAM because there are so many closure activation records
option(BBZ_DISABLE_MESSAGES "Whether to disable usage and transfer of any kind of Buzz message." OFF)

# TODO Currently, there is no implementation of swarmlist broadcasts because
# neighbors.kin and neighbors.nonkin, which are the only closures that would
# make use of it, are not implemented.
option(BBZ_DISABLE_SWARMLIST_BROADCASTS "Whether we disable the broadcasting of our swarmlist to neighboring robots." ON)
