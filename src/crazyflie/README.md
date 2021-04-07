BittyBuzz on Crazyflie
===================

This is a documentation on how to setup the development environment to run **BittyBuzz** on **Crazyflie**. This was only tested on **Ubuntu 16.04** and **Ubuntu 18.04**

Prerequisites
=============

You need the [ST-LINK/V2](http://www.st.com/en/development-tools/st-link-v2.html) programmer to program the Zooids.

Programs that need to be installed in your machine:
* [Buzz](https://github.com/MISTLab/Buzz) (Don't forget to run `sudo ldconfig`
after the installation)
* The [stlink](https://github.com/texane/stlink) suite (we need `st-flash` to send 
programs and `st-util` to debug)
* CMake 3.5.1 or higher (on Debian distributions `sudo apt-get install cmake`)
* The ARM compiler (on Debian distributions `sudo apt-get install gcc-arm-none-eabi`)
* The ARM debugger (on Debian distributions `sudo apt-get install gdb-arm-none-eabi`)
* The GNU C Compiler (on Debian distributions `sudo apt-get install gcc`)


Compilation
===========

At the moment, BittyBuzz can only compile Buzz scripts that are placed in the
`<BittyBuzz_Root>/src/<robot>/behaviors`. 
You have to provide a *C* script that will link your compiled Buzz script with the *BittyBuzz VM*. You must also provide a *Buzz Symbol Table* file (`.bst`). It's no longer needed to put anything in this file. It will eventually become optionnal. You can take example on the scripts
currently in that folder. The C, BST and Buzz files must all have to same name.

Setup the build directory
-------------------------

These are the steps you have to follow to build BittyBuzz for Crazyflie:

1. In a terminal, go to the root directory of the BittyBuzz repository.
2. Run `mkdir build_crazyflie && cd build_crazyflie`
3. Run `cmake -DCMAKE_TOOLCHAIN_FILE=../src/cmake/Crazyflie.cmake ../src`
4. Build using `make RESULT=<ID>`
   whereby `<ID>` is replaced by the number ID of the crazyflie you wish to assign

Build your scripts
------------------

You can now run `make help` to get the list of all Make targets you can build.
For example, you can run `make behaviors RESULT=0` to build all the Buzz scripts.

You have to specify the ID of the robot to which you wish to upload the program in
the `RESULT` variable.

Uploading behavior script to Crazyflie
==================

You can upload the script wirelessly to the Crazyflie using Crazyradio PA dongle. To do so, follow instructions in [Crazyflie-lib-python](https://github.com/bitcraze/crazyflie-lib-python) to set up your dongle. At Crazyflie-lib-python root directory, you can setup cflib system-wide by running `python3 setup.py install`.

Once done, you can flash your Crazyflie by running:
`
python3 -m cfloader flash build_crazyflie/crazyflie/behaviors/<behavior>-crazyflie.bin stm32-fw
`
where `<behavior>` is the name of your behavior script. 

**Note:**
Using cfloader script to flash your Crazyflie wirelessly only works for .bin files.

Debugging
=========

Normal case
-----------

There is actually no way to debug a Buzz script directly. However, you can debug
the BittyBuzz VM itself while running a Buzz script on it.

You will first need to compile your Buzz script as mentioned higher above in the
Compilation section. This will produce a `.elf` file in the output directory.

Then, plug your _ST-LINK/V2_ programmer to your computer and make sure that its
pins make contact with the zooid.

Then, open a terminal window and launch `st-util` to start the debug server. The _ST-LINK/V2_ should blink
red and green and then turn green to notify that it's ready. If the led turns
yellow, stop the `st-util` program (with `CTRL-C`) and unplug/plug the
_ST-LINK/V2_ and make sure that the zooid is turned ON, that it has enough battery
and that the pins make contact with the board of the zooid.

You must now run the ARM Debugger and follow these next steps:

1. Run `arm-none-eabi-gdb -q <path to the build directory>/zooids/behaviors/<name of your script>-zooids.elf` 
2. In the debugger, type `target extended-remote :4242` to connect to the debugger server.
3. In the debugger, type `load` to load the script in the zooid.

You can now run the ARM Debugger as if it was a normal `gdb` debugger. For example, you can put a breakpoint in the main function (warning, the MCU running on Zooids can only handle a maximum of 4 break points):

4. `br main`
5. `c`

You might want to check the functions in `bittybuzz/bbzvm.c` and the macros and structures defined in `bittybuzz/bbzvm.h` which could be helpful while debugging.

Here are some useful command that could help you debug your scripts:

* printing the value of an expression: `p <expr>` (ex.: `p *vm` will print the current state of the VM)
  * printing the current state of the stack: `p vm->stack`
  * printing the value of an object on the heap: `p *bbzheap_obj_at(<idx>)` (replace `<idx>` with the Heap Index of the object you're looking for)
  * printing the address of the current instruction: `p vm->pc`

Firmware update steps
=====================

**Note:**
In cases where there is a sub-folder in the firmware but not one the BittyBuzz file system, do not add a sub-folder.

1. Replace all header files in `crazyflie/lib/inc/cf<source_type>` by all the header files (.h) in `crazyflie-firmware/src/<source_type>`.
2. Replace all source files in `crazyflie/lib/src/cf<source_type>` by all the source files (.c) in `crazyflie-firmware/src/<source_type>`.
3. Put back BittyBuzz macros that were in `crazyflie/lib/inc/cfconfig/FreeRTOSConfig.h` and `crazyflie/lib/inc/cfconfig/config.h` (ex: BBZ_TASK_NAME, BBZ_TASK_STACKSIZE, etc).
4. Check `crazyflie/lib/src/bbzcrazyflie.c` to make sure all the macros and data types used here have been defined in one of the 2 config files.
5. Replace all header files in `crazyflie/lib/inc/FreeRTOS/` by all the header files (.h) in `crazyflie-firmware/vendor/FreeRTOS/include`.
6. Replace all source files in `crazyflie/lib/src/FreeRTOS/` by all the source files (.c) in `crazyflie-firmware/vendor/FreeRTOS/` and `crazyflie-firmware/vendor/FreeRTOS/portable/MemMang`.
7. Replace the following folders in `crazyflie/drivers/` by the corresponding in `crazyflie-firmware/src/lib`: FatFS, STM32* and vl53l1.
8. Replace `crazyflie/driver/CMSIS/Device/ST/STM32F4xx` by `crazyflie-firmware/src/lib/CMSIS/STM32F4xx`.
9. Add `crazyflie-firmware/src/init/startup_stm32f40xx.s` to `crazyflie/driver/CMSIS/Device/ST/STM32F4xx/Source`.
10. Replace all `.ld` files in `crazyflie/lib/linker/` by the linker scripts in `crazyflie-firmware/tools/make/F405/linker`.
11. Make sure that all the new files (if any) are added in the corresponding sources variable in `../cmake/Crazyflie.cmake` (ex: set(CFMODULES_SOURCES lighthouse_storage.c ...), etc ).
12. Remove deleted files name from `../cmake/Crazyflie.cmake`.
13. Try to build and install a behavior on the crazyflie. (You might have some errors related to the file system structure, simply adapt the include statements to the new structure).

**Trouble shouting**
- In case of linker errors, you are probably missing some new files in `../cmake/Crazyflie.cmake`.
- If some variables of functions are undefined or the flashing fails, you might be missing some flags. Compare the flags set in `../cmake/Crazyflie.cmake` and the compilation flags in `crazyflie/behaviors/CMakeLists.txt` to the flags used in `crazyflie-firmware/Makefile` and `crazyflie-firmware/tools/make/targets.mk`.