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

set(LOAD_ADDRESS_CLOAD "0x8004000")
set(PLATFORM_NAME_cf2 CF2 platform)

#
# Version of the crazyflie
#
set(local_revision 26)
set(revision 43b6c25db2ee)
set(tag 2018.10-rc1-26)
set(branch master)
set(modified false)
set(irevision0 0x43b6c25d)
set(irevision1 0xb2ee)
set(robot_id $(RESULT))

set(CRAZYFLIE_DRIVER_DIR "${CMAKE_SOURCE_DIR}/crazyflie/drivers")
set(CRAZYFLIE_LIB_DIR "${CMAKE_SOURCE_DIR}/crazyflie/lib")

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
-I${CRAZYFLIE_LIB_DIR}/inc/cfdriver/bosch \
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
-I${CMAKE_SOURCE_DIR} \
-I${CMAKE_BINARY_DIR} \
-I${CMAKE_BINARY_DIR}/crazyflie/lib/inc \
-I${CRAZYFLIE_DRIVER_DIR}/qfplib")

set(CF_STDPERI_SOURCES stm32f4xx_adc.c stm32f4xx_dbgmcu.c stm32f4xx_dma.c stm32f4xx_exti.c stm32f4xx_flash.c stm32f4xx_gpio.c stm32f4xx_i2c.c stm32f4xx_iwdg.c stm32f4xx_rcc.c stm32f4xx_spi.c stm32f4xx_tim.c stm32f4xx_usart.c stm32f4xx_misc.c stm32f4xx_syscfg.c)
#Removed after usart: stm32f4xx_can.c stm32f4xx_crc.c stm32f4xx_dac.c stm32f4xx_fsmc.c stm32f4xx_hash.c stm32f4xx_dma2d.c stm32f4xx_hash_md5.c stm32f4xx_hash_sha1.c stm32f4xx_ltdc.c stm32f4xx_pwr.c stm32f4xx_rtc.c stm32f4xx_sdio.c stm32f4xx_wwdg.c stm32f4xx_sai.c stm32f4xx_rng.c stm32f4xx_dcmi.c stm32f4xx_cryp_tdes.c stm32f4xx_cryp_aes.c stm32f4xx_cryp_des.c stm32f4xx_cryp.c

set(CFPLATFORM_SOURCES platform_cf2.c platform.c platform_stm32f4.c platform_utils.c)

set(CFMODULES_SOURCES lighthouse_storage.c health.c mm_sweep_angles.c mm_distance.c mm_flow.c mm_tdoa.c mm_pose.c mm_yaw_error.c mm_position.c mm_absolute_height.c mm_tof.c mm_tdoa_robust.c lighthouse_deck_flasher.c kalman_supervisor.c lighthouse_position_est.c app_channel.c collision_avoidance.c peer_localization.c position_controller_indi.c kalman_core.c pptraj_compressed.c controller_indi.c static_mem.c tdoaEngineInstance.c lighthouse_core.c attitude_pid_controller.c comm.c commander.c console.c controller.c controller_mellinger.c controller_pid.c crtp.c crtp_commander.c crtp_commander_generic.c crtp_commander_high_level.c crtp_commander_rpyt.c crtp_localization_service.c crtpservice.c estimator.c estimator_complementary.c estimator_kalman.c extrx.c log.c mem.c msp.c outlierFilter.c param.c pid.c planner.c platformservice.c position_controller_pid.c position_estimator_altitude.c power_distribution_stock.c pptraj.c queuemonitor.c range.c sensfusion6.c sound_cf2.c stabilizer.c sysload.c system.c worker.c eventtrigger.c supervisor.c)

set(CFDECK_SOURCES activeMarkerDeck.c activeMarkerUartTest.c aideck.c aidecktest.c deck_spi3.c exptestBolt.c lhtesterdeck.c uart1test.c uart2test.c uarttest.c locodeck.c flowdeck_v1v2.c deck.c deck_drivers.c deck_info.c deck_test.c deck_analog.c deck_constants.c deck_digital.c deck_spi.c ledring12.c lpsTwrTag.c lpsTdoa2Tag.c lpsTdoa3Tag.c zranger.c zranger2.c bigquad.c buzzdeck.c cppmdeck.c gtgps.c lighthouse.c multiranger.c oa.c usddeck.c exptest.c exptestRR.c deck_memory.c)

set(CFDRIVER_SOURCES lh_bootloader.c exti.c nvic.c motors.c fatfs_sd.c ak8963.c cppm.c eeprom.c i2cdev.c i2c_drv.c led.c lps25h.c maxsonar.c mpu6500.c pca9685.c piezo.c pmw3901.c swd.c uart1.c uart2.c uart_syslink.c vl53l0x.c vl53l1x.c watchdog.c ws2812_cf2.c)

set(CFBOSCH_SOURCES bmi055_accel.c bmi055_gyro.c bmi160.c bmp280.c bstdr_comm_support.c bmm150.c bmi088_accel.c bmi088_gyro.c bmi088_fifo.c bmp3.c)

Set(CFUTILS_SOURCES kve_storage.c kve.c rateSupervisor.c pulse_processor_v1.c ootx_decoder.c lighthouse_calibration.c pulse_processor_v2.c cfassert.c clockCorrectionEngine.c configblockeeprom.c cpuid.c crc32.c eprintf.c filter.c FreeRTOS-openocd.c num.c sleepus.c debug.c version.c tdoaEngine.c tdoaStats.c tdoaStorage.c lighthouse_geometry.c pulse_processor.c statsCnt.c)

set(CFFREERTOS_SOURCES croutine.c event_groups.c list.c queue.c tasks.c timers.c port.c heap_4.c stream_buffer.c)

set(CFHAL_SOURCES amg8833.c storage.c ow_common.c sensors.c buzzer.c freeRTOSdebug.c ledseq.c ow_syslink.c pca95x4.c pm_stm32f4.c proximity.c radiolink.c syslink.c usb.c usb_bsp.c usbd_desc.c usblink.c usec_time.c pca9555.c sensors_bmi088_bmp388.c sensors_mpu9250_lps25h.c sensors_bmi088_i2c.c)
#sensors_bmi088_bmp388.c sensors_mpu9250_lps25h.c

set(STM32_SYS_SOURCE system_stm32f4xx.c startup_stm32f40xx.s)

set(STM32_USBDEVICELIB_SOURCE usbd_core.c usbd_ioreq.c usbd_req.c)

set(STM32_USBOTG_DRIVER_SOURCE usb_core.c usb_dcd.c usb_dcd_int.c)

set(VL53L1_SOURCE vl53l1_api.c vl53l1_api_calibration.c vl53l1_api_core.c vl53l1_api_debug.c vl53l1_api_preset_modes.c vl53l1_api_strings.c vl53l1_core.c vl53l1_core_support.c vl53l1_error_strings.c vl53l1_register_funcs.c vl53l1_silicon_core.c vl53l1_wait.c)

set(FATFS_SOURCES ffunicode.c ff.c)

set(LIBDW1000_SOURCES libdw1000.c libdw1000Spi.c)

set(DSP_FILTERINGFUNCTIONS_SOURCES arm_biquad_cascade_df1_f32.c arm_biquad_cascade_df1_init_f32.c arm_biquad_cascade_df2T_f32.c arm_biquad_cascade_df2T_f64.c arm_biquad_cascade_df2T_init_f32.c arm_biquad_cascade_df2T_init_f64.c arm_biquad_cascade_stereo_df2T_f32.c arm_biquad_cascade_stereo_df2T_init_f32.c arm_conv_f32.c arm_conv_partial_f32.c arm_correlate_f32.c arm_fir_decimate_f32.c arm_fir_decimate_init_f32.c arm_fir_f32.c arm_fir_init_f32.c arm_fir_interpolate_f32.c arm_fir_interpolate_init_f32.c arm_fir_lattice_f32.c arm_fir_lattice_init_f32.c arm_fir_sparse_f32.c arm_fir_sparse_init_f32.c arm_iir_lattice_f32.c arm_iir_lattice_init_f32.c arm_lms_f32.c arm_lms_init_f32.c arm_lms_norm_f32.c arm_lms_norm_init_f32.c)

set(DSP_STATISTICFUNCTIONS_SOURCES arm_max_f32.c arm_min_f32.c arm_power_f32.c arm_rms_f32.c arm_var_f32.c arm_std_f32.c arm_mean_f32.c)

set(DSP_MATRIXFUNCTIONS_SOURCES arm_mat_add_f32.c arm_mat_cmplx_mult_f32.c arm_mat_init_f32.c arm_mat_inverse_f32.c arm_mat_mult_f32.c arm_mat_scale_f32.c arm_mat_sub_f32.c arm_mat_trans_f32.c)

set(DSP_FASTMATHFUNCTIONS_SOURCES arm_cos_f32.c arm_sin_f32.c)

set(DSP_CONTROLLERFUNCTIONS_SOURCES arm_pid_init_f32.c arm_pid_reset_f32.c arm_sin_cos_f32.c)

set(DSP_COMMONTABLES_SOURCES arm_common_tables.c arm_const_structs.c)

set(DSP_TRANSFORMFUNCTIONS_SOURCES arm_bitreversal2.S arm_cfft_f32.c arm_cfft_radix2_f32.c arm_cfft_radix2_init_f32.c arm_cfft_radix4_f32.c arm_cfft_radix4_init_f32.c arm_cfft_radix8_f32.c arm_dct4_f32.c arm_dct4_init_f32.c arm_rfft_f32.c arm_rfft_fast_f32.c arm_rfft_fast_init_f32.c arm_rfft_init_f32.c)

set(DSP_COMPLEXMATHFUNCTIONS_SOURCES arm_cmplx_conj_f32.c arm_cmplx_dot_prod_f32.c arm_cmplx_mag_f32.c arm_cmplx_mag_squared_f32.c arm_cmplx_mult_cmplx_f32.c arm_cmplx_mult_real_f32.c)

set(DSP_BASICMATHFUNCTIONS_SOURCES arm_abs_f32.c arm_add_f32.c arm_dot_prod_f32.c arm_mult_f32.c arm_negate_f32.c arm_offset_f32.c arm_scale_f32.c arm_sub_f32.c)

set(DSP_SUPPORTFUNCTIONS_SOURCES arm_copy_f32.c arm_fill_f32.c)

set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/crazyflie/lib/linker/FLASH_CLOAD.ld")
#set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/crazyflie/lib/linker/stm32f4cf.ld")
set(LIBS "-L${CRAZYFLIE_LIB_DIR}/linker")
#-L${CRAZYFLIE_DRIVER_DIR}/CMSIS/Lib
set(REV "D")
set(ESTIMATOR "any")
set(CONTROLLER "Any") # one of Any, PID, Mellinger
set(POWER_DISTRIBUTION "stock")
set(CLOAD_SCRIPT "python3 -m cfloader")
set(LPS_TDOA_ENABLE 0)
set(LPS_TDOA3_ENABLE 0)
set(LPS_TDMA_ENABLE 0)
set(LOCODECK_USE_ALT_PINS 0)

if(LPS_TDOA_ENABLE)
  set(LPS_DEF "-DLPS_TDOA_ENABLE=1")
else(LPS_TDOA_ENABLE) 
  set(LPS_DEF "${LPS_DEF} -DLPS_TDOA_ENABLE=0")
endif(LPS_TDOA_ENABLE)

if(LPS_TDOA3_ENABLE)
  set(LPS_DEF "${LPS_DEF} -DLPS_TDOA3_ENABLE=1")
else(LPS_TDOA3_ENABLE)
  set(LPS_DEF "${LPS_DEF} -DLPS_TDOA3_ENABLE=0")
endif(LPS_TDOA3_ENABLE)

if(LPS_TDMA_ENABLE)
  set(LPS_DEF "${LPS_DEF} -DLPS_TDMA_ENABLE=1")
else(LPS_TDMA_ENABLE)
  set(LPS_DEF "${LPS_DEF} -DLPS_TDMA_ENABLE=0")
endif(LPS_TDMA_ENABLE)

if(LOCODECK_USE_ALT_PINS)
  set(LPS_DEF "${LPS_DEF} -DLOCODECK_USE_ALT_PINS=1")
endif(LOCODECK_USE_ALT_PINS)

set(PROCESSOR "-mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16")

set(DEFS "-DCRAZYFLIE_FW -DARM_MATH_CM4 -D__FPU_PRESENT=1 -D__TARGET_FPU_VFP -DSTM32F4XX -DSTM32F40_41xxx -DHSE_VALUE=8000000 -DUSE_STDPERIPH_DRIVER -DUSE_RADIOLINK_CRTP -DENABLE_UART -DBOARD_REV_${REV} -DESTIMATOR_NAME=${ESTIMATOR}Estimator -DCONTROLLER_NAME=ControllerType${CONTROLLER} -DPOWER_DISTRIBUTION_TYPE_${POWER_DISTRIBUTION}")

set(SENSOR_CONFIG "-DSENSORS_IGNORE_BAROMETER_FAIL -DSENSOR_INCLUDED_MPU9250_LPS25H -DSENSOR_INCLUDED_BMI088_BMP388")
#-DSENSOR_INCLUDED_BMI088_BMP388 -DSENSOR_INCLUDED_MPU9250_LPS25H

#set(FORCEDEVICE "-DDEVICE_TYPE_STRING_FORCE=CF20 -DDECK_FORCE=bcDWM1000")

set(CFLAGS "-Os -std=gnu11 -g3 -mfp16-format=ieee")
#originally: uses -std=gnu99 -Os -g3
#set(CFLAGS "${CFLAGS} -Wextra -Wshadow -Wimplicit-function-declaration")
#set(CFLAGS "${CFLAGS} -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes -Wno-comment -Wno-unused-variable -Wno-unused-function")
set(CFLAGS "${CFLAGS} -ffunction-sections -fdata-sections -fno-math-errno")
set(CFLAGS "${CFLAGS} -MD -MP -Wmissing-braces -Wall -Wundef -Wdouble-promotion")
set(CFLAGS "${CFLAGS} ${INCLUDE_DIR} ${DEFS} ${PROCESSOR} ${SENSOR_CONFIG} ${LPS_DEF} ${FORCEDEVICE}")
set(CFLAGS "${CFLAGS} -fno-strict-aliasing -fshort-wchar")
# set(CFLAGS "${CFLAGS} -fno-builtin -fomit-frame-pointer -mabi=aapcs -fno-unroll-loops -ffast-math -ftree-vectorize -fno-common -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -fomit-frame-pointer -ffast-math")

set(LDFLAGS "--static ${PROCESSOR} ${LIBS} -T${LINKER_SCRIPT}")
set(LDSCRIPT "")
#set(LDFLAGS "${LDFLAGS} -Wl,--gc-sections ${LIBS} ${CRAZYFLIE_LIB_DIR}/linker/startup_stm32f0xx.S ${CRAZYFLIE_DRIVER_DIR}/qfplib/qfplib.s")
set(LDFLAGS "--specs=nosys.specs --specs=nano.specs ${LDFLAGS} -Wl,--gc-sections,--cref,--undefined=uxTopUsedPriority,--no-wchar-size-warning,-u,__errno")
#-Wl,-u,__errno,--cref,--gc-sections,--undefined=uxTopUsedPriority ${LIBS} ${CRAZYFLIE_DRIVER_DIR}/CMSIS/Device/ST/STM32F4xx/Source/startup_stm32f40xx.s")
#set(LDFLAGS "${LDFLAGS},--specs=rdimon.specs --cref, --undefined=uxTopUsedPriority")
#--static -nostartfiles -g3 in LDFLAGS

# LDLIBS
#set(LDLIBS "-Wl,--start-group -lm -Wl,--end-group")
set(LDLIBS "-Wl,--start-group -lm -Wl,--end-group")
#-u,__errno,

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
option(BBZ_NEIGHBORS_USE_FLOATS "Whether to use floats for the neighbor's range and bearing measurments." ON)
option(BBZ_ENABLE_FLOAT_OPERATIONS "Whether to enable floats operations" ON)
option(BBZ_BYTEWISE_ASSIGNMENT "Whether to make assignment byte per byte or directly. (used to ensure compatibility with Cortex-M0)" OFF) #Turned ON for Cortex-M0. CF uses Cortex-M4
set(BBZHEAP_SIZE 3500)
set(BBZSTACK_SIZE 440)
# message("BBZHEAP_SIZE := ${BBZHEAP_SIZE}")
set(BBZHEAP_GCMARK_DEPTH 16)
set(BBZNEIGHBORS_CAP 10)
set(BBZMSG_IN_PROC_MAX 10)

#originally: all commented out
#set(BBZ_DISABLE_NEIGHBORS ON)
#set(BBZ_DISABLE_VSTIGS ON)
# set(BBZ_DISABLE_SWARMS ON)

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
        COMMAND ${OBJCOPY} -Obinary --pad-to 0 --remove-section=.bss --remove-section=.nzds  --remove-section=._usrstack ${_ELF_TARGET} ${_BIN_TARGET}
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
        COMMENT "Uploading ${_HEX_TARGET} to Crazyflie ${robot_id}")

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
