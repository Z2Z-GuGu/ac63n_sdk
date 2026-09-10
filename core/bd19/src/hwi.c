//*********************************************************************************//
// Module name : HWI                                                               //
// Description : hardware interrupt subroutine                                     //
// By Designer : ZeQuan_liu                                                        //
// Dat changed :                                                                   //
//*********************************************************************************//

#include "typedef.h"
#include "asm/hwi.h"
#include "asm/csfr.h"

extern int ISR_BASE;
#define ISR_ENTRY  (u32)&ISR_BASE


/*static u32 IntCounter[CPU_CORE_NUM];*/

/* volatile int cpu_lock_cnt[CPU_CORE_NUM] = {0}; */
/* volatile int irq_lock_cnt[CPU_CORE_NUM] = {0}; */

/*static void local_irq_enable(void)
{
    int tmp;
    __asm__ volatile("%0 =icfg" : "=r"(tmp));
    tmp |= 1 << 8;
    __asm__ volatile("icfg = %0" :: "r"(tmp));
}

static void local_irq_disable(void)
{
    int tmp;
    __asm__ volatile("%0 = icfg" : "=r"(tmp));
    tmp &= ~(1 << 8);
    __asm__ volatile("icfg = %0" :: "r"(tmp));
}*/

void request_irq(u8 index, u8 priority, void (*handler)(void), u8 cpu_id)
{
    unsigned char icfg_wdat;
    unsigned int icfg_num, icfg_bit;
    unsigned int *israddr = (unsigned int *)ISR_ENTRY;

    israddr[index] = (u32)handler;

    icfg_num =  index / 8;
    icfg_bit = (index % 8) * 4;
    icfg_wdat = ((priority & 0x7) << 1) | BIT(0);
    unsigned int icfg_clr = ~(0xf << icfg_bit);
    unsigned int icfg_set = icfg_wdat << icfg_bit;

    local_irq_disable();

    volatile unsigned int *icfg_ptr = &(j32CPU(0)->ICFG00);
    icfg_ptr[icfg_num] = (icfg_ptr[icfg_num] & icfg_clr) | icfg_set;

    local_irq_enable();
}


void interrupt_init()
{
    int i ;

    local_irq_disable();

    volatile unsigned int *icfg_ptr = &(j32CPU(0)->ICFG00);
    for (i = 0 ; i < 32 ; i++) {
        icfg_ptr[i] = 0;
    }

    local_irq_enable();
}

//------------------------------------------------------//
// interrupt ip
//------------------------------------------------------//
void reg_set_ip(unsigned char index, unsigned char priority)
{
    unsigned char icfg_wdat;
    unsigned int icfg_num, icfg_bit;

    icfg_num =  index / 8;
    icfg_bit = (index % 8) * 4;
    icfg_wdat = (priority & 0x7) << 1;
    unsigned int icfg_clr = ~(0xe << icfg_bit);
    unsigned int icfg_set = icfg_wdat << icfg_bit;

    local_irq_disable();

    volatile unsigned int *icfg_ptr = &(j32CPU(0)->ICFG00);
    icfg_ptr[icfg_num] = (icfg_ptr[icfg_num] & icfg_clr) | icfg_set;

    local_irq_enable();
}

//------------------------------------------------------//
// interrupt ie
//------------------------------------------------------//

void bit_set_ie(unsigned char index)
{
    unsigned int icfg_num, icfg_bit;

    icfg_num =  index / 8;
    icfg_bit = (index % 8) * 4;

    local_irq_disable();

    volatile unsigned int *icfg_ptr = &(j32CPU(0)->ICFG00);
    icfg_ptr[icfg_num] |= BIT(icfg_bit);

    local_irq_enable();
}

void bit_clr_ie(unsigned char index)
{
    unsigned int icfg_num, icfg_bit;

    icfg_num =  index / 8;
    icfg_bit = (index % 8) * 4;

    local_irq_disable();
    volatile unsigned int *icfg_ptr = &(j32CPU(0)->ICFG00);
    icfg_ptr[icfg_num] &= ~BIT(icfg_bit);
    local_irq_enable();
}

void hwi_all_close(void)
{
    int i ;

    for (i = 0; i < 64; i++) {
        bit_clr_ie(i);
    }
}

bool irq_read(u32 index)
{
    unsigned int icfg_num =  index / 8;
    unsigned int icfg_bit = (index % 8) * 4;

    volatile unsigned int *icfg_ptr = &(j32CPU(0)->ICFG00);
    return (icfg_ptr[icfg_num] & BIT(icfg_bit)) ? 1 : 0;
}



void sfc_set_unenc_addr_range(u32 low_addr, u32 high_addr)
{
    //ENC_CON &= ~BIT(4);
    //SFC_UNENC_ADRL = 0x0000000 + (low_addr - SFC_BASE_ADR);
    //SFC_UNENC_ADRH = 0x0000000 + (high_addr - SFC_BASE_ADR);
    //ENC_CON |= BIT(4);
}

void unrequest_irq(u8 index)
{
    bit_clr_ie(index);
}

#ifdef IRQ_TIME_COUNT_EN

#include "printf.h"
static u32 irq_enter_time[80] = {0};
static u32 irq_enter_timestamp[80] = {0};

void irq_handler_enter(int irq)
{
    if (!(T2_CON & BIT(0))) {
        T2_CNT = 0;
        T2_PRD = 120000000 / 10;
        T2_CON = 1;
    }
    irq_enter_timestamp[irq] = T2_CNT;
}

void irq_handler_exit(int irq)
{
    u32 t = T2_CNT;
    if (t > irq_enter_timestamp[irq]) {
        t -= irq_enter_timestamp[irq];
    } else {
        t += T2_PRD - irq_enter_timestamp[irq];
    }

    irq_enter_time[irq] += t;
}

void irq_handler_times_dump()
{
    int i;
    u32 total = 0;

    for (i = 0; i < ARRAY_SIZE(irq_enter_time); i++) {
        if (irq_enter_time[i]) {
            total += irq_enter_time[i];
            log_info("%d: %d\n", i, irq_enter_time[i]);
            irq_enter_time[i] = 0;
        }
    }
    log_info("total: %d\n", total / 120000);
}
#endif

static const u8 usb_upgrade_mode_magic[16] = {
    0x33, 0xEA, 0xBB, 0x74, 0x85, 0x71, 0x8F, 0x32, 0xA7, 0x3C, 0xF0, 0x18, 0x4D, 0x20, 0x08, 0xF7
};

void check_upgrade()
{
    char *magic_str = (char *)ISR_ENTRY;
    magic_str += IRQ_SOFT0_IDX * 4;

    if (memcmp(magic_str, usb_upgrade_mode_magic, sizeof(usb_upgrade_mode_magic)) == 0) {
        memset(magic_str, 0, sizeof(usb_upgrade_mode_magic));
        void usb_upgrade_mode();
        usb_upgrade_mode();
    }
}
void chip_reboot_entry_usb_upgrade_mode()
{
    char *magic_str = (char *)ISR_ENTRY;
    magic_str += IRQ_SOFT0_IDX * 4;
    memcpy(magic_str, usb_upgrade_mode_magic, sizeof(usb_upgrade_mode_magic));
    cpu_reset();
}

extern u32 nvram_list[];
#define NV_RAM_LIST_ADDR  nvram_list
static u8 uboot_uart_upgrade_mode_magic[8] = {
    'u', 'b', 'o', 'o', 't', 0x5a, 's', 't',
};

static u8 uboot_uart_upgrade_succ_magic[8] = {
    'u', 'b', 'o', 'o', 't', 0xa5, 'o', 'k',
};

//须在memory_init();前检测该标志
void check_uboot_uart_upgrade()
{
    if (memcmp((char *)NV_RAM_LIST_ADDR, uboot_uart_upgrade_succ_magic, sizeof(uboot_uart_upgrade_succ_magic)) == 0) {
        memset((char *)NV_RAM_LIST_ADDR, 0, sizeof(uboot_uart_upgrade_succ_magic));
        log_info("uboot uart upgrade succ\n");
    }
}


void hw_mmu_disable(void);
void chip_reboot_entry_uboot_uart_upgrade_mode()
{
    hw_mmu_disable();
    memcpy((char *)NV_RAM_LIST_ADDR, uboot_uart_upgrade_mode_magic, sizeof(uboot_uart_upgrade_mode_magic));
    cpu_reset();
}


