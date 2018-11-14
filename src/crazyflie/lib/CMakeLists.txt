include_directories(${CMAKE_CURRENT_BINARY_DIR}/inc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/incL)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/srcL)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../drivers/STM32F0xx_HAL_Driver/Inc)
link_directories(${CMAKE_CURRENT_BINARY_DIR})


FUNCTION(PREPEND var prefix)
   SET(listVar "")
   FOREACH(f ${ARGN})
      LIST(APPEND listVar "${prefix}/${f}")
   ENDFOREACH(f)
   SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(PREPEND)

PREPEND(CFPLATFORM_SOURCES "${CRAZYFLIE_LIB_DIR}/src/cfplatform" ${CFPLATFORM_SOURCES})
PREPEND(CFDRIVER_SOURCES "${CRAZYFLIE_LIB_DIR}/src/cfdriver" ${CFDRIVER_SOURCES})
PREPEND(CFUTILS_SOURCES "${CRAZYFLIE_LIB_DIR}/src/cfutils" ${CFUTILS_SOURCES}) 
PREPEND(CFFREERTOS_SOURCES "${CRAZYFLIE_LIB_DIR}/src/FreeRTOS" ${CFFREERTOS_SOURCES})
PREPEND(CF_STDPERI_SOURCES "${CRAZYFLIE_DRIVER_DIR}/STM32F4xx_StdPeriph_Driver/src" ${CF_STDPERI_SOURCES})
PREPEND(CFDECK_SOURCES "${CRAZYFLIE_LIB_DIR}/src/cfdeck" ${CFDECK_SOURCES})
PREPEND(STM32_SYS_SOURCE "${CRAZYFLIE_DRIVER_DIR}/CMSIS/Device/ST/STM32F4xx/Source" ${STM32_SYS_SOURCE})
PREPEND(STM32_USBDEVICELIB_SOURCE "${CRAZYFLIE_DRIVER_DIR}/STM32_USB_Device_Library/Core/src" ${STM32_USBDEVICELIB_SOURCE})
PREPEND(STM32_USBOTG_DRIVER_SOURCE "${CRAZYFLIE_DRIVER_DIR}/STM32_USB_OTG_Driver/src" ${STM32_USBOTG_DRIVER_SOURCE})
PREPEND(VL53L1_SOURCE "${CRAZYFLIE_DRIVER_DIR}/vl53l1/core/src" ${VL53L1_SOURCE})
PREPEND(CFMODULES_SOURCES "${CRAZYFLIE_LIB_DIR}/src/cfmodules" ${CFMODULES_SOURCES})
PREPEND(CFHAL_SOURCES "${CRAZYFLIE_LIB_DIR}/src/cfhal" ${CFHAL_SOURCES})
PREPEND(FATFS_SOURCES "${CRAZYFLIE_DRIVER_DIR}/FatFS" ${FATFS_SOURCES})
PREPEND(LIBDW1000_SOURCES "${CRAZYFLIE_DRIVER_DIR}/libdw1000/src" ${LIBDW1000_SOURCES})
PREPEND(DSP_FILTERINGFUNCTIONS_SOURCES "${CRAZYFLIE_DRIVER_DIR}/CMSIS/DSP_Lib/Source/FilteringFunctions" ${DSP_FILTERINGFUNCTIONS_SOURCES})
PREPEND(DSP_STATISTICFUNCTIONS_SOURCES "${CRAZYFLIE_DRIVER_DIR}/CMSIS/DSP_Lib/Source/StatisticsFunctions" ${DSP_STATISTICFUNCTIONS_SOURCES})
PREPEND(DSP_MATRIXFUNCTIONS_SOURCES "${CRAZYFLIE_DRIVER_DIR}/CMSIS/DSP_Lib/Source/MatrixFunctions" ${DSP_MATRIXFUNCTIONS_SOURCES})
PREPEND(DSP_FASTMATHFUNCTIONS_SOURCES "${CRAZYFLIE_DRIVER_DIR}/CMSIS/DSP_Lib/Source/FastMathFunctions" ${DSP_FASTMATHFUNCTIONS_SOURCES})
PREPEND(DSP_CONTROLLERFUNCTIONS_SOURCES "${CRAZYFLIE_DRIVER_DIR}/CMSIS/DSP_Lib/Source/ControllerFunctions" ${DSP_CONTROLLERFUNCTIONS_SOURCES})
PREPEND(DSP_COMMONTABLES_SOURCES "${CRAZYFLIE_DRIVER_DIR}/CMSIS/DSP_Lib/Source/CommonTables" ${DSP_COMMONTABLES_SOURCES})
PREPEND(CFBOSCH_SOURCES "${CRAZYFLIE_LIB_DIR}/src/cfdriver/bosch" ${CFBOSCH_SOURCES})

enable_language(ASM)
crazyflie_add_library(bbzcrazyflie
        inc/bbzmain.h
        src/bbzcrazyflie.c
        inc/bbzcrazyflie.h
        #linker/startup_stm32f40xx.s
        #BY WJ: inc/colors.h
        #BY WJ: src/colors.c
        #BY WJ: inc/config.h
        #BY WJ: inc/functions.h
       #BY WJ:  src/functions.c
       #BY WJ:  inc/led.h
       #BY WJ:  src/led.c
       #BY WJ:  inc/lsm6ds3.h
       #BY WJ:  src/lsm6ds3.c
       #BY WJ:  inc/motors.h
       #BY WJ:  src/motors.c
      #BY WJ:   inc/nRF24L01.h
      #BY WJ:   inc/position_control.h
      #BY WJ:   src/position_control.c
      #BY WJ:   inc/qt1070.h
      #BY WJ:   src/qt1070.c
      #BY WJ:  inc/radio.h
      #BY WJ:   src/radio.c
      #BY WJ:   inc/RF24.h
      #BY WJ:   src/RF24.c
       #BY WJ:  inc/RingBuffer.h
      #BY WJ:   src/RingBuffer.c
      #BY WJ:   inc/sensors.h
       #BY WJ:  src/sensors.c
       #BY WJ:  inc/stm32f0xx_conf.h
       #BY WJ:  inc/stm32f0xx_hal_conf.h
      #BY WJ:   src/stm32f0xx_hal_msp.c
       #BY WJ:  inc/stm32f0xx_it.h
       #BY WJ:  src/stm32f0xx_it.c
       #BY WJ:  inc/stm32f0xx_lp_modes.h
       #BY WJ:  src/stm32f0xx_lp_modes.c
        #src/cfplatform/system_stm32f4xx.c
       #BY WJ:  inc/utilities.h
       #BY WJ:  src/utilities.c
        inc/cfconfig/config.h
        inc/cfconfig/ffconf.h
        inc/cfconfig/FreeRTOSConfig.h
        inc/cfconfig/nvicconf.h
        inc/cfconfig/stm32f4xx_conf.h
        inc/cfconfig/stm32fxxx.h
        inc/cfconfig/trace.h
        incL/memcpy_fast.h
        srcL/memcpy_fast.c
        incL/__cross_studio_io.h
        incL/__crossworks.h
        incL/__debug_stdio.h
        incL/cross_studio_io.h
        incL/debugio.h
        inc/FreeRTOS/portmacro.h
        inc/libarm_math.a
        ${CFDECK_SOURCES}
        ${CFUTILS_SOURCES}
        ${CFPLATFORM_SOURCES}
        ${CFDRIVER_SOURCES}
        ${CFFREERTOS_SOURCES}
        ${CF_STDPERI_SOURCES}
        ${STM32_SYS_SOURCE}
        ${STM32_USBDEVICELIB_SOURCE}
        ${STM32_USBOTG_DRIVER_SOURCE}
        ${VL53L1_SOURCE}
        ${CFMODULES_SOURCES}
        ${CFHAL_SOURCES}
        ${FATFS_SOURCES}
        ${LIBDW1000_SOURCES}
        ${DSP_FILTERINGFUNCTIONS_SOURCES}
        ${DSP_STATISTICFUNCTIONS_SOURCES}
        ${DSP_MATRIXFUNCTIONS_SOURCES}
        ${DSP_FASTMATHFUNCTIONS_SOURCES}
        ${DSP_CONTROLLERFUNCTIONS_SOURCES}
        ${DSP_COMMONTABLES_SOURCES}
        ${CFBOSCH_SOURCES}
)

# We have to use 'bbzcrazyflie_objects' instead of the usual library file because of an issue with the linker that prevent the script from initializing
crazyflie_add_executable(blank src/bbzblank.c $<TARGET_OBJECTS:bbzcrazyflie_objects>)
crazyflie_target_link_libraries(blank bbzcrazyflie bittybuzz)
