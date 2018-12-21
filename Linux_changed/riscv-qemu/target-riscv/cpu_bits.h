/* Below taken from Spike's decode.h and encoding.h.
 * Using these directly drastically simplifies updating to new versions of the
 * RISC-V privileged specification */

#define get_field(reg, mask) (((reg) & \
                 (target_ulong)(mask)) / ((mask) & ~((mask) << 1)))
#define set_field(reg, mask, val) (((reg) & ~(target_ulong)(mask)) | \
                 (((target_ulong)(val) * ((mask) & ~((mask) << 1))) & \
                 (target_ulong)(mask)))

#define PGSHIFT 12

#define FP_RD_NE  0
#define FP_RD_0   1
#define FP_RD_DN  2
#define FP_RD_UP  3
#define FP_RD_NMM 4

#define FSR_RD_SHIFT 5
#define FSR_RD   (0x7 << FSR_RD_SHIFT)

#define FPEXC_NX 0x01
#define FPEXC_UF 0x02
#define FPEXC_OF 0x04
#define FPEXC_DZ 0x08
#define FPEXC_NV 0x10

#define FSR_AEXC_SHIFT 0
#define FSR_NVA  (FPEXC_NV << FSR_AEXC_SHIFT)
#define FSR_OFA  (FPEXC_OF << FSR_AEXC_SHIFT)
#define FSR_UFA  (FPEXC_UF << FSR_AEXC_SHIFT)
#define FSR_DZA  (FPEXC_DZ << FSR_AEXC_SHIFT)
#define FSR_NXA  (FPEXC_NX << FSR_AEXC_SHIFT)
#define FSR_AEXC (FSR_NVA | FSR_OFA | FSR_UFA | FSR_DZA | FSR_NXA)

#include "disas/riscv-opc.h"

#define IS_RV_INTERRUPT(ival) (ival & (0x1 << 31))

#define MSTATUS_UIE         0x00000001
#define MSTATUS_SIE         0x00000002
#define MSTATUS_HIE         0x00000004
#define MSTATUS_MIE         0x00000008
#define MSTATUS_UPIE        0x00000010
#define MSTATUS_SPIE        0x00000020
#define MSTATUS_HPIE        0x00000040
#define MSTATUS_MPIE        0x00000080
#define MSTATUS_SPP         0x00000100
#define MSTATUS_HPP         0x00000600
#define MSTATUS_MPP         0x00001800
#define MSTATUS_FS          0x00006000
#define MSTATUS_XS          0x00018000
#define MSTATUS_MPRV        0x00020000
#define MSTATUS_PUM         0x00040000 /* until: priv-1.9.1 */
#define MSTATUS_SUM         0x00040000 /* since: priv-1.10 */
#define MSTATUS_MXR         0x00080000
#define MSTATUS_VM          0x1F000000 /* until: priv-1.9.1 */
#define MSTATUS_TVM         0x00100000 /* since: priv-1.10 */
#define MSTATUS_TW          0x20000000 /* since: priv-1.10 */
#define MSTATUS_TSR         0x40000000 /* since: priv-1.10 */

#define MSTATUS64_UXL       0x0000000300000000
#define MSTATUS64_SXL       0x0000000C00000000

#define MSTATUS32_SD        0x80000000
#define MSTATUS64_SD        0x8000000000000000

#if defined (TARGET_RISCV32)
#define MSTATUS_SD MSTATUS32_SD
#elif defined (TARGET_RISCV64)
#define MSTATUS_SD MSTATUS64_SD
#endif

#define SSTATUS_UIE         0x00000001
#define SSTATUS_SIE         0x00000002
#define SSTATUS_UPIE        0x00000010
#define SSTATUS_SPIE        0x00000020
#define SSTATUS_SPP         0x00000100
#define SSTATUS_FS          0x00006000
#define SSTATUS_XS          0x00018000
#define SSTATUS_PUM         0x00040000 /* until: priv-1.9.1 */
#define SSTATUS_SUM         0x00040000 /* since: priv-1.10 */
#define SSTATUS_MXR         0x00080000

#define SSTATUS32_SD        0x80000000
#define SSTATUS64_SD        0x8000000000000000

#if defined (TARGET_RISCV32)
#define SSTATUS_SD SSTATUS32_SD
#elif defined (TARGET_RISCV64)
#define SSTATUS_SD SSTATUS64_SD
#endif

#define MIP_SSIP            (1 << IRQ_S_SOFT)
#define MIP_HSIP            (1 << IRQ_H_SOFT)
#define MIP_MSIP            (1 << IRQ_M_SOFT)
#define MIP_STIP            (1 << IRQ_S_TIMER)
#define MIP_HTIP            (1 << IRQ_H_TIMER)
#define MIP_MTIP            (1 << IRQ_M_TIMER)
#define MIP_SEIP            (1 << IRQ_S_EXT)
#define MIP_HEIP            (1 << IRQ_H_EXT)
#define MIP_MEIP            (1 << IRQ_M_EXT)

#define SIP_SSIP MIP_SSIP
#define SIP_STIP MIP_STIP
#define SIP_SEIP MIP_SEIP

#define PRV_U 0
#define PRV_S 1
#define PRV_H 2
#define PRV_M 3

/* privileged ISA 1.9.1 VM modes (mstatus.vm) */
#define VM_1_09_MBARE 0  /* until: priv-1.9.1 */
#define VM_1_09_MBB   1  /* until: priv-1.9.1 */
#define VM_1_09_MBBID 2  /* until: priv-1.9.1 */
#define VM_1_09_SV32  8  /* until: priv-1.9.1 */
#define VM_1_09_SV39  9  /* until: priv-1.9.1 */
#define VM_1_09_SV48  10 /* until: priv-1.9.1 */

/* privileged ISA 1.10.0 VM modes (satp.mode) */
#define VM_1_10_MBARE 0  /* since: priv-1.10 */
#define VM_1_10_SV32  1  /* since: priv-1.10 */
#define VM_1_10_SV39  8  /* since: priv-1.10 */
#define VM_1_10_SV48  9  /* since: priv-1.10 */
#define VM_1_10_SV57  10 /* since: priv-1.10 */
#define VM_1_10_SV64  11 /* since: priv-1.10 */

/* privileged ISA interrupt causes */
#define IRQ_U_SOFT      0  /* since: priv-1.10 */
#define IRQ_S_SOFT      1
#define IRQ_H_SOFT      2  /* until: priv-1.9.1 */
#define IRQ_M_SOFT      3  /* until: priv-1.9.1 */
#define IRQ_U_TIMER     4  /* since: priv-1.10 */
#define IRQ_S_TIMER     5
#define IRQ_H_TIMER     6  /* until: priv-1.9.1 */
#define IRQ_M_TIMER     7  /* until: priv-1.9.1 */
#define IRQ_U_EXT       8  /* since: priv-1.10 */
#define IRQ_S_EXT       9
#define IRQ_H_EXT       10 /* until: priv-1.9.1 */
#define IRQ_M_EXT       11 /* until: priv-1.9.1 */
#define IRQ_X_COP       12 /* non-standard */
#define IRQ_X_HOST      13 /* non-standard */

/* Default addresses */
#define DEFAULT_RSTVEC     0x00001000
#define DEFAULT_NMIVEC     0x00001004
#define DEFAULT_MTVEC      0x00001010
#define CONFIG_STRING_ADDR 0x0000100C
#define EXT_IO_BASE        0x40000000
#define DRAM_BASE          0x80000000

/* RV32 satp field masks */
#define SATP32_MODE 0x80000000
#define SATP32_ASID 0x7fc00000
#define SATP32_PPN  0x003fffff

/* RV64 satp field masks */
#define SATP64_MODE 0xF000000000000000
#define SATP64_ASID 0x0FFFF00000000000
#define SATP64_PPN  0x00000FFFFFFFFFFF

#if defined(TARGET_RISCV32)
#define SATP_MODE SATP32_MODE
#define SATP_ASID SATP32_ASID
#define SATP_PPN  SATP32_PPN
#endif
#if defined(TARGET_RISCV64)
#define SATP_MODE SATP64_MODE
#define SATP_ASID SATP64_ASID
#define SATP_PPN  SATP64_PPN
#endif

/* breakpoint control fields */
#define BPCONTROL_X           0x00000001
#define BPCONTROL_W           0x00000002
#define BPCONTROL_R           0x00000004
#define BPCONTROL_U           0x00000008
#define BPCONTROL_S           0x00000010
#define BPCONTROL_H           0x00000020
#define BPCONTROL_M           0x00000040
#define BPCONTROL_BPMATCH     0x00000780
#define BPCONTROL_BPAMASKMAX 0x0F80000000000000
#define BPCONTROL_TDRTYPE    0xF000000000000000

/* page table entry (PTE) fields */
#define PTE_V     0x001 /* Valid */
#define PTE_R     0x002 /* Read */
#define PTE_W     0x004 /* Write */
#define PTE_X     0x008 /* Execute */
#define PTE_U     0x010 /* User */
#define PTE_G     0x020 /* Global */
#define PTE_A     0x040 /* Accessed */
#define PTE_D     0x080 /* Dirty */
#define PTE_SOFT  0x300 /* Reserved for Software */

#define PTE_PPN_SHIFT 10

#define PTE_TABLE(PTE) (((PTE) & (PTE_V | PTE_R | PTE_W | PTE_X)) == PTE_V)
/* end Spike decode.h, encoding.h section */
