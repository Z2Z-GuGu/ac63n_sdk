#include "includes.h"
#include "resfile.h"
#include "bsp_malloc.h"
#include "msg.h"
#include "power_interface.h"
#include "vm.h"

extern void check_upgrade();
extern int user_main();
extern void board_init();
extern void debug_uart_early_init();
extern void exception_irq_handler();



BOOT_INFO boot_info sec(.boot_info);

void boot_info_init(void *_info)
{
    BOOT_DEVICE_INFO *info = (BOOT_DEVICE_INFO *)_info;

    boot_info.flash_size = info->fs_info->FlashSize;
    boot_info.vm.align = info->fs_info->align;
    boot_info.chip_id = info->chip_id;
    boot_info.trim_value = info->trim_value;

    memcpy(&boot_info.sfc, &info->sfc, sizeof(struct sfc_info));
}

void vm_init_app()
{
    u32 saddr = 0;
    u32 size = 0;
    u8 mode = 0;
    struct vm_info vm;
    memcpy(&vm, &boot_info.vm, sizeof(struct vm_info));
    saddr = vm.vm_saddr;
    size = vm.vm_size;

    if (vm.align == 1) {
        mode = 1;       //page operated
        size = size > (8 * 1024) ? (8 * 1024) : size;
    } else {
        mode = 0;       //sector operated
        size = size > (16 * 1024) ? (16 * 1024) : size;
    }

    log_info("vm_info:addr:0x%x, len:0x%x, mode:0x%x", saddr, size, mode);

    if (vm_init(NULL, saddr, size, mode) != VM_ERR_NONE) {
        ASSERT(0, "vm init err");
    }
    log_info("vm init ok");
}

void maskrom_init()
{

}

/* 系统初始化（供业务 main 调用，配置时钟/端口/关键外设） */
void system_init(void)
{
    wdt_close();

    clk_voltage_mode(CLOCK_MODE_ADAPTIVE, SYSVDD_VOL_SEL_126V);
    // clk_early_init(SYS_CLOCK_INPUT_PLL_RCL, 32768, 24000000);    // 默认不使用内置LRC时钟
    clk_early_init(SYS_CLOCK_INPUT_PLL_BT_OSC, 24000000, 24000000);

    port_init();

    p11_init();     //P11 系统必须提前打开

    resfile_init(); // 必须打开

    request_irq(1, 2, exception_irq_handler, 0);
}

#ifndef JL_DISABLE_BOOT_MAIN
int main()
{
    wdt_init(WDT_4S);
    /*wdt_close();*/
    check_upgrade();
    efuse_init();

    clk_voltage_mode(CLOCK_MODE_ADAPTIVE, SYSVDD_VOL_SEL_126V);
    clk_early_init(SYS_CLOCK_INPUT_PLL_RCL, 32768, 24000000);

    port_init();

    debug_uart_early_init();
    log_info("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    log_info("        boot %s %s", __DATE__, __TIME__);
    clk_dump();
    log_info("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    power_reset_source_dump();

    memory_init();

    //P11 系统必须提前打开
    p11_init();

    resfile_init();

    vm_init_app();

    board_init();

    request_irq(1, 2, exception_irq_handler, 0);

    debug_init();

    timer1_init();

    msg_init();

    user_main();

    return 0;
}
#endif /* JL_DISABLE_BOOT_MAIN */
