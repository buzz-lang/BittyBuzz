#
# Look for the necessary tools
#
find_program(AVR_AR avr-ar)
find_program(AVR_CC avr-gcc)
find_program(AVR_OBJCOPY avr-objcopy)
find_program(AVR_OBJDUMP avr-objdump)
find_program(AVR_STRIP avr-strip)
find_program(AVR_UPLOAD avrdude)

#
# Toolchain variables
#
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR avr)
set(CMAKE_C_COMPILER ${AVR_CC})

#
# AVR variables
#
set(AVR_MCU "atmega328p")    # MCU for avr-gcc
set(AVR_DEVICE "m328p")      # device for avrdude -p
set(AVR_PORT "usb")          # port for avrdude -P
set(AVR_BOOTLOADER "0x7000") # section start for bootloader code

set(AVR_CFLAGS "-std=c99 -mmcu=${AVR_MCU} -Wall -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -fno-strict-aliasing -ffunction-sections -fdata-sections -DF_CPU=8000000")
set(AVR_LDFLAGS "-mmcu=${AVR_MCU} -Wl,-s -Wl,--gc-sections")

#
# Variable caching (for compiler swap functions)
#
set(NATIVE_SYSTEM_NAME ${CMAKE_SYSTEM_NAME} CACHE STRING
    "Name of the system we are programming for.")
set(NATIVE_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR} CACHE STRING
    "Processor type for the system we are programming for.")
set(NATIVE_C_COMPILER ${CMAKE_C_COMPILER} CACHE STRING
    "Path to the compiler for the system we are programing for.")

set(NATIVE_C_FLAGS " " CACHE STRING "GCC flags for the native compiler.")
set(CURRENT_COMPILER "NATIVE" CACHE STRING "Which compiler we are using.")

#
# BittyBuzz variables
#
set(BBZ_ROBOT kilobot)

#
# CMake command to compile an executable
#
function(kilobot_add_executable _TARGET)
  # Target names
  set(_ELF_TARGET ${_TARGET}-${BBZ_ROBOT}.elf)
  set(_LSS_TARGET ${_TARGET}-${BBZ_ROBOT}.lss)
  set(_BIN_TARGET ${_TARGET}-${BBZ_ROBOT}.bin)
  set(_HEX_TARGET ${_TARGET}-${BBZ_ROBOT}.hex)
  set(_EEP_TARGET ${_TARGET}-${BBZ_ROBOT}.eep)
  set(_MAP_TARGET ${_TARGET}-${BBZ_ROBOT}.map)
  # .elf -> .lss
  add_custom_command(OUTPUT ${_LSS_TARGET}
    COMMAND ${AVR_OBJDUMP} -d -S ${_ELF_TARGET} > ${_LSS_TARGET}
    DEPENDS ${_ELF_TARGET})
  # .elf -> .hex
  add_custom_command(OUTPUT ${_HEX_TARGET}
    COMMAND ${AVR_OBJCOPY} -O ihex -R .eeprom -R .fuse -R .lock -R .signature ${_ELF_TARGET} ${_HEX_TARGET}
    DEPENDS ${_ELF_TARGET})
  # .elf -> .eep
  add_custom_command(OUTPUT ${_EEP_TARGET}
    COMMAND ${AVR_OBJCOPY} -O ihex -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 ${_ELF_TARGET} ${_EEP_TARGET}
    DEPENDS ${_ELF_TARGET})
  # .elf -> .bin
  add_custom_command(OUTPUT ${_BIN_TARGET}
    COMMAND ${AVR_OBJCOPY} -O binary -R .eeprom -R .fuse -R .lock -R .signature ${_ELF_TARGET} ${_BIN_TARGET}
    DEPENDS ${_ELF_TARGET})
  # Compile .elf file
  add_executable(${_ELF_TARGET} EXCLUDE_FROM_ALL ${ARGN})
  math(EXPR _BCODE_ADDR "28672-(79+2)")
  set_target_properties(${_ELF_TARGET}
    PROPERTIES
    COMPILE_FLAGS "${AVR_CFLAGS}"
    LINK_FLAGS "${AVR_LDFLAGS} -Wl,-Map,${_MAP_TARGET} -Wl,--section-start=.bcode=0x${_BCODE_ADDR}")

  # Make target
  add_custom_target(${_TARGET} DEPENDS ${_LSS_TARGET} ${_HEX_TARGET} ${_EEP_TARGET} ${_BIN_TARGET})
  set_target_properties(${_TARGET} PROPERTIES OUTPUT_NAME "${_ELF_TARGET}")

  # Uploading file
  add_custom_target(upload_${_TARGET}
    ${AVR_UPLOAD} -p ${AVR_DEVICE} -P ${AVR_PORT} -c avrispmkII -U "flash:w:${_HEX_TARGET}:i"
    DEPENDS ${_HEX_TARGET}
    COMMENT "Uploading ${_HEX_TARGET} to Kilobot")

  # Uploading/Initializing EEPROM
  add_custom_target(eep_upload_${_TARGET}
    ${AVR_UPLOAD} -p ${AVR_DEVICE} -P ${AVR_PORT} -c avrispmkII -U "eeprom:w:${_EEP_TARGET}:i"
    DEPENDS ${_EEP_TARGET}
    COMMENT "Uploading EEPROM data from ${_EEP_TARGET} to Kilobot")

  # Extra files to clean
  get_directory_property(_CLEAN_FILES ADDITIONAL_MAKE_CLEAN_FILES)
  set_directory_properties(
    PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "${_CLEAN_FILES};${_MAP_TARGET}")
endfunction(kilobot_add_executable _TARGET)

#
# CMake command to compile a library
#
function(kilobot_add_library _TARGET)
  # Set library file name
  set(_LIB_TARGET ${_TARGET}-${BBZ_ROBOT})
  # Add library target
  add_library(${_TARGET} STATIC ${ARGN})
  set_target_properties(${_TARGET} PROPERTIES 
    COMPILE_FLAGS ${AVR_CFLAGS}
    LINK_FLAGS ${AVR_LDFLAGS}
    OUTPUT_NAME ${_LIB_TARGET})
endfunction(kilobot_add_library _TARGET)

#
# CMake command to link a target to a library
#
function(kilobot_target_link_libraries _TARGET)
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
endfunction(kilobot_target_link_libraries _TARGET)