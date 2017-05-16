What is BittyBuzz?
==================

BittyBuzz is an implementation of the
[Buzz programming language](http://the.swarming.buzz) for
microcrontrollers. BittyBuzz is designed to fit a 32k flash memory and
work with as little as 2k of RAM.

While the BittyBuzz VM has a number of limitations with respect to the
original Buzz VM, BittyBuzz supports 100% of the Buzz bytecode.

Compiling BittyBuzz
===================

For your PC
-----------

Type these commands:

    $ cd bittybuzz
    $ mkdir build
    $ cd build
    $ cmake -DHEAP_SIZE=1024 -DHEAP_ELEMS_PER_TSEG=5 -DSTACK_SIZE=256 ../src/

For the Kilobot
---------------

To compile BittyBuzz to an .hex file that can be used with the
Kilobot, you need _avr-gcc_ and related tools.

Type these commands:

    $ cd bittybuzz
    $ mkdir build_kilobot
    $ cd build_kilobot
    $ cmake -DCMAKE_TOOLCHAIN_FILE=../src/cmake/Kilobot.cmake -DHEAP_SIZE=1024 -DHEAP_ELEMS_PER_TSEG=5 -DSTACK_SIZE=256 ../src/
    $ make
