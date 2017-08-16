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
Kilobot, you need _avr-gcc_ and related tools installed under
`/usr/lib/avr/`.

Type these commands:

    $ cd bittybuzz
    $ mkdir build_kilobot
    $ cd build_kilobot
    $ cmake -DCMAKE_TOOLCHAIN_FILE=../src/cmake/Kilobot.cmake ../src/
    $ make

The configuration options available for this compilation type are similar
to those of the PC compilation.

Writing your own behaviors
--------------------------

**For kilobots**

Currently, BittyBuzz does not support global installation. Behaviors must thus be
implemented under `src/kilobot/mains` directly.

The C source file should be placed inside `src/kilobot/mains`, whereas the Buzz
script is expected to have the same name and be placer under
`src/kilobot/mains/buzz_scripts`.

_Important:_ After adding new files, be sure to rerun the CMake command again
for CMake to take them into account.

Options
-------

It is possible to specify a custom value for a range of configuration values. Behaviors on low-resource robots often require parameter-tweaking. However, we
recommend against changing these values unless it is necessary.

- `BBZHEAP_SIZE` : Size of the heap (in bytes) [Default: 1088].
- `BBZHEAP_ELEMS_PER_TSEG` : Number of table entries per table segment [Default: 5].
- `BBZSTACK_SIZE` : Size of the VM's stack (in number of objects) [Default: 96].
- `BBZVSTIG_CAP` : Capacity of the virtual stigmergy (in number of entries) [Default: 3].
- `BBZNEIGHBORS_CAP` : Capacity of the neighbors structure (in number of neighbors) [Default: 15, Max: 255]. Used only when `BBZ_XTREME_MEMORY` is ON.
- `BBZINMSG_QUEUE_CAP` : Capacity of the incoming message queue [Default: 10].
- `BBZOUTMSG_QUEUE_CAP` : Capacity of the outgoing message queue [Default: 10].
- `RESERVED_ACTREC_MAX` : Number of table objects reserved on the heap for lambdas'
activation record [Default: 28, Max: 254].
- `BBZLAMPORT_THRESHOLD` : Maximum difference for between two (2) lamport clocks for one to be considered newer than the other [Default: 50].
- `BBZHEAP_GCMARK_DEPTH` : Max recursion depth for the heap's Garbage Collector marking function [Default: 8].
- `BBZMSG_IN_PROC_MAX` : Max number of incomming messages processed per time step [Default: 10].
- `BBZ_XTREME_MEMORY` : Whether to use high memory optimizations [Default: ON for kilobots otherwise OFF, Values: {ON, OFF}].
- `BBZ_USE_PRIORITY_SORT` : Whether to use priority sort on out-messages queue [Default: OFF, Values: {ON, OFF}].
- `BBZ_USE_FLOAT` : Whether to use float type [Default: OFF, Values: {ON, OFF}].
- `BBZ_DISABLE_NEIGHBORS` : Whether to disable usage of neighbors' data structure and messages [Default: OFF, Values: {ON, OFF}].
- `BBZ_DISABLE_VSTIGS` : Whether to disable usage of virtual stigmergies' data structure and messages [Default: OFF, Values: {ON, OFF}].
- `BBZ_DISABLE_SWARMS` : Whether to disable usage of swarms' data structure and messages [Default: OFF, Values: {ON, OFF}].
- `BBZ_DISABLE_MESSAGES` : Whether to disable usage and transfer of any kind of Buzz message [Default: OFF, Values: {ON, OFF}].

For example, for a Buzz program requiring larger stack sizes but less heap allocations, you may run cmake as:

    $ cmake -DBBZHEAP_SIZE=750 -DBBZSTACK_SIZE=200 ../src

Generating documentation
------------------------

One may optionaly create Doxygen-generated documentation of BittyBuzz.
This requires [Doxygen](http://www.stack.nl/~dimitri/doxygen/) and can be done via:

    $ make doc

The documentation will be subsequently available in HTML and LaTeX formats under `<build_dir>/doc`, for example, `build/doc`.
