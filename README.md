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
    $ cmake ../src/
    $ make

Should you wish to, you may specify a custom value for these variables:

- HEAP_SIZE: Size of the heap (in bytes) [Default: 1024].
- HEAP_ELEMS_PER_TSEG: Number of table entries per table segment [Default: 5].
- STACK_SIZE: Size of the VM's stack (in number of objects) [Default: 256].
- MAX_ACTREC_COUNT: Number of table segments reserved for lambdas'
activation record [Default: 32].

For example, for a Buzz program requiring larger stack sizes but less heap allocations, you may run cmake as:

    $ cmake -DHEAP_SIZE=768 -DSTACK_SIZE=384 ../src

For the Kilobot
---------------

To compile BittyBuzz to an .hex file that can be used with the
Kilobot, you need _avr-gcc_ and related tools.

Type these commands:

    $ cd bittybuzz
    $ mkdir build_kilobot
    $ cd build_kilobot
    $ cmake -DCMAKE_TOOLCHAIN_FILE=../src/cmake/Kilobot.cmake ../src/
    $ make

The configuration options available for this compilation type are similar
to those of the PC compilation.