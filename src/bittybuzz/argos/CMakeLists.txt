#
# Create kilobots' BittyBuzz controller
#
add_library(argos3plugin_simulator_bbzkilobot SHARED
        bbz_kilobot_controller.h
        bbz_kilobot_controller.cpp
        bbz_main.c)
use_host_compiler()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fpermissive")

include_directories(control_interface)
add_executable(bbz_kilobot bbz_main.c)
target_link_libraries(bbz_kilobot
        bittybuzz_dl
        argos3plugin_simulator_kilolib
        rt)
