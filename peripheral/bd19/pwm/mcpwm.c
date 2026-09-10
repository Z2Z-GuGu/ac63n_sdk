#include "mcpwm.h"


#define MCPWM_CLK   clk_get("mcpwm")


/**
 * @brief 通过index获取mctimer的寄存器地址
 * @param index mctimer的序号
 * @return mctimer寄存器地址
 */
PWM_TIMER_REG *get_pwm_timer_reg(pwm_ch_num_type index)
{
    PWM_TIMER_REG *reg = NULL;
    switch (index) {
    case pwm_ch0:
        reg = (PWM_TIMER_REG *)(&(JL_MCPWM->TMR0_CON));
        break;
    case pwm_ch1:
        reg = (PWM_TIMER_REG *)(&(JL_MCPWM->TMR1_CON));
        break;
    case pwm_ch2:
        reg = (PWM_TIMER_REG *)(&(JL_MCPWM->TMR2_CON));
        break;
    case pwm_ch3:
        reg = (PWM_TIMER_REG *)(&(JL_MCPWM->TMR3_CON));
        break;
    default:
        break;
    }

    return reg;
}

/**
 * @brief 通过index获取mcpwm的寄存器地址
 * @param index mcpwm的序号
 * @return mcpwm寄存器地址
 */
PWM_CH_REG *get_pwm_ch_reg(pwm_ch_num_type index)
{
    PWM_CH_REG *reg = NULL;
    switch (index) {
    case pwm_ch0:
        reg = (PWM_CH_REG *)(&(JL_MCPWM->CH0_CON0));
        break;
    case pwm_ch1:
        reg = (PWM_CH_REG *)(&(JL_MCPWM->CH1_CON0));
        break;
    case pwm_ch2:
        reg = (PWM_CH_REG *)(&(JL_MCPWM->CH2_CON0));
        break;
    case pwm_ch3:
        reg = (PWM_CH_REG *)(&(JL_MCPWM->CH3_CON0));
        break;
    default:
        break;
    }

    return reg;
}

/**
 * @brief 更改MCPWM的频率
 * @parm ch 通道号
 * @parm frequency 频率
 */
void mcpwm_set_frequency(pwm_ch_num_type ch, u32 frequency)
{
    PWM_TIMER_REG *reg = get_pwm_timer_reg(ch);
    if (reg == NULL) {
        return;
    }
    u32 i = 0;
    u32 mcpwm_div_clk = 0;
    u32 mcpwm_tmr_pr = 0;
    u32 mcpwm_fre_min = 0;
    reg->tmr_con = 0;
    reg->tmr_cnt = 0;
    reg->tmr_pr = 0;
    u32 clk = MCPWM_CLK;
    for (i = 0; i < 16; i++) {
        mcpwm_fre_min = clk / (65536 * (1 << i));
        if ((frequency >= mcpwm_fre_min) || (i == 15)) {
            break;
        }
    }
    reg->tmr_con |= (i << 3); //div 2^i
    mcpwm_div_clk = clk / (1 << i);
    if (frequency == 0) {
        mcpwm_tmr_pr = 0;
    } else {
        mcpwm_tmr_pr = (mcpwm_div_clk / frequency) - 1;
    }
    reg->tmr_pr = mcpwm_tmr_pr;
    reg->tmr_con |= 0b01;
}

static u32 old_mcpwm_clk = 0;
static void clock_critical_enter(void)
{
    old_mcpwm_clk = clk_get("mcpwm");
}
static void clock_critical_exit(void)
{
    u32 new_mcpwm_clk = clk_get("mcpwm");
    if (new_mcpwm_clk == old_mcpwm_clk) {
        return;
    }
    PWM_CH_REG *pwm_reg = NULL;
    PWM_TIMER_REG *timer_reg = NULL;
    for (u8 ch = 0; ch < pwm_ch_max; ch++) {
        if (JL_MCPWM->MCPWM_CON0 & BIT(ch + 8)) {
            pwm_reg = get_pwm_ch_reg(ch);
            timer_reg = get_pwm_timer_reg(ch);
            if (new_mcpwm_clk > old_mcpwm_clk) {
                timer_reg->tmr_pr = timer_reg->tmr_pr * (new_mcpwm_clk / old_mcpwm_clk);
                pwm_reg->ch_cmpl = pwm_reg->ch_cmpl * (new_mcpwm_clk / old_mcpwm_clk);
            } else {
                timer_reg->tmr_pr = timer_reg->tmr_pr / (old_mcpwm_clk / new_mcpwm_clk);
                pwm_reg->ch_cmpl = pwm_reg->ch_cmpl / (old_mcpwm_clk / new_mcpwm_clk);
            }
            pwm_reg->ch_cmph = pwm_reg->ch_cmpl;
        }
    }
}
CLOCK_CRITICAL_HANDLE_REG(mcpwm, clock_critical_enter, clock_critical_exit)

/*
 * @brief 设置一个通道的占空比
 * @parm pwm_ch_num 通道号：pwm_ch0，pwm_ch1，pwm_ch2
 * @parm duty 占空比：0 ~ 10000 对应 0% ~ 100%
 */
void mcpwm_set_duty(pwm_ch_num_type pwm_ch, u16 duty)
{
    PWM_TIMER_REG *timer_reg = get_pwm_timer_reg(pwm_ch);
    PWM_CH_REG *pwm_reg = get_pwm_ch_reg(pwm_ch);

    if (pwm_reg && timer_reg) {
        pwm_reg->ch_cmpl = timer_reg->tmr_pr * duty / 10000;
        pwm_reg->ch_cmph = pwm_reg->ch_cmpl;
        timer_reg->tmr_cnt = 0;
        timer_reg->tmr_con |= 0b01;
        if (duty == 10000) {
            timer_reg->tmr_cnt = 0;
            timer_reg->tmr_con &= ~(0b11);
        } else if (duty == 0) {
            timer_reg->tmr_cnt = pwm_reg->ch_cmpl;
            timer_reg->tmr_con &= ~(0b11);
        }
    }
}

/*
 * @brief 打开或者关闭一个时基
 * @parm pwm_ch_num 通道号：pwm_ch0，pwm_ch1，pwm_ch2
 * @parm enable 1：打开  0：关闭
 */
void mctimer_ch_open_or_close(pwm_ch_num_type pwm_ch, u8 enable)
{
    if (pwm_ch > pwm_ch_max) {
        return;
    }
    if (enable) {
        JL_MCPWM->MCPWM_CON0 |= BIT(pwm_ch + 8); //TnEN
    } else {
        JL_MCPWM->MCPWM_CON0 &= (~BIT(pwm_ch + 8)); //TnDIS
    }
}


/*
 * @brief 打开或者关闭一个通道
 * @parm pwm_ch_num 通道号：pwm_ch0，pwm_ch1，pwm_ch2
 * @parm enable 1：打开  0：关闭
 */
void mcpwm_ch_open_or_close(pwm_ch_num_type pwm_ch, u8 enable)
{
    if (pwm_ch >= pwm_ch_max) {
        return;
    }

    if (enable) {
        JL_MCPWM->MCPWM_CON0 |= BIT(pwm_ch); //PWMnEN
    } else {
        JL_MCPWM->MCPWM_CON0 &= (~BIT(pwm_ch)); //PWMnDIS
    }
}

/*
 * @brief 关闭MCPWM模块
 * @parm pwm_ch_num 通道号：pwm_ch0，pwm_ch1，pwm_ch2
 */
void mcpwm_open(pwm_ch_num_type pwm_ch)
{
    if (pwm_ch >= pwm_ch_max) {
        return;
    }
    PWM_CH_REG *pwm_reg = get_pwm_ch_reg(pwm_ch);
    pwm_reg->ch_con1 &= ~(0b111 << 8);
    pwm_reg->ch_con1 |= (pwm_ch << 8); //sel mctmr

    mcpwm_ch_open_or_close(pwm_ch, 1);
    mctimer_ch_open_or_close(pwm_ch, 1);
}


/*
 * @brief 关闭MCPWM模块
 * @parm pwm_ch_num 通道号：pwm_ch0，pwm_ch1，pwm_ch2
 */
void mcpwm_close(pwm_ch_num_type pwm_ch)
{
    mctimer_ch_open_or_close(pwm_ch, 0);
    mcpwm_ch_open_or_close(pwm_ch, 0);
}


void log_pwm_info(pwm_ch_num_type pwm_ch)
{
    PWM_CH_REG *pwm_reg = get_pwm_ch_reg(pwm_ch);
    PWM_TIMER_REG *timer_reg = get_pwm_timer_reg(pwm_ch);
    log_info("tmr%d con0 = 0x%x\n", pwm_ch, timer_reg->tmr_con);
    log_info("tmr%d pr = 0x%x\n", pwm_ch, timer_reg->tmr_pr);
    log_info("pwm ch%d_con0 = 0x%x\n", pwm_ch, pwm_reg->ch_con0);
    log_info("pwm ch%d_con1 = 0x%x\n", pwm_ch, pwm_reg->ch_con1);
    log_info("pwm ch%d_cmph = 0x%x, pwm ch%d_cmpl = 0x%x\n", pwm_ch, pwm_reg->ch_cmph, pwm_ch, pwm_reg->ch_cmpl);
    log_info("MCPWM_CON0 = 0x%x\n", JL_MCPWM->MCPWM_CON0);
    log_info("mcpwm clk = %d\n", MCPWM_CLK);
}

/*
 * @brief  pwm初始化函数
 * @param  arg 初始化的参数结构体地址 ： struct pwm_platform_data *
*/
void mcpwm_init(struct pwm_platform_data *arg)
{
    PWM_CH_REG *pwm_reg = get_pwm_ch_reg(arg->pwm_ch_num);
    if (pwm_reg == NULL) {
        return;
    }

    //set mctimer frequency
    mcpwm_set_frequency(arg->pwm_ch_num, arg->frequency);

    pwm_reg->ch_con0 = 0;


    if (arg->complementary_en) {            //是否互补
        pwm_reg->ch_con0 &= ~(BIT(5) | BIT(4));
        pwm_reg->ch_con0 |= BIT(5); //L_INV
    } else {
        pwm_reg->ch_con0 &= ~(BIT(5) | BIT(4));
    }

    mcpwm_open(arg->pwm_ch_num); 	 //mcpwm enable
    //set duty
    mcpwm_set_duty(arg->pwm_ch_num, arg->duty);

    //H:
    if (arg->h_pin < IO_MAX_NUM) {      //任意引脚
        pwm_reg->ch_con0 |= BIT(2);     //H_EN
        gpio_set_fun_output_port(arg->h_pin, FO_MCPWM_CH0H + 2 * arg->pwm_ch_num, 0, 1);
        gpio_set_direction(arg->h_pin, 0); //DIR output
    }
    //L:
    if (arg->l_pin < IO_MAX_NUM) {      //任意引脚
        pwm_reg->ch_con0 |= BIT(3);     //L_EN
        gpio_set_fun_output_port(arg->l_pin, FO_MCPWM_CH0L + 2 * arg->pwm_ch_num, 0, 1);
        gpio_set_direction(arg->l_pin, 0); //DIR output
    }

    log_pwm_info(arg->pwm_ch_num);
}


/************************************************ 以下是SDK提供的参考示例 ***************************************************/

void mcpwm_test(void)
{
#define PWM_CH0_ENABLE 		1
#define PWM_CH1_ENABLE 		1
#define PWM_CH2_ENABLE 		1
#define PWM_CH3_ENABLE 		1

    struct pwm_platform_data pwm_p_data;

#if PWM_CH0_ENABLE
    memset((u8 *)&pwm_p_data, 0, sizeof(struct pwm_platform_data));
    pwm_p_data.pwm_ch_num = pwm_ch0;                        //通道0
    pwm_p_data.frequency = 1000;                     		//1KHz
    pwm_p_data.duty = 2000;                                 //占空比20%
    pwm_p_data.h_pin = IO_PORTA_06;                         //任意IO
    pwm_p_data.l_pin = -1;                         			//不用则填 -1
    mcpwm_init(&pwm_p_data);
#endif

#if PWM_CH1_ENABLE
    memset((u8 *)&pwm_p_data, 0, sizeof(struct pwm_platform_data));
    pwm_p_data.pwm_ch_num = pwm_ch1;                        //通道1
    pwm_p_data.frequency = 10000;                     		//10KHz
    pwm_p_data.duty = 5000;                                 //占空比50%
    pwm_p_data.h_pin = IO_PORTA_07;                         //任意IO
    pwm_p_data.l_pin = IO_PORTA_08;                         //任意IO
    pwm_p_data.complementary_en = 1;                        //两个引脚的波形, 1: 互补, 0: 同步;
    mcpwm_init(&pwm_p_data);
#endif

#if PWM_CH2_ENABLE
    memset((u8 *)&pwm_p_data, 0, sizeof(struct pwm_platform_data));
    pwm_p_data.pwm_ch_num = pwm_ch2;                        //通道2
    pwm_p_data.frequency = 2000;                     		//2KHz
    pwm_p_data.duty = 7500;                                 //占空比75%
    pwm_p_data.h_pin = -1;                         			//不用则填 -1
    pwm_p_data.l_pin = IO_PORTA_03;                         //任意IO
    mcpwm_init(&pwm_p_data);
#endif

#if PWM_CH3_ENABLE
    memset((u8 *)&pwm_p_data, 0, sizeof(struct pwm_platform_data));
    pwm_p_data.pwm_ch_num = pwm_ch3;                        //通道3
    pwm_p_data.frequency = 2000;                     		//2KHz
    pwm_p_data.duty = 5000;                                 //占空比50%
    pwm_p_data.h_pin = IO_PORTA_04;                         //任意IO
    pwm_p_data.l_pin = IO_PORTA_05;                         //任意IO
    pwm_p_data.complementary_en = 0;                        //两个引脚的波形, 1: 互补, 0: 同步;
    mcpwm_init(&pwm_p_data);
#endif

#if 0
    extern void wdt_clear();
    while (1) {
        wdt_clear();
    }
#endif
}



