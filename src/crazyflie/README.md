BittyBuzz on Crazyflie
===================

This is a documentation on how to setup the development environment to run **BittyBuzz** on **Crazyflie**. This was only tested on **Ubuntu 16.04**

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
   whereby <ID> is replaced by the number ID of the crazyflie you wish to assign

Build your scripts
------------------

You can now run `make help` to get the list of all Make targets you can build.
For example, you can run `make behaviors RESULT=0` to build all the Buzz scripts.

You have to specify the ID of the robot to which you wish to upload the program in
the `RESULT` variable.

Program the Zooids
==================

You can send any of the Buzz scripts to a Zooids by simply running the command
`make upload_<script_name> RESULT=<robot_id>` . Replace `<script_name>` by the name
of the Buzz script you want to upload and `<robot_id>` by the ID of the robot you
want to upload the script to.

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
