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
Kilobot, you need `avr-gcc` and related tools. `avr-gcc` is expected to be
installed under `/usr/lib/avr/` so that CMake may find its header files.

Type these commands:

    $ cd bittybuzz
    $ mkdir build_kilobot
    $ cd build_kilobot
    $ cmake -DCMAKE_TOOLCHAIN_FILE=../src/cmake/Kilobot.cmake ../src/
    $ make

Generating documentation
========================

A good place to get started with BittyBuzz is the source code documentation.
One may optionaly create such Doxygen-generated documentation of BittyBuzz.
This requires [Doxygen](http://www.stack.nl/~dimitri/doxygen/) and can be done via:

    $ make doc

The documentation will be subsequently available in HTML and LaTeX formats
under `<build_dir>/doc`, for example, `build/doc`.

Writing your own behaviors
==========================

For kilobots
------------

Currently, BittyBuzz does not support global installation. Behaviors must thus be
implemented under `src/kilobot/behaviors` directly.

The C source file should be placed inside `src/<robot_name>/behaviors`,
whereas the Buzz script is expected to have the same name and be
placed under `src/<robot_name>/behaviors/buzz_scripts`. You should also place a
Buzz String Table (`.bst`) file (which allows BittyBuzz to generate a
string ID corresponding to each string) next to your Buzz script, which
should contain any string used within the C code and that does not appear
in the Buzz script. Look at existing files if you are unsure.

*EDIT: It is no longer required to put any thing in the `.bst` file. However, the file itself is still required to be there. It will eventualy become optional.*

At this point, you may run `make` inside your kilobot build directory to
generate a HEX file that can be sent to the kilobots. You will find it
under `<build_dir>/kilobot/behaviors/<buzz_script_name>/<buzz_script_name>.hex`.
This file can be sent to the kilobots using the
[KiloGUI](https://www.kilobotics.com/download).

_Important:_ After adding new files, be sure to run `cmake ../src` inside
your kilobot build directory for CMake to take them into account.

For Zooids
----------

See `src/zooids/README.md`


For Crazyflie
----------

See `src/crazyflie/README.md`


Options
-------

It is possible to specify a custom value for a range of configuration values.
Behaviors on low-resource robots often require parameter-tweaking. However, we
recommend against changing these values unless it is necessary.

The table below describes all configurable values and classifies each one by
the likelihood that one will require changing it.

| Option                         | Description                                                | Adjustment likelihood                    | PC   | Kilobot |
|--------------------------------|------------------------------------------------------------|:----------------------------------------:|:----:|:-------:|
| `BBZHEAP_SIZE`                 | Size of the heap (B)                                       | <span style="color:#800">High</span>     | 3264 | 1088    |
| `BBZHEAP_ELEMS_PER_TSEG`       | Num. entries per table segment                             | <span style="color:#880">Moderate</span> | 5    | 5       |
| `BBZSTACK_SIZE`                | Size of the stack (num. objects)                           | <span style="color:#800">High</span>     | 96   | 96      |
| `BBZVSTIG_CAP`                 | Capacity of the `stigmergy` structure (num. entries)       | <span style="color:#800">High</span>     | 3    | 3       |
| `BBZNEIGHBORS_CAP`             | Capacity of the `neighbors` structure (num. neighbors)     | <span style="color:#080">Low</span>      | 15   | 15      |
| `BBZINMSG_QUEUE_CAP`           | Capacity of the incoming message queue (num. msgs)         | <span style="color:#080">Low</span>      | 10   | 10      |
| `BBZOUTMSG_QUEUE_CAP`          | Capacity of the outgoing message queue (num. msgs)         | <span style="color:#080">Low</span>      | 10   | 10      |
| `BBZHEAP_RSV_ACTREC_MAX`       | Num. objects on the heap reserved for activation records   | <span style="color:#880">Moderate</span> | 28   | 28      |
| `BBZLAMPORT_THRESHOLD`         | Length of Lamport clocks' accepting zone                   | <span style="color:#080">Low</span>      | 50   | 50      |
| `BBZHEAP_GCMARK_DEPTH`         | Garbage collector max recursion depth                      | <span style="color:#080">Low</span>      | 8    | 8       |
| `BBZMSG_IN_PROC_MAX`           | Max. num. of incoming messages processed per timestep      | <span style="color:#880">Moderate</span> | 10   | 10      |
| `BBZNEIGHBORS_CLR_PERIOD`      | Num. timesteps between neighbor clears                     | <span style="color:#080">Low</span>      | 10   | 10      |
| `BBZNEIGHBORS_MARK_TIME`       | Num. timesteps before clear we spend marking neighbors     | <span style="color:#080">Low</span>      | 4    | 4       |
| `BBZ_XTREME_MEMORY`            | Whether to reduce RAM at the cost of Flash                 | <span style="color:#880">Moderate</span> | OFF  | ON      |
| `BBZ_USE_PRIORITY_SORT`        | Whether to use priority sort on outgoing message queue     | <span style="color:#080">Low</span>      | OFF  | OFF     |
| `BBZ_USE_FLOAT`                | Whether to use float type                                  | <span style="color:#080">Low</span>      | OFF  | OFF     |
| `BBZ_DISABLE_NEIGHBORS`        | Whether to disable the `neighbors` structure               | <span style="color:#800">High</span>     | OFF  | OFF     |
| `BBZ_DISABLE_VSTIGS`           | Whether to disable the `stigmergy` structure               | <span style="color:#800">High</span>     | OFF  | OFF     |
| `BBZ_DISABLE_SWARMS`           | Whether to disable the `swarms` structure                  | <span style="color:#800">High</span>     | OFF  | OFF     |
| `BBZ_DISABLE_MESSAGES`         | Whether to disable Buzz messages                           | <span style="color:#880">Moderate</span> | OFF  | OFF     |
| `BBZ_DISABLE_PY_BEHAV`         | Whether to disable Python behaviors of closures            | <span style="color:#080">Low</span>      | OFF  | OFF     |
| `BBZ_NEIGHBORS_USE_FLOATS`     | Whether to use floats for the neighbor's range and bearing | <span style="color:#880">Moderate</span> | ON   | OFF     |
| `BBZ_ENABLE_FLOATS_OPERATIONS` | Whether to enable floats operations                         | <span style="color:#880></span>          | ON   | OFF     |

For example, for a Buzz program requiring larger stack sizes but less heap allocations, you may run cmake as:

    $ cmake -DBBZHEAP_SIZE=750 -DBBZSTACK_SIZE=200 ../src
