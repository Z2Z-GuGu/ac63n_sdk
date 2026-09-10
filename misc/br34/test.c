#include "includes.h"
#include "msg.h"
#include "power_api.h"

#if   1
///测试模块，写上对应的名字
enum {
    TEST_MODE_1 = 1,


    TEST_IRFLT,
    TEST_CTMU,
    TEST_IIC,
    TEST_SPI,
    TEST_PWM_LED,
    TEST_TIMER_PWM,
    TEST_UART,
    TEST_CLK_SET,
    TEST_CHARGE_MODE,
    TEST_PMU_MODE,

    TEST_MODE_MAX,
};

static u8 test_mode = 0;
static u8 charge_current;
static u8 low_power_mode = 0;
static u32 sys_clk_freq = 24000000;

//测试模块初始化
static void mode_init(u8 mode)
{
    switch (mode) {
    case TEST_IRFLT:
        break;
    case TEST_CTMU:
        break;
    case TEST_IIC:
        break;
    case TEST_CLK_SET:
        log_info("clk test");
        clk_set("sys", sys_clk_freq);
        clk_dump();
        //输出时钟到PA3
        clk_out(IO_PORTA_03, HSB_CLK_OUT);
        break;
    case TEST_CHARGE_MODE:
        log_info("TEST_CHARGE_MODE init");
        charge_current = charge_get_mA_config();
        charge_set_mA(charge_current);
        break;

    case TEST_PMU_MODE:
        log_info("TEST_PMU_MODE");
        break;
    }
}

//测试模块退出(如果需要退出)
static void mode_exit(u8 mode)
{
    switch (mode) {
    case TEST_IRFLT:
        break;
    case TEST_CTMU:
        break;
    case TEST_IIC:
        break;
    case TEST_CLK_SET:
        //关闭时钟输出到IO
        gpio_set_direction(IO_PORTA_03, 1);
        break;
    case TEST_CHARGE_MODE:
        log_info("TEST_CHARGE_MODE exit");
        charge_set_mA(charge_get_mA_config());
        break;
    case TEST_PMU_MODE:
        break;
    }
}

//测试模块响应key（如有需要）
static void mode_key(u8 mode)
{
    switch (mode) {
    case TEST_IRFLT:
        break;
    case TEST_CTMU:
        break;
    case TEST_IIC:
        break;
    case TEST_CLK_SET:
        log_info("clk test %d", sys_clk_freq);
        clk_set("sys", sys_clk_freq);
        clk_dump();
        sys_clk_freq += 1000000;
        if (sys_clk_freq > 128000000) {
            sys_clk_freq = 24000000;
        }
        break;
    case TEST_CHARGE_MODE:
        charge_current++;
        if (charge_current > CHARGE_mA_220) {
            charge_current = CHARGE_mA_20;
        }
        charge_set_mA(charge_current);

    case TEST_PMU_MODE:
        lowpower_mode(low_power_mode++);
        break;
    }
}


//切换测试模式
static void switch_test_mode()
{
    if (test_mode != 0) {
        mode_exit(test_mode);
    }

    test_mode++;
    if (test_mode == TEST_MODE_MAX) {
        test_mode = 1;
    }

    log_info("switch_test_mode %d", test_mode);
    mode_init(test_mode);
}

//测试消息响应
void sdk_test(void)
{
    int msg = get_msg();
    if (msg == NO_MSG) {
        return;
    }

    switch (msg) {
    case MSG_TEST_IO_KEY1_SHORT:
        switch_test_mode();
        break;
    case MSG_TEST_IO_KEY1_LONG:
        break;
    case MSG_TEST_IO_KEY1_HOLD:
        break;
    case MSG_TEST_IO_KEY1_LONG_HOLD_UP:
        break;



    case MSG_TEST_IO_KEY2_SHORT:
        log_info("mode key");
        mode_key(test_mode);
        break;
    case MSG_TEST_IO_KEY2_LONG:
        break;
    case MSG_TEST_IO_KEY2_HOLD:
        break;
    case MSG_TEST_IO_KEY2_LONG_HOLD_UP:
        break;
    default:
        break;
    }
}

#endif
