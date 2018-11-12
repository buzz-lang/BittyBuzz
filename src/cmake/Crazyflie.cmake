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
find_program(AS ${CMD_PREFIX}-as)
find_program(LD ${CMD_PREFIX}-gcc)
find_program(UPLOADER st-flash)

#
# Toolchain variables
#
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
CMAKE_FORCE_C_COMPILER(${CC} GNU)
CMAKE_FORCE_CXX_COMPILER(${CC} GNU)

set(LOAD_ADDRESS "0x8004000")

set(CRAZYFLIE_DRIVER_DIR "${CMAKE_SOURCE_DIR}/crazyflie/drivers")
set(CRAZYFLIE_LIB_DIR "${CMAKE_SOURCE_DIR}/crazyflie/lib")

#set(BSP_DIR "${CRAZYFLIE_DRIVER_DIR}/BSP/STM32091C_EVAL/")
#set(HAL_DIR "${CRAZYFLIE_DRIVER_DIR}/STM32F0xx_HAL_Driver")

set(INCLUDE_DIR "-I${CRAZYFLIE_LIB_DIR}/incL \
-I${CRAZYFLIE_LIB_DIR}/inc \
-I${CRAZYFLIE_LIB_DIR}/inc/cfdeck \
-I${CRAZYFLIE_LIB_DIR}/inc/cfdriver \
-I${CRAZYFLIE_LIB_DIR}/inc/cfmodules \
-I${CRAZYFLIE_LIB_DIR}/inc/cfhal \
-I${CRAZYFLIE_LIB_DIR}/inc/cfconfig \
-I${CRAZYFLIE_LIB_DIR}/inc/FreeRTOS \
-I${CRAZYFLIE_LIB_DIR}/inc/cfutils \
-I${CRAZYFLIE_LIB_DIR}/inc/cfplatform \
-I${CRAZYFLIE_DRIVER_DIR}/CMSIS/Include \
-I${CRAZYFLIE_DRIVER_DIR}/CMSIS/DSP_Lib \
-I${CRAZYFLIE_DRIVER_DIR}/CMSIS/Device/ST/STM32F4xx/Include \
-I${CRAZYFLIE_DRIVER_DIR}/STM32F4xx_StdPeriph_Driver/inc \
-I${CRAZYFLIE_LIB_DIR}/linker \
-I${CRAZYFLIE_DRIVER_DIR}/STM32_USB_OTG_Driver/inc \
-I${CRAZYFLIE_DRIVER_DIR}/STM32_USB_Device_Library/Core/inc \
-I${CRAZYFLIE_DRIVER_DIR}/vl53l1 \
-I${CRAZYFLIE_DRIVER_DIR}/vl53l1/core/inc \
-I${CRAZYFLIE_DRIVER_DIR}/FatFS \
-I${CRAZYFLIE_DRIVER_DIR}/libdw1000/inc \
-I${CRAZYFLIE_DRIVER_DIR}/BSP/STM32072B_EVAL \
-I${CRAZYFLIE_DRIVER_DIR}/BSP/Components/Common \
-I${CMAKE_SOURCE_DIR} \
-I${CMAKE_BINARY_DIR} \
-I${CMAKE_BINARY_DIR}/crazyflie/lib/inc \
-I${CRAZYFLIE_DRIVER_DIR}/qfplib")

set(CF_STDPERI_SOURCES stm32f4xx_gpio.c stm32f4xx_tim.c stm32f4xx_syscfg.c stm32f4xx_rcc.c stm32f4xx_adc.c stm32f4xx_can.c stm32f4xx_crc.c stm32f4xx_cryp.c stm32f4xx_cryp_aes.c stm32f4xx_cryp_des.c stm32f4xx_cryp_tdes.c stm32f4xx_dac.c stm32f4xx_dbgmcu.c stm32f4xx_dcmi.c stm32f4xx_dma.c stm32f4xx_dma2d.c stm32f4xx_exti.c stm32f4xx_flash.c stm32f4xx_fsmc.c stm32f4xx_hash.c stm32f4xx_hash_md5.c stm32f4xx_hash_sha1.c stm32f4xx_i2c.c stm32f4xx_iwdg.c stm32f4xx_ltdc.c stm32f4xx_misc.c stm32f4xx_pwr.c stm32f4xx_rng.c stm32f4xx_rtc.c stm32f4xx_sai.c stm32f4xx_sdio.c stm32f4xx_spi.c stm32f4xx_usart.c stm32f4xx_wwdg.c)

set(CFPLATFORM_SOURCES platform_cf2.c)

set(CFMODULES_SOURCES attitude_pid_controller.c comm.c commander.c console.c controller.c controller_mellinger.c controller_pid.c crtp.c crtp_commander.c crtp_commander_generic.c crtp_commander_high_level.c crtp_commander_rpyt.c crtp_localization_service.c crtpservice.c estimator.c estimator_complementary.c estimator_kalman.c extrx.c log.c mem_cf2.c msp.c outlierFilter.c param.c pid.c planner.c platformservice.c position_controller_pid.c position_estimator_altitude.c power_distribution_stock.c pptraj.c queuemonitor.c range.c sensfusion6.c sitaw.c sound_cf2.c stabilizer.c sysload.c system.c trigger.c worker.c)

set(CFDECK_SOURCES locodeck.c flowdeck_v1v2.c deck.c deck_drivers.c deck_info.c deck_test.c deck_analog.c deck_constants.c deck_digital.c deck_spi.c ledring12.c lpsTwrTag.c lpsTdoa2Tag.c lpsTdoa3Tag.c zranger.c zranger2.c)

set(CFDRIVER_SOURCES exti.c nvic.c motors.c diskio.c fatfs_sd.c led_f405.c ak8963.c cppm.c eeprom.c i2cdev_f405.c i2c_drv.c lps25h.c maxsonar.c mpu6500.c pca9685.c piezo.c pmw3901.c swd.c uart1.c uart2.c uart_syslink.c vl53l0x.c vl53l1x.c watchdog.c ws2812_cf2.c platform_info_stm32.c)

Set(CFUTILS_SOURCES abort.c cfassert.c clockCorrectionEngine.c configblockeeprom.c cpuid.c crc.c crc_bosch.c eprintf.c filter.c FreeRTOS-openocd.c num.c sleepus.c debug.c version.c tdoaEngine.c tdoaStats.c tdoaStorage.c)

set(CFFREERTOS_SOURCES croutine.c event_groups.c list.c queue.c tasks.c timers.c port.c heap_4.c)

set(CFHAL_SOURCES sensors_cf2.c buzzer.c freeRTOSdebug.c ledseq.c ow_syslink.c pca95x4.c pm_f405.c proximity.c radiolink.c syslink.c usb.c usb_bsp.c usbd_desc.c usblink.c usec_time.c pca9555.c)

set(STM32_SYS_SOURCE system_stm32f4xx.c)

set(STM32_USBDEVICELIB_SOURCE usbd_core.c usbd_ioreq.c usbd_req.c)

set(STM32_USBOTG_DRIVER_SOURCE usb_core.c usb_dcd.c usb_dcd_int.c)

set(VL53L1_SOURCE vl53l1_api.c vl53l1_api_calibration.c vl53l1_api_core.c vl53l1_api_debug.c vl53l1_api_preset_modes.c vl53l1_api_strings.c vl53l1_core.c vl53l1_core_support.c vl53l1_error_strings.c vl53l1_register_funcs.c vl53l1_silicon_core.c vl53l1_wait.c)

set(FATFS_SOURCES ff.c syscall.c unicode.c)

set(LIBDW1000_SOURCES libdw1000.c libdw1000Spi.c)

set(DSP_FILTERINGFUNCTIONS_SOURCES arm_biquad_cascade_df1_f32.c arm_biquad_cascade_df1_init_f32.c arm_biquad_cascade_df2T_f32.c arm_biquad_cascade_df2T_f64.c arm_biquad_cascade_df2T_init_f32.c arm_biquad_cascade_df2T_init_f64.c arm_biquad_cascade_stereo_df2T_f32.c arm_biquad_cascade_stereo_df2T_init_f32.c arm_conv_f32.c arm_conv_partial_f32.c arm_correlate_f32.c arm_fir_decimate_f32.c arm_fir_decimate_init_f32.c arm_fir_f32.c arm_fir_init_f32.c arm_fir_interpolate_f32.c arm_fir_interpolate_init_f32.c arm_fir_lattice_f32.c arm_fir_lattice_init_f32.c arm_fir_sparse_f32.c arm_fir_sparse_init_f32.c arm_iir_lattice_f32.c arm_iir_lattice_init_f32.c arm_lms_f32.c arm_lms_init_f32.c arm_lms_norm_f32.c arm_lms_norm_init_f32.c)

set(DSP_STATISTICFUNCTIONS_SOURCES arm_max_f32.c arm_min_f32.c arm_power_f32.c arm_rms_f32.c arm_var_f32.c arm_std_f32.c arm_mean_f32.c)

set(DSP_MATRIXFUNCTIONS_SOURCES arm_mat_add_f32.c arm_mat_cmplx_mult_f32.c arm_mat_init_f32.c arm_mat_inverse_f32.c arm_mat_mult_f32.c arm_mat_scale_f32.c arm_mat_sub_f32.c arm_mat_trans_f32.c)

set(DSP_FASTMATHFUNCTIONS_SOURCES arm_cos_f32.c arm_sin_f32.c)

set(DSP_CONTROLLERFUNCTIONS_SOURCES arm_pid_init_f32.c arm_pid_reset_f32.c arm_sin_cos_f32.c)

set(DSP_COMMONTABLES_SOURCES arm_common_tables.c arm_const_structs.c)

#set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/crazyflie/lib/linker/FLASH_CLOAD.ld")
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/crazyflie/lib/linker/stm32f4cf.ld")
set(LIBS "-L${CRAZYFLIE_DRIVER_DIR}/CMSIS/Lib")
set(REV "D")
set(ESTIMATOR "any")
set(CONTROLLER "Any") # one of Any, PID, Mellinger
set(POWER_DISTRIBUTION "stock")
set(CLOAD_SCRIPT "python3 -m cfloader")

#set(DEFS "-DUSE_HAL_DRIVER -DSTM32F051x8 -DSTARTUP_FROM_RESET -DUSE_SPI_CRC=0")
set(DEFS "-DARM_MATH_CM4 -D__FPU_PRESENT=1 -D__TARGET_FPU_VFP -DSTM32F4XX -DSTM32F40_41xxx -DHSE_VALUE=8000000 -DUSE_STDPERIPH_DRIVER -DUSE_RADIOLINK_CRTP -DENABLE_UART -DBOARD_REV_${REV} -DESTIMATOR_NAME=${ESTIMATOR}Estimator -DCONTROLLER_NAME=ControllerType${CONTROLLER} -DPOWER_DISTRIBUTION_TYPE_${POWER_DISTRIBUTION}")

set(CFLAGS "-Os -std=gnu11 -g3 -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16")
#originally: uses -std=gnu99
#set(CFLAGS "${CFLAGS} -Wextra -Wshadow -Wimplicit-function-declaration")
#set(CFLAGS "${CFLAGS} -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes")
set(CFLAGS "${CFLAGS} -fno-common -ffunction-sections -fdata-sections -fno-math-errno")
set(CFLAGS "${CFLAGS} -MD -MP -Wmissing-braces -Wall -Wundef -Wno-comment -Wno-unused-variable -Wno-unused-function -Wdouble-promotion")
set(CFLAGS "${CFLAGS} ${INCLUDE_DIR} ${DEFS}")
set(CFLAGS "${CFLAGS} -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -fno-strict-aliasing -fomit-frame-pointer -ffast-math")
# set(CFLAGS "${CFLAGS} -fno-builtin -fomit-frame-pointer -mabi=aapcs -fno-unroll-loops -ffast-math -ftree-vectorize")

set(LDFLAGS "--static -nostartfiles -mthumb -g3 -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -T${LINKER_SCRIPT}")
set(LDSCRIPT "")
#set(LDFLAGS "${LDFLAGS} -Wl,--gc-sections ${LIBS} ${CRAZYFLIE_LIB_DIR}/linker/startup_stm32f0xx.S ${CRAZYFLIE_DRIVER_DIR}/qfplib/qfplib.s")
set(LDFLAGS "--specs=rdimon.specs --specs=nosys.specs --specs=nano.specs ${LDFLAGS} -Wl,--gc-sections ${LIBS} ${CRAZYFLIE_DRIVER_DIR}/CMSIS/Device/ST/STM32F4xx/Source/startup_stm32f40xx.s")

# LDLIBS
set(LDLIBS "-Wl,--start-group -lm -Wl,--end-group")

set(VPATH "${CRAZYFLIE_LIB_DIR}/src")
set(VPATH "${VPATH} ${CRAZYFLIE_LIB_DIR}/srcL")
set(VPATH "${VPATH} ${CRAZYFLIE_DRIVER_DIR}/STM32F4xx_StdPeriph_Driver/src")
set(VPATH "${VPATH} ${CRAZYFLIE_DRIVER_DIR}/STM32_USB_Device_Library/Core/src")
set(VPATH "${VPATH} ${CRAZYFLIE_DRIVER_DIR}/STM32_USB_OTG_Driver/src")
set(VPATH "${VPATH} ${CRAZYFLIE_DRIVER_DIR}/vl53l1/core/src")
set(VPATH "${VPATH} ${CRAZYFLIE_DRIVER_DIR}/CMSIS/DSP_Lib/Source/FilteringFunctions")
set(VPATH "${VPATH} ${CRAZYFLIE_DRIVER_DIR}/CMSIS/DSP_Lib/Source/BasicMathFunctions")
set(VPATH "${VPATH} ${CRAZYFLIE_DRIVER_DIR}/CMSIS/DSP_Lib/Source/CommonTables")
set(VPATH "${VPATH} ${CRAZYFLIE_DRIVER_DIR}/CMSIS/DSP_Lib/Source/ComplexMathFunctions")
set(VPATH "${VPATH} ${CRAZYFLIE_DRIVER_DIR}/CMSIS/DSP_Lib/Source/ControllerFunctions")
set(VPATH "${VPATH} ${CRAZYFLIE_DRIVER_DIR}/CMSIS/DSP_Lib/Source/FastMathFunctions")
set(VPATH "${VPATH} ${CRAZYFLIE_DRIVER_DIR}/CMSIS/DSP_Lib/Source/MatrixFunctions")
set(VPATH "${VPATH} ${CRAZYFLIE_DRIVER_DIR}/CMSIS/DSP_Lib/Source/StatisticsFunctions")
set(VPATH "${VPATH} ${CRAZYFLIE_DRIVER_DIR}/CMSIS/DSP_Lib/Source/SupportFunctions")
set(VPATH "${VPATH} ${CRAZYFLIE_DRIVER_DIR}/CMSIS/DSP_Lib/Source/TransformFunctions")
#set(VPATH "${VPATH} ${BSP_DIR}")
#set(VPATH "${VPATH} ${HAL_DIR}/Src")

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
set(BBZ_ROBOT crazyflie)
option(BBZ_XTREME_MEMORY "Whether to enable high memory-optimization." OFF)
option(BBZ_BYTEWISE_ASSIGNMENT "Wether to make assignment byte per byte or directly. (used to ensure compatibility with Cortex-M0)" ON)
set(BBZHEAP_SIZE 2048)
set(BBZSTACK_SIZE 128)
# message("BBZHEAP_SIZE := ${BBZHEAP_SIZE}")
set(BBZHEAP_GCMARK_DEPTH 16)
set(BBZNEIGHBORS_CAP 10)
set(BBZMSG_IN_PROC_MAX 10)

# set(BBZ_DISABLE_NEIGHBORS ON)
# set(BBZ_DISABLE_VSTIGS ON)
# set(BBZ_DISABLE_SWARMS ON)

set(local_revision 0)
set(revision 0)
set(tag 2018)
set(branch master)
set(modified false)

configure_file(${CRAZYFLIE_LIB_DIR}/src/cfutils/version.vtpl ${CRAZYFLIE_LIB_DIR}/src/cfutils/version.c @ONLY)

#
# CMake command to compile an executable
#
function(crazyflie_add_executable _TARGET)
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
        COMMENT "Uploading ${_HEX_TARGET} to crazyflie")

    # Extra files to clean
    get_directory_property(_CLEAN_FILES ADDITIONAL_MAKE_CLEAN_FILES)
    set_directory_properties(
        PROPERTIES
        ADDITIONAL_MAKE_CLEAN_FILES "${_CLEAN_FILES};${_MAP_TARGET}")
endfunction(crazyflie_add_executable _TARGET)

#
# CMake command to compile a library
#
function(crazyflie_add_library _TARGET)
    # Set library file name
    set(_LIB_TARGET ${_TARGET}-${BBZ_ROBOT})
    # Add library target
    add_library(${_TARGET} STATIC ${ARGN})
    # This line is necessary to compile Buzz scripts correctly, or the script won't even initialize.
    add_library(${_TARGET}_objects OBJECT ${ARGN})
    set_target_properties(${_TARGET} PROPERTIES
        COMPILE_FLAGS "${CFLAGS} -DRID=$(RESULT)"
        LINK_FLAGS ${LDFLAGS}
        OUTPUT_NAME ${_LIB_TARGET})
    set_target_properties(${_TARGET}_objects PROPERTIES
        COMPILE_FLAGS "${CFLAGS} -DRID=$(RESULT)"
        LINK_FLAGS ${LDFLAGS}
        OUTPUT_NAME ${_LIB_TARGET})
endfunction(crazyflie_add_library _TARGET)

#
# CMake command to link a target to a library
#
function(crazyflie_target_link_libraries _TARGET)
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
endfunction(crazyflie_target_link_libraries _TARGET)
