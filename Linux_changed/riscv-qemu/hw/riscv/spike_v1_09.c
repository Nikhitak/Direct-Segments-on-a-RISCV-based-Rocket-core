/*
 * QEMU RISC-V Spike Board
 *
 * Author: Sagar Karandikar, sagark@eecs.berkeley.edu
 *
 * This provides a RISC-V Board with the following devices:
 *
 * 0) HTIF Test Pass/Fail Reporting (no syscall proxy)
 * 1) HTIF Console
 *
 * These are created by htif_mm_init below.
 *
 * This board currently uses a hardcoded devicetree that indicates one hart.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "qemu/osdep.h"
#include "qemu/log.h"
#include "qemu/error-report.h"
#include "hw/hw.h"
#include "hw/boards.h"
#include "hw/loader.h"
#include "hw/sysbus.h"
#include "hw/riscv/cpudevs.h"
#include "hw/riscv/riscv_htif.h"
#include "hw/riscv/riscv_hart.h"
#include "hw/riscv/sifive_clint.h"
#include "hw/riscv/spike.h"
#include "sysemu/char.h"
#include "sysemu/arch_init.h"
#include "exec/address-spaces.h"
#include "elf.h"

static const struct MemmapEntry {
    hwaddr base;
    hwaddr size;    
} spike_memmap[] =
{
    [SPIKE_MROM] =     {     0x1000,     0x2000 },
    [SPIKE_CLINT] =    {  0x2000000,    0x10000 },
    [SPIKE_DRAM] =     { 0x80000000,        0x0 },
};

static uint64_t identity_translate(void *opaque, uint64_t addr)
{
    return addr;
}

static uint64_t load_kernel(const char *kernel_filename)
{
    uint64_t kernel_entry, kernel_high;

    if (load_elf(kernel_filename, identity_translate, NULL,
                 &kernel_entry, NULL, &kernel_high,
                 /* little_endian = */ 0, ELF_MACHINE, 1, 0) < 0) {
        error_report("qemu: could not load kernel '%s'\n", kernel_filename);
        exit(1);
    }
    return kernel_entry;
}

static void riscv_spike_board_init(MachineState *machine)
{
    const struct MemmapEntry *memmap = spike_memmap;

    SpikeState *s = g_new0(SpikeState, 1);
    /* const char *cpu_model = machine->cpu_model; */
    /* const char *kernel_cmdline = machine->kernel_cmdline; */
    /* const char *initrd_filename = machine->initrd_filename; */
    MemoryRegion *system_memory = get_system_memory();
    MemoryRegion *main_mem = g_new(MemoryRegion, 1);
    MemoryRegion *boot_rom = g_new(MemoryRegion, 1);

    /* Initialize SOC */
    object_initialize(&s->soc, sizeof(s->soc), TYPE_RISCV_HART_ARRAY);
    object_property_add_child(OBJECT(machine), "soc", OBJECT(&s->soc),
                              &error_abort);
    object_property_set_str(OBJECT(&s->soc), TYPE_RISCV_CPU_IMAFDCSU_PRIV_1_09,
                            "cpu-model", &error_abort);
    object_property_set_int(OBJECT(&s->soc), smp_cpus, "num-harts",
                            &error_abort);
    object_property_set_bool(OBJECT(&s->soc), true, "realized",
                            &error_abort);

    /* register system main memory (actual RAM) */
    memory_region_init_ram(main_mem, NULL, "riscv_spike_board.ram",
                           machine->ram_size, &error_fatal);
    vmstate_register_ram_global(main_mem);
    memory_region_add_subregion(system_memory, DRAM_BASE, main_mem);

    /* boot rom */
    memory_region_init_ram(boot_rom, NULL, "riscv_spike_board.bootrom",
                           0x40000, &error_fatal);
    vmstate_register_ram_global(boot_rom);
    memory_region_add_subregion(system_memory, 0x0, boot_rom);

    if (machine->kernel_filename) {
        load_kernel(machine->kernel_filename);
    }

    uint32_t reset_vec[8] = {
        0x297 + memmap[SPIKE_DRAM].base - memmap[SPIKE_MROM].base, /* lui */
        0x00028067,                   /* jump to DRAM_BASE */
        0x00000000,                   /* reserved */
        memmap[SPIKE_MROM].base + sizeof(reset_vec), /* config string pointer */
        0, 0, 0, 0                    /* trap vector */
    };

    /* part one of config string - before memory size specified */
    const char *config_string_tmpl =
        "platform {\n"
        "  vendor ucb;\n"
        "  arch spike;\n"
        "};\n"
        "rtc {\n"
        "  addr 0x" "40000000" ";\n"
        "};\n"
        "ram {\n"
        "  0 {\n"
        "    addr 0x" "80000000" ";\n"
        "    size 0x" "%016" PRIx64 ";\n"
        "  };\n"
        "};\n"
        "core {\n"
        "  0" " {\n"
        "    " "0 {\n"
        "      isa " "rv64imafd" ";\n"
        "      timecmp 0x" "40000008" ";\n"
        "      ipi 0x" "40001000" ";\n" /* match dummy ipi region above */
        "    };\n"
        "  };\n"
        "};\n";

    /* build config string with supplied memory size */
    size_t config_string_size = strlen(config_string_tmpl) + 16;
    char *config_string = malloc(config_string_size);
    snprintf(config_string, config_string_size,
        config_string_tmpl, (uint64_t)ram_size);
    size_t config_string_len = strlen(config_string);

    /* copy in the reset vector */
    cpu_physical_memory_write(memmap[SPIKE_MROM].base, reset_vec,
        sizeof(reset_vec));

    /* copy in the config string */
    cpu_physical_memory_write(memmap[SPIKE_MROM].base + sizeof(reset_vec),
        config_string, config_string_len);

    /* add memory mapped htif registers at location specified in the symbol
       table of the elf being loaded (thus kernel_filename is passed to the
       init rather than an address) */
    htif_mm_init(system_memory, machine->kernel_filename,
        s->soc.harts[0].env.irq[4], boot_rom,
        &s->soc.harts[0].env, serial_hds[0]);

    /* Core Local Interruptor (timer and IPI) */
    sifive_clint_create(0x40000000, 0x2000, &s->soc, 0x1000, 0x8, 0x0);
}

static int riscv_spike_board_sysbus_device_init(SysBusDevice *sysbusdev)
{
    return 0;
}

static void riscv_spike_board_class_init(ObjectClass *klass, void *data)
{
    SysBusDeviceClass *k = SYS_BUS_DEVICE_CLASS(klass);
    k->init = riscv_spike_board_sysbus_device_init;
}

static const TypeInfo riscv_spike_board_device = {
    .name          = TYPE_RISCV_SPIKE_V1_09_1_BOARD,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(SpikeState),
    .class_init    = riscv_spike_board_class_init,
};

static void riscv_spike_board_machine_init(MachineClass *mc)
{
    mc->desc = "RISC-V Spike Board (Privileged ISA v1.9.1)";
    mc->init = riscv_spike_board_init;
    mc->max_cpus = 1;
    mc->is_default = 1;
}

DEFINE_MACHINE("spike_v1.9", riscv_spike_board_machine_init)

static void riscv_spike_board_register_types(void)
{
    type_register_static(&riscv_spike_board_device);
}

type_init(riscv_spike_board_register_types);
