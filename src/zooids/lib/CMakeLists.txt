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

PREPEND(ZOOIDS_SOURCES "${HAL_DIR}/Src" ${ZOOIDS_SOURCES})

enable_language(ASM)
zooids_add_library(bbzzooids
        inc/bbzmain.h
        src/bbzzooids.c
        inc/bbzzooids.h
        # linker/startup_stm32f0xx.S
        inc/colors.h
        src/colors.c
        inc/config.h
        # inc/eeprom.h
        # src/eeprom.c
        inc/functions.h
        src/functions.c
        inc/led.h
        src/led.c
        inc/lsm6ds3.h
        src/lsm6ds3.c
        # inc/MadgwickAHRS.h
        # src/MadgwickAHRS.c
        inc/motors.h
        src/motors.c
        inc/nRF24L01.h
        inc/position_control.h
        src/position_control.c
        inc/qt1070.h
        src/qt1070.c
        inc/radio.h
        src/radio.c
        inc/RF24.h
        src/RF24.c
        inc/RingBuffer.h
        src/RingBuffer.c
        inc/sensors.h
        src/sensors.c
        inc/stm32f0xx_conf.h
        inc/stm32f0xx_hal_conf.h
        src/stm32f0xx_hal_msp.c
        inc/stm32f0xx_it.h
        src/stm32f0xx_it.c
        inc/stm32f0xx_lp_modes.h
        src/stm32f0xx_lp_modes.c
        src/system_stm32f0xx.c
        inc/utilities.h
        src/utilities.c
        incL/memcpy_fast.h
        srcL/memcpy_fast.c
        incL/__cross_studio_io.h
        incL/__crossworks.h
        incL/__debug_stdio.h
        incL/cross_studio_io.h
        incL/debugio.h
        ${ZOOIDS_SOURCES}
)

zooids_add_executable(blank src/bbzblank.c $<TARGET_OBJECTS:bbzzooids_objects>)
zooids_target_link_libraries(blank bbzzooids bittybuzz)