# ==========================================
# =          FUNCTION DEFINITIONS          =
# ==========================================

function(add_bcode_generator)
    file(GLOB BYTECODE_GEN_SOURCES "*.c")
    add_executable(bcodegen ${BYTECODE_GEN_SOURCES})
endfunction()

# ==========================================
# =              CMAKE SCRIPT              =
# ==========================================

use_host_compiler()
add_bcode_generator()