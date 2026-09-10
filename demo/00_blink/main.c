/**
 * @file main.c
 * @brief AC632N (bd19) LED 闪烁示例程序
 *
 * 入口 user_main() 由 components/bd19/boot/boot.c 中的 main() 完成系统初始化后调用。
 * 程序把 LED 引脚配置为推挽输出，并在主循环中低电平点亮 / 高电平熄灭循环翻转。
 */

#include "includes.h"
#include "msg.h"
#include "typedef.h"
#include "printf.h"

/* LED 引脚（由 demo CMakeLists 传入的平台宏决定）：
 *   AC632N：IO_PORTB_07，低电平点亮，高电平熄灭
 *   AC628N（AC638N）：IO_PORTA_01
 */
#if defined(AC628N)
#define LED_IO      IO_PORTA_01
#else
#define LED_IO      IO_PORTB_07
#endif
#define LED_ON()    gpio_write(LED_IO, 0)
#define LED_OFF()   gpio_write(LED_IO, 1)

#define APP_VERSION_CHECK   1
extern int app_version_check();

/*
 * @brief 简单的忙等待延时（主机时钟 24MHz 下的近似毫秒值）。
 *        delay() 在 asm/cpu.h 中定义为逐条 nop 空转，仅用于翻转 LED。
 */
static void delay_ms(u32 ms)
{
    while (ms--) {
        volatile u32 i;
        for (i = 0; i < 2400; i++) {
            delay(10);
        }
    }
}

/**
 * @brief 用户入口，boot.c main() 初始化完毕后调用
 */
int user_main()
{
#if (APP_VERSION_CHECK)
    app_version_check();
#endif

    /* 把 LED 引脚配置成 GP_A 输出，初始高电平（LED 灭） */
    gpio_direction_output(LED_IO, 1);

    while (1) {
        wdt_clear();

        LED_OFF();
        delay_ms(100);

        LED_ON();
        delay_ms(100);

        /* 空闲等待，降低功耗（等效 LIT） */
        // __asm__ volatile("idle");
    }

    return 0;
}