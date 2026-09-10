#include "includes.h"
#include "msg.h"
#include "power_api.h"

//本文件仅用于测试部分模块
#if   1
///测试模块编号，部分模块不在这里测试
enum {
    /* TEST_PMU_MODE = 1, */
    /*     TEST_MCPWM, */
    TEST_MCPWM = 1,
    TEST_PWM_LED,
    TEST_TIMER_CAP,
    TEST_TIMER_PWM,
    TEST_IIC,
    TEST_SPI,
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
    case TEST_MCPWM:
        log_info("__________**********************");
        log_info("__________-------test MCPWM-------");
        log_info("__________**********************");
        void mcpwm_test(void);
        mcpwm_test();
        break;
    case TEST_PWM_LED:
        log_info("__________**********************");
        log_info("__________-------test PWM_LED-------");
        log_info("__________**********************");
        void pwm_led_test(void);
        pwm_led_test();
        break;
    case TEST_TIMER_CAP:
        log_info("__________**********************");
        log_info("__________-------test CAP-------");
        log_info("__________**********************");
        void timer_cap_test(void);
        timer_cap_test();
        break;
    case TEST_TIMER_PWM:
        log_info("__________**********************");
        log_info("__________-------test PWM-------");
        log_info("__________**********************");
        void timer_pwm_test(void);
        timer_pwm_test();
        break;
    case TEST_IIC:
        log_info("__________**********************");
        log_info("__________-------test IIC-------");
        log_info("__________**********************");
        void eeprom_test_main();
        eeprom_test_main();
        break;
    case TEST_SPI:
        log_info("__________**********************");
        log_info("__________-------test SPI-------");
        log_info("__________**********************");
        void spi_test_main();
        spi_test_main();
        break;
    case TEST_CLK_SET:
        log_info("__________**********************");
        log_info("__________--------test CLK--------");
        log_info("__________**********************");
        clk_set("sys", sys_clk_freq);
        clk_out(IO_PORTA_03, HSB_CLK_OUT);
        break;
    case TEST_CHARGE_MODE:
        log_info("__________**********************");
        log_info("__________--------test CHARGE--------");
        log_info("__________**********************");
        puts("TEST_CHARGE_MODE init");
        charge_current = charge_get_mA_config();
        charge_set_mA(charge_current);
        break;

    case TEST_PMU_MODE:
        log_info("__________**********************");
        log_info("__________--------test PMU--------");
        log_info("__________**********************");
        break;
    }
}

//测试模块退出(如果需要退出)
static void mode_exit(u8 mode)
{
    switch (mode) {
    case TEST_MCPWM:
        void mcpwm_close(pwm_ch_num_type pwm_ch);
        mcpwm_close(0);
        mcpwm_close(1);
        mcpwm_close(2);
        mcpwm_close(3);
        gpio_disable_fun_output_port(IO_PORTA_06);
        gpio_disable_fun_output_port(IO_PORTA_07);
        gpio_disable_fun_output_port(IO_PORTA_08);
        gpio_disable_fun_output_port(IO_PORTA_03);
        gpio_disable_fun_output_port(IO_PORTA_04);
        gpio_disable_fun_output_port(IO_PORTA_05);
        break;
    case TEST_PWM_LED:
        void close_pwm_led(void);
        close_pwm_led();
        break;
    case TEST_TIMER_CAP:
        void close_timer_cap(JL_TIMER_TypeDef * JL_TIMERx);
        close_timer_cap(JL_TIMER3);
        gpio_disable_fun_input_port(IO_PORTA_03);
        break;
    case TEST_TIMER_PWM:
        void close_timer_pwm(JL_TIMER_TypeDef * JL_TIMERx);
        close_timer_pwm(JL_TIMER2);
        close_timer_pwm(JL_TIMER3);
        gpio_disable_fun_output_port(IO_PORTA_03);
        gpio_disable_fun_output_port(IO_PORTA_04);
        break;
    case TEST_IIC:
        break;
    case TEST_SPI:
        break;
    case TEST_CLK_SET:
        //关闭时钟输出到IO
        JL_PORTA->DIR |= BIT(3);
        break;
    case TEST_CHARGE_MODE:
        puts("TEST_CHARGE_MODE exit");
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
    case TEST_IIC:
        break;
    case TEST_SPI:
        break;
    case TEST_CLK_SET:
        u32 pll_clk = clk_set("sys", sys_clk_freq);
        log_info("clk test %d %d\n", sys_clk_freq, pll_clk);
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
        log_info("charge_current:%d", charge_current);
        charge_set_mA(charge_current);
        break;

    case TEST_PMU_MODE:
        lowpower_mode(low_power_mode++);
        log_info("TEST_PMU_MODE_OUT");
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



void irflt_test(void);
void plcnt_touch_key_test(void);
void adc_test(void);
void uart_loop_test();

//测试消息响应
void sdk_test(void)
{
    /* irflt_test(); */
    /* plcnt_touch_key_test(); */
    /* adc_test(); */
    /* uart_loop_test(); */
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
        puts("mode key");
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
