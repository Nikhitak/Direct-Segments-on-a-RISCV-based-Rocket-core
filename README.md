This repository contains Direct segments built on top of a RISC-V Rocket Core. For more information on Rocket Chip, please consult our [RISC-V Workshop Paper](https://carrv.github.io/2018/papers/CARRV_2018_paper_4.pdf).

## Table of Contents

+ [Quick instructions](#quick) 
+ [What's in this Direct Segments repository?](#what)
+ [How should I use this Direct Segments implementation?](#how)
    + [Using the cycle-accurate Verilator simulation](#emulator)
    + [Using RISC-V Qemu](#fpga)
    + [Using FireSim is a cycle-accurate, FPGA-accelerated Simulation](#vlsi)

## <a name="quick"></a> Quick Instructions

### Checkout The Code

    $ git clone https://github.com/Nikhitak/Direct-Segments-on-a-RISCV-based-Rocket-core.git
    $ cd rocket-chip
    $ git submodule update --init --recursive

### Setting up the RISCV environment variable

To build the rocket-chip repository, you must point the RISCV
environment variable to your riscv-tools installation directory. 

    $ export RISCV=/path/to/riscv/toolchain/installation
    
The riscv-tools repository is already included in 
rocket-chip as a git submodule. You **must** build this version 
of riscv-tools:

    $ cd rocket-chip/riscv-tools
    $ git submodule update --init --recursive
    $ export RISCV=/path/to/install/riscv/toolchain
    $ export MAKEFLAGS="$MAKEFLAGS -jN" # Assuming you have N cores on your host system
    $ ./build.sh

For more information (or if you run into any issues), please consult the
[riscv-tools/README](https://github.com/riscv/riscv-tools/blob/master/README.md).

### Install Necessary Dependencies

You may need to install some additional packages to use this repository.
Rather than list all dependencies here, please see the appropriate section of the READMEs for each of the subprojects:

* [riscv-tools "Ubuntu Packages Needed"](https://github.com/riscv/riscv-tools/blob/priv-1.10/README.md#quickstart)
* [chisel3 "Installation"](https://github.com/ucb-bar/chisel3#installation)

### Building The Project

First, to build the C simulator:

    $ cd rocket-chip/emulator
    $ make

In either case, you can run a set of assembly tests or simple benchmarks
(Assuming you have N cores on your host system):

    $ make -jN run-asm-tests
    $ make -jN run-bmark-tests

To generate FPGA- or VLSI-synthesizable verilog (output will be in `vsim/generated-src`):

    $ cd rocket-chip/vsim
    $ make verilog

### Building RISC-V Linux Kernel image

    $ cd Linux_changed
    $ make all

## <a name="what"></a> What's in the Rocket chip generator repository?


### Direct Segments introduction

In order to enable fast
translation for part of a process’s address space that does not benefit from page-based memory, earlier work introduces Direct
Segments, which map a contiguous region of virtual address space. 
directly to a contiguous region of physical address space.Virtual addresses that do not fall
in this contiguous region are page mapped using the conventional paging technique.


### Hardware support 
We added support for three supervisor level registers:
Supervisor Direct Segment Base (SDSB), Supervisor Direct Segment
Limit (SDSL), and Supervisor Direct Segment offset (SDSO) to store
the base, limit and offset required for Direct Segment lookup. We modified the TLB unit in Rocket to do a direct segement lookup on  a TLB miss.

Changes are in 
    $ rocket-chip/src/main/scala/rocket/TLB.scala
    $ rocket-chip/src/main/scala/rocket/PTW.scala


### Software support 

We modifed the RISC-V Linux kernel to provide two basic functionalities to support Direct Segments. First, the OS provides a Primary
Region as an abstraction for the application to specify which portion can benefit from Direct Segments. Second, the OS reserves a
portion of Physical memory and maps to the primary region by
configuring the Direct Segment registers

Changes are in 
    $ Linux_changed/linux/arch/riscv/include/asm/mmu_context.h
    $ Linux_changed/linux/include/linux/mm_types.h
    $ Linux_changed/linux/fs/exec.c
    $ Linux_changed/linux/mm/mmap.c


## <a name="how"></a> How should I use this Direct Segments implementation?

Before going any further, you must point the RISCV environment variable
to your riscv-tools installation directory. If you do not yet have
riscv-tools installed, follow the directions in the
[riscv-tools/README](https://github.com/riscv/riscv-tools/blob/master/README.md).

    export RISCV=/path/to/install/riscv/toolchain

Otherwise, you will see the following error message while executing any
command in the rocket-chip generator:

    *** Please set environment variable RISCV. Please take a look at README.

### <a name="emulator"></a> 1) Using the high-performance cycle-accurate Verilator

Your next step is to get the Verilator working. Assuming you have N
cores on your host system, do the following:

    $ cd $ROCKETCHIP/emulator
    $ make -jN run

By doing so, the build system will generate C++ code for the
cycle-accurate emulator, compile the emulator, compile all RISC-V
assembly tests and benchmarks, and run both tests and benchmarks on the
emulator. If make finished without any errors, it means that the
generated Rocket chip has passed all assembly tests and benchmarks!

You can also run assembly tests and benchmarks separately:

    $ make -jN run-asm-tests
    $ make -jN run-bmark-tests

To generate vcd waveforms, you can run one of the following commands:

    $ make -jN run-debug
    $ make -jN run-asm-tests-debug
    $ make -jN run-bmark-tests-debug

Or call out individual assembly tests or benchmarks:

    $ make output/rv64ui-p-add.out
    $ make output/rv64ui-p-add.vcd

Now take a look in the emulator/generated-src directory. You will find
Chisel generated verilog code and its associated C++ code generated by
verilator.

    $ ls $ROCKETCHIP/emulator/generated-src
    DefaultConfig.dts
    DefaultConfig.graphml
    DefaultConfig.json
    DefaultConfig.memmap.json
    freechips.rocketchip.system.DefaultConfig
    freechips.rocketchip.system.DefaultConfig.d
    freechips.rocketchip.system.DefaultConfig.fir
    freechips.rocketchip.system.DefaultConfig.v
    $ ls $ROCKETCHIP/emulator/generated-src/freechips.rocketchip.system.DefaultConfig
    VTestHarness__1.cpp
    VTestHarness__2.cpp
    VTestHarness__3.cpp
    ...

Also, output of the executed assembly tests and benchmarks can be found
at emulator/output/\*.out. Each file has a cycle-by-cycle dump of
write-back stage of the pipeline. Here's an excerpt of
emulator/output/rv64ui-p-add.out:

    C0: 483 [1] pc=[00000002138] W[r 3=000000007fff7fff][1] R[r 1=000000007fffffff] R[r 2=ffffffffffff8000] inst=[002081b3] add s1, ra, s0
    C0: 484 [1] pc=[0000000213c] W[r29=000000007fff8000][1] R[r31=ffffffff80007ffe] R[r31=0000000000000005] inst=[7fff8eb7] lui t3, 0x7fff8
    C0: 485 [0] pc=[00000002140] W[r 0=0000000000000000][0] R[r 0=0000000000000000] R[r 0=0000000000000000] inst=[00000000] unknown

The first [1] at cycle 483, core 0, shows that there's a
valid instruction at PC 0x2138 in the writeback stage, which is
0x002081b3 (add s1, ra, s0). The second [1] tells us that the register
file is writing r3 with the corresponding value 0x7fff7fff. When the add
instruction was in the decode stage, the pipeline had read r1 and r2
with the corresponding values next to it. Similarly at cycle 484,
there's a valid instruction (lui instruction) at PC 0x213c in the
writeback stage. At cycle 485, there isn't a valid instruction in the
writeback stage, perhaps, because of a instruction cache miss at PC
0x2140.

### <a name="fpga"></a> 2) Using RISC-V Qemu

You can use Direct Segments prototype added to RISCV-Qemu to quickly test the direct segments fuctionality.  

    $ cd Linux_changed
    $ ./riscv64-softmmu/qemu-system-riscv64 -kernel Linux_changed/work/riscv-pk/bbl -nographic -machine spike_v1.10


### <a name="vlsi"></a> 3) Using FireSim as a cycle-accurate, FPGA-accelerated Simulation

Follow the Firesim set up and intallation document at [Firesim](https://docs.fires.im/en/latest/index.html)
Follow the steps for Running a Single Node Simulation


    $ cp Linux_changed/work/riscv-pk/bbl firesim/sw/firesim-software/images/br-disk-bin

## <a name="attribution"></a> Attribution

If used for research, please cite Direct Segments RISC-V workshop paper:

