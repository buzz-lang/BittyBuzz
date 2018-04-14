BittyBuzz on Zooids
===================

This is a documentation on how to setup the development environment to run **BittyBuzz** on **Zooids**. This was only tested on **Ubuntu 16.04**

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
`<BittyBuzz_Root>/src/<robot>/behaviors`. You have to provide a *C* script that will
link your compiled Buzz script with the *BittyBuzz VM*. You must also provide a
*Buzz Symbol Table* file (`.bst`). It's not needed anymore to put anything in this
file. It will eventually become optionnal. You can take example on the scripts
currently in that folder. The C, BST and Buzz files must all have to same name.

Setup the build directory
-------------------------

These are the steps you have to follow to build BittyBuzz for Zooids:

1. In a terminal, go in the root directory of the BittyBuzz repository.
2. Run `mkdir build-zooids && cd build-zooids`.
3. Run `cmake -DCMAKE_TOOLCHAIN_FILE=../src/cmake/Zooids.cmake ../src`

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

EXAMPLE DE DEBUGGING
PARLER DE L'ÉMULATEUR
COMMENTER LE CODE (LES MODIFS & LE CMAKE AVEC LES OBJECT AU LIEU DES LIBRAIRIES)