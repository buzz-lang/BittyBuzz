include_directories(${CMAKE_CURRENT_BINARY_DIR})
link_directories(${CMAKE_CURRENT_BINARY_DIR})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

enable_language(ASM)
kilobot_add_library(bbzkilobot
        bbzbitfield.h
        bbzkilobot.h
        bbzkilobot.c
        bbzmacros.h
        bbzmessage.h
        bbzmessage_crc.h
        bbzmessage_crc.c
        bbzmessage_send.h
        bbzmessage_send.S
        config.h
)

kilobot_add_executable(blank bbzblank.c)
kilobot_target_link_libraries(blank bbzkilobot bittybuzz)