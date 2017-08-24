
function(buzz_find_program varOut prgm)
    set(path ${varOut}_PATH)
    find_program(${path} ${prgm})
    if ("${${path}}" STREQUAL "${path}-NOTFOUND")
        message(SEND_ERROR "Buzz program \"${prgm}\" could not be found. "
                "Compile and install instructions can be found at "
                "https://github.com/MISTLab/Buzz")
        set(${varOut} "${varOut}-NOTFOUND" CACHE STRING "Path to a Buzz program.")
    else()
        set(${varOut} "${${path}}" CACHE STRING "Path to a Buzz program.")
    endif()
    unset(${path} CACHE)
endfunction()

buzz_find_program(BZZC   bzzc)
buzz_find_program(BZZPAR bzzparse)
buzz_find_program(BZZASM bzzasm)
buzz_find_program(BZZRUN bzzrun)