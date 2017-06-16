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

Options
-------

It is possible to specify a custom value for these configuration values.

- `BBZHEAP_SIZE` : Size of the heap (in bytes) [Default: 1024].
- `BBZHEAP_ELEMS_PER_TSEG` : Number of table entries per table segment [Default: 5].
- `BBZSTACK_SIZE` : Size of the VM's stack (in number of objects) [Default: 256].
- `BBZVSTIG_CAP` : Capacity of the virtual stigmergy (in number of entries) [Default: 3].
- `BBZNEIGHBORS_CAP` : Capacity of the neighbors structure (in number of neighbors) [Default: 4, Max: 255]. Used only when `BBZ_XTREME_MEMORY` is ON.
- `RESERVED_ACTREC_MAX` : Number of table segments reserved for lambdas'
activation record [Default: 32, Max: 254].
- `BBZ_XTREME_MEMORY` : Whether to use high memory optimizations [Default: ON for kilobots otherwise OFF, Values: {ON, OFF}].

For example, for a Buzz program requiring larger stack sizes but less heap allocations, you may run cmake as:

    $ cmake -DBBZHEAP_SIZE=768 -DBBZSTACK_SIZE=384 ../src