/**
 * @file main.c
 * @brief main 入口：调用系统初始化 + LED 闪烁主循环
 *
 * 系统初始化（时钟/端口/关键外设）封装在 components/<平台>/boot/boot.c
 * 的 system_init() 中，本文件只负责调用它并实现业务逻辑。
 */

#include "includes.h"
#include "typedef.h"
#include "boot.h"
#include "gpio.h"

#if defined(AC628N)
#define LED_IO      IO_PORTA_01
#else
#define LED_IO      IO_PORTB_07
#endif

static void delay_ms(u32 ms)
{
    while (ms--) {
        volatile u32 i;
        for (i = 0; i < 2400; i++) {
            delay(10);
        }
    }
}

int main()
{
    system_init();  // boot 库提供：时钟/端口/关键外设初始化

    gpio_direction_output(LED_IO, 1);   // 配置 LED 引脚为输出，初始高电平

    while (1) {
        gpio_toggle(LED_IO);
        delay_ms(100);
    }

    return 0;
}