include(CMakeForceCompiler)

#
# Look for the necessary tools
#
set(CMD_PREFIX arm-none-eabi)
find_program(CC ${CMD_PREFIX}-gcc)
find_program(AR ${CMD_PREFIX}-ar)
find_program(OBJCOPY ${CMD_PREFIX}-objcopy)
find_program(OBJDUMP ${CMD_PREFIX}-objdump)
find_program(SIZE ${CMD_PREFIX}-size)
find_program(GDB ${CMD_PREFIX}-gdb)
find_program(UPLOADER st-flash)

#
# Toolchain variables
#
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
CMAKE_FORCE_C_COMPILER(${CC} GNU)
CMAKE_FORCE_CXX_COMPILER(${CC} GNU)

set(ZOOIDS_DRIVER_DIR "${CMAKE_SOURCE_DIR}/zooids/drivers")
set(ZOOIDS_LIB_DIR "${CMAKE_SOURCE_DIR}/zooids/lib")

set(BSP_DIR "${ZOOIDS_DRIVER_DIR}/BSP/STM32091C_EVAL/")
set(HAL_DIR "${ZOOIDS_DRIVER_DIR}/STM32F0xx_HAL_Driver")
set(INCLUDE_DIR "-I${ZOOIDS_LIB_DIR}/incL -I${ZOOIDS_LIB_DIR}/inc -I${ZOOIDS_DRIVER_DIR}/CMSIS/Include -I${ZOOIDS_DRIVER_DIR}/CMSIS/Device/ST/STM32F0xx/Include -I${ZOOIDS_DRIVER_DIR}/STM32F0xx_HAL_Driver/Inc -I${ZOOIDS_DRIVER_DIR}/BSP/STM32072B_EVAL -I${ZOOIDS_DRIVER_DIR}/BSP/Components/Common -I${CMAKE_SOURCE_DIR} -I${CMAKE_BINARY_DIR} -I${CMAKE_BINARY_DIR}/zooids/lib/inc -I${ZOOIDS_DRIVER_DIR}/qfplib")
set(ZOOIDS_SOURCES stm32f0xx_hal.c stm32f0xx_hal_adc.c stm32f0xx_hal_adc_ex.c stm32f0xx_hal_can.c stm32f0xx_hal_cec.c stm32f0xx_hal_comp.c stm32f0xx_hal_cortex.c stm32f0xx_hal_crc.c stm32f0xx_hal_crc_ex.c stm32f0xx_hal_dac.c stm32f0xx_hal_dac_ex.c stm32f0xx_hal_dma.c stm32f0xx_hal_flash.c stm32f0xx_hal_flash_ex.c stm32f0xx_hal_gpio.c stm32f0xx_hal_i2c.c stm32f0xx_hal_i2c_ex.c stm32f0xx_hal_i2s.c stm32f0xx_hal_irda.c stm32f0xx_hal_iwdg.c stm32f0xx_hal_pcd.c stm32f0xx_hal_pcd_ex.c stm32f0xx_hal_pwr.c stm32f0xx_hal_pwr_ex.c stm32f0xx_hal_rcc.c stm32f0xx_hal_rcc_ex.c stm32f0xx_hal_rtc.c stm32f0xx_hal_rtc_ex.c stm32f0xx_hal_smartcard.c stm32f0xx_hal_smartcard_ex.c stm32f0xx_hal_smbus.c stm32f0xx_hal_spi.c stm32f0xx_hal_spi_ex.c stm32f0xx_hal_tim.c stm32f0xx_hal_tim_ex.c stm32f0xx_hal_tsc.c stm32f0xx_hal_uart.c stm32f0xx_hal_uart_ex.c stm32f0xx_hal_usart.c stm32f0xx_hal_wwdg.c)
set(ZOOIDS_HEADERS ${ZOOIDS_LIB_DIR}/inc/stm32f0xx_hal_conf.h ${ZOOIDS_LIB_DIR}/inc/stm32f0xx_conf.h ${ZOOIDS_LIB_DIR}/inc/stm32f0xx_it.h ${ZOOIDS_LIB_DIR}/inc/utilities.h ${ZOOIDS_LIB_DIR}/inc/qt1070.h ${ZOOIDS_LIB_DIR}/inc/config.h ${ZOOIDS_LIB_DIR}/inc/functions.h ${ZOOIDS_LIB_DIR}/inc/lsm6ds3.h ${ZOOIDS_LIB_DIR}/inc/RF24.h ${ZOOIDS_LIB_DIR}/inc/nRF24L01.h ${ZOOIDS_LIB_DIR}/inc/stm32f0xx_lp_modes.h ${ZOOIDS_DRIVER_DIR}/CMSIS/Device/ST/STM32F0xx/Include/stm32f051x8.h ${ZOOIDS_LIB_DIR}/inc/MadgwickAHRS.h ${ZOOIDS_LIB_DIR}/inc/RingBuffer.h ${ZOOIDS_LIB_DIR}/inc/colors.h ${ZOOIDS_LIB_DIR}/inc/led.h ${ZOOIDS_LIB_DIR}/inc/motors.h ${ZOOIDS_LIB_DIR}/inc/radio.h ${ZOOIDS_LIB_DIR}/inc/sensors.h ${ZOOIDS_LIB_DIR}/inc/position_control.h)

set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/zooids/lib/linker/stm32f072.ld")
set(LIBS "-L${ZOOIDS_DRIVER_DIR}/CMSIS/Lib")

set(DEFS "-DUSE_HAL_DRIVER -DSTM32F051x8 -DSTARTUP_FROM_RESET -DUSE_SPI_CRC=0")

set(CFLAGS "-Os -std=gnu99 -g3 -mthumb -mcpu=cortex-m0 -msoft-float")
#set(CFLAGS "${CFLAGS} -Wextra -Wshadow -Wimplicit-function-declaration")
#set(CFLAGS "${CFLAGS} -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes")
set(CFLAGS "${CFLAGS} -fno-common -ffunction-sections -fdata-sections")
set(CFLAGS "${CFLAGS} -MD -Wall -Wundef -Wno-comment -Wno-unused-variable -Wno-unused-function")
set(CFLAGS "${CFLAGS} ${INCLUDE_DIR} ${DEFS}")
set(CFLAGS "${CFLAGS} -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -fno-strict-aliasing -fomit-frame-pointer -ffast-math")
# set(CFLAGS "${CFLAGS} -fno-builtin -fomit-frame-pointer -mabi=aapcs -fno-unroll-loops -ffast-math -ftree-vectorize")

set(LDFLAGS "--static -nostartfiles -mthumb -g3 -mcpu=cortex-m0 -msoft-float -T${LINKER_SCRIPT}")
set(LDSCRIPT "")
set(LDFLAGS "${LDFLAGS} -Wl,--gc-sections ${LIBS} ${ZOOIDS_LIB_DIR}/linker/startup_stm32f0xx.S ${ZOOIDS_DRIVER_DIR}/qfplib/qfplib.s")
# LDLIBS
set(LDLIBS "-Wl,--start-group -lm -Wl,--end-group")

set(VPATH "${ZOOIDS_LIB_DIR}/src")
set(VPATH "${VPATH} ${ZOOIDS_LIB_DIR}/srcL")
set(VPATH "${VPATH} ${BSP_DIR}")
set(VPATH "${VPATH} ${HAL_DIR}/Src")

#
# Variable caching (for compiler swap functions)
#
set(NATIVE_SYSTEM_NAME ${CMAKE_SYSTEM_NAME} CACHE STRING
    "Name of the system we are programming for.")
set(NATIVE_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR} CACHE STRING
    "Processor type for the system we are programming for.")
set(NATIVE_C_COMPILER ${CMAKE_C_COMPILER} CACHE STRING
    "Path to the compiler for the system we are programing for." FORCE)

set(NATIVE_C_FLAGS "${CFLAGS} -DRID=$(RESULT)" CACHE STRING "GCC flags for the native compiler." FORCE)
set(CURRENT_COMPILER "NATIVE" CACHE STRING "Which compiler we are using.")

#
# BittyBuzz variables
#
set(BBZ_ROBOT zooids)
option(BBZ_XTREME_MEMORY "Whether to enable high memory-optimization." OFF)
option(BBZ_BYTEWISE_ASSIGNMENT "Wether to make assignment byte per byte." ON)
set(BBZHEAP_SIZE 4096)
set(BBZSTACK_SIZE 256)
# message("BBZHEAP_SIZE := ${BBZHEAP_SIZE}")
set(BBZHEAP_GCMARK_DEPTH 15)

set(BBZ_DISABLE_NEIGHBORS ON)
set(BBZ_DISABLE_VSTIGS OFF)
set(BBZ_DISABLE_SWARMS ON)

#
# CMake command to compile an executable
#
function(zooids_add_executable _TARGET)
    # Target names
    set(_ELF_TARGET ${_TARGET}-${BBZ_ROBOT}.elf)
    set(_LSS_TARGET ${_TARGET}-${BBZ_ROBOT}.lss)
    set(_BIN_TARGET ${_TARGET}-${BBZ_ROBOT}.bin)
    set(_HEX_TARGET ${_TARGET}-${BBZ_ROBOT}.hex)
    set(_MAP_TARGET ${_TARGET}-${BBZ_ROBOT}.map)
    # .elf -> .lss
    add_custom_command(OUTPUT ${_LSS_TARGET}
        COMMAND ${OBJDUMP} -d -S ${_ELF_TARGET} > ${_LSS_TARGET}
        DEPENDS ${_ELF_TARGET})
    # .elf -> .hex
    add_custom_command(OUTPUT ${_HEX_TARGET}
        COMMAND ${OBJCOPY} -Oihex ${_ELF_TARGET} ${_HEX_TARGET}
        COMMAND ${SIZE} ${_HEX_TARGET}
        DEPENDS ${_ELF_TARGET})
    # .elf -> .bin
    add_custom_command(OUTPUT ${_BIN_TARGET}
        COMMAND ${OBJCOPY} -Obinary ${_ELF_TARGET} ${_BIN_TARGET}
        DEPENDS ${_ELF_TARGET})
    # Compile .elf file
    add_executable(${_ELF_TARGET} EXCLUDE_FROM_ALL ${ARGN})
    set_target_properties(${_ELF_TARGET}
        PROPERTIES
        COMPILE_FLAGS "${CFLAGS} -DRID=$(RESULT)"
        LINK_FLAGS "${LDSCRIPT} ${LDFLAGS} -Wl,-Map=${_MAP_TARGET} ${LDLIBS}")
    add_custom_command(TARGET ${_ELF_TARGET} POST_BUILD
        COMMAND ${SIZE} ${_ELF_TARGET}
        COMMENT "Calculating ELF file size:")

    # Make target
    add_custom_target(${_TARGET} DEPENDS ${_LSS_TARGET} ${_HEX_TARGET} ${_BIN_TARGET})
    set_target_properties(${_TARGET} PROPERTIES OUTPUT_NAME "${_ELF_TARGET}")

    # Uploading file
    add_custom_target(upload_${_TARGET}
        LD_LIBRARY_PATH=/usr/local/lib/ ${UPLOADER} erase && LD_LIBRARY_PATH=/usr/local/lib/ ${UPLOADER} --reset --format ihex write ${_HEX_TARGET}
        DEPENDS ${_HEX_TARGET}
        COMMENT "Uploading ${_HEX_TARGET} to zooids")

    # Extra files to clean
    get_directory_property(_CLEAN_FILES ADDITIONAL_MAKE_CLEAN_FILES)
    set_directory_properties(
        PROPERTIES
        ADDITIONAL_MAKE_CLEAN_FILES "${_CLEAN_FILES};${_MAP_TARGET}")
endfunction(zooids_add_executable _TARGET)

#
# CMake command to compile a library
#
function(zooids_add_library _TARGET)
    # Set library file name
    set(_LIB_TARGET ${_TARGET}-${BBZ_ROBOT})
    # Add library target
    add_library(${_TARGET} STATIC ${ARGN})
    add_library(${_TARGET}_objects OBJECT ${ARGN})
    set_target_properties(${_TARGET} PROPERTIES
        COMPILE_FLAGS "${CFLAGS} -DRID=$(RESULT)"
        LINK_FLAGS ${LDFLAGS}
        OUTPUT_NAME ${_LIB_TARGET})
    set_target_properties(${_TARGET}_objects PROPERTIES
        COMPILE_FLAGS "${CFLAGS} -DRID=$(RESULT)"
        LINK_FLAGS ${LDFLAGS}
        OUTPUT_NAME ${_LIB_TARGET})
endfunction(zooids_add_library _TARGET)

#
# CMake command to link a target to a library
#
function(zooids_target_link_libraries _TARGET)
    set(_ELF_TARGET ${_TARGET}-${BBZ_ROBOT}.elf)
    # Put as first in the target list the name of the target known by CMake
    get_target_property(_TARGET_LIST ${_TARGET} OUTPUT_NAME)
    # Go through the arguments, add them as dependencies and make a
    # list of CMake targets and non-CMake targets.
    foreach(_T ${ARGN})
        add_dependencies(${_TARGET} ${_T})
        add_dependencies(${_ELF_TARGET} ${_T})
        if(TARGET ${_T})
            get_target_property(_P ${_T} OUTPUT_NAME)
            list(APPEND _TARGET_LIST ${_P})
        else(TARGET ${_T})
            list(APPEND _NON_TARGET_LIST ${_T})
        endif(TARGET ${_T})
    endforeach(_T ${ARGN})
    # Declare the target list
    target_link_libraries(${_TARGET_LIST} ${_NON_TARGET_LIST})
endfunction(zooids_target_link_libraries _TARGET)