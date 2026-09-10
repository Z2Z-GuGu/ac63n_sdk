#ifndef __MCPWM_H__
#define __MCPWM_H__

#include "typedef.h"
#include "includes.h"

/* pwm通道选择 */
typedef enum {
    pwm_ch0,
    pwm_ch1,
    pwm_ch2,
    pwm_ch3,
    pwm_ch_max,
} pwm_ch_num_type;

/* MCPWM TIMER寄存器 */
typedef struct _pwm_timer_reg {
    volatile u32 tmr_con;
    volatile u32 tmr_cnt;
    volatile u32 tmr_pr;
} PWM_TIMER_REG;

/* MCPWM通道寄存器 */
typedef struct _pwm_ch_reg {
    volatile u32 ch_con0;
    volatile u32 ch_con1;
    volatile u32 ch_cmph;
    volatile u32 ch_cmpl;
} PWM_CH_REG;

/* 初始化要用的参数结构体 */
struct pwm_platform_data {
    pwm_ch_num_type pwm_ch_num;                         ///< 选择pwm通道号
    u32 frequency;                               		///< 初始共同频率，CH0, CH, CH2,,,,,,
    u16 duty;                                           ///< 初始占空比，0~10000 对应 0%~100% 。每个通道可以有不同的占空比。互补模式的占空比体现在高引脚的波形上。
    u8 h_pin;                                           ///< 一个通道的H引脚。
    u8 l_pin;                                           ///< 一个通道的L引脚，不需要则填-1
    u8 complementary_en;                                ///< 该通道的两个引脚输出的波形。0: 同步， 1: 互补，互补波形的占空比体现在H引脚上
};


/**
 * @brief 更改MCPWM的频率
 * @parm ch 通道号
 * @parm frequency 频率
 */
void mcpwm_set_frequency(pwm_ch_num_type ch, u32 frequency);

/**
 * @brief 设置一个通道的占空比
 * @parm pwm_ch_num 通道号：pwm_ch0，pwm_ch1，pwm_ch2
 * @parm duty 占空比：0 ~ 10000 对应 0% ~ 100%
 */
void mcpwm_set_duty(pwm_ch_num_type pwm_ch, u16 duty);

/*
 * @brief 打开或者关闭一个时基
 * @parm pwm_ch_num 通道号：pwm_ch0，pwm_ch1，pwm_ch2
 * @parm enable 1：打开  0：关闭
 */
void mctimer_ch_open_or_close(pwm_ch_num_type pwm_ch, u8 enable);

/**
 * @brief 打开或者关闭一个通道
 * @parm pwm_ch_num 通道号：pwm_ch0，pwm_ch1，pwm_ch2
 * @parm enable 1：打开  0：关闭
 */
void mcpwm_ch_open_or_close(pwm_ch_num_type pwm_ch, u8 enable);

/**
 * @brief 关闭MCPWM模块
 * @parm pwm_ch_num 通道号：pwm_ch0，pwm_ch1，pwm_ch2
 */
void mcpwm_open(pwm_ch_num_type pwm_ch);

/**
 * @brief 关闭MCPWM模块
 * @parm pwm_ch_num 通道号：pwm_ch0，pwm_ch1，pwm_ch2
 */
void mcpwm_close(pwm_ch_num_type pwm_ch);

/**
 * @brief  pwm初始化函数
 * @param  arg 初始化的参数结构体地址 ： struct pwm_platform_data *
*/
void mcpwm_init(struct pwm_platform_data *arg);


void mcpwm_test(void);



#endif



