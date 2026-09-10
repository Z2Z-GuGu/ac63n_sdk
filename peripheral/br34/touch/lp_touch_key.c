#include "p11.h"
#include "p33.h"
#include "lp_touch_key_api.h"
#include "lp_touch_key_hw.h"
#include "gpio.h"
#include "clock.h"

#define LOG_TAG_CONST       LP_KEY
#define LOG_TAG             "[LP_KEY]"
/* #define LOG_ERROR_ENABLE */
/* #define LOG_DEBUG_ENABLE */
#define LOG_INFO_ENABLE
#define LOG_DUMP_ENABLE
#define LOG_CLI_ENABLE
#include "debug_log.h"

#define CTMU_CH0_MODULE_DEBUG 	0
#define CTMU_CH1_MODULE_DEBUG 	0
#define TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE 	0
#define TCFG_EARTCH_EVENT_HANDLE_ENABLE		0
#define LP_TOUCH_KEY_TIMER_MAGIC_NUM 		0xFFFF

#define LPCTM_TOUCH_CLK	0 //0: 32k, 1: 250k , 2 : pll

struct ctmu_key {
    u8 init;
    u8 ch_init;
    u8 click_cnt;
    u8 last_key;
    u8 last_ear_in_state;
    u8 ch0_msg_lock;
    u8 softoff_mode;
    u16 ch0_msg_lock_timer;
    u16 short_timer;
    u16 long_timer;
    u16 ear_in_timer;
    u32 lrc_hz;
    u8 ch1_inear_ok;
    u16 ch1_trim_value;
    u16 ch1_trim_flag;
    const struct lp_touch_key_platform_data *config;
};

enum ctmu_key_event {
    CTMU_KEY_NULL,
    CTMU_KEY_SHORT_CLICK,
    CTMU_KEY_LONG_CLICK,
    CTMU_KEY_HOLD_CLICK,
};

enum ch1_event_list {
    CH1_EAR_IN,
    CH1_EAR_OUT,
};

enum LP_TOUCH_SOFTOFF_MODE {
    LP_TOUCH_SOFTOFF_MODE_LEGACY  = 0, //普通关机
    LP_TOUCH_SOFTOFF_MODE_ADVANCE = 1, //带触摸关机
};

static struct ctmu_key _ctmu_key = {
    .click_cnt = 0,
    .last_ear_in_state = CH1_EAR_OUT,
    .short_timer = LP_TOUCH_KEY_TIMER_MAGIC_NUM,
    .long_timer = LP_TOUCH_KEY_TIMER_MAGIC_NUM,
    .ear_in_timer = LP_TOUCH_KEY_TIMER_MAGIC_NUM,
    .last_key = CTMU_KEY_NULL,
};

struct ch_adjust_table {
    u16 cfg0;
    u16 cfg1;
    u16 cfg2;
};

//cap(电容)检测灵敏度级数配置
//模具厚度, 触摸片面积有关
//模具越厚, 触摸片面积越大, 触摸时电容变化量
//cap检测灵敏度级数配置建议从级数0开始调, 选取合适的灵敏度;
const static struct ch_adjust_table ch0_sensitivity_table[] = {
    /*  cfg0 		cfg1 		cfg2 */
    {20, 		30, 		550}, //cap检测灵敏度级数0
    {15, 		20, 		270}, //cap检测灵敏度级数1
    {15, 		20, 		240}, //cap检测灵敏度级数2
    {15, 		20, 		210}, //cap检测灵敏度级数3
    {15, 		20, 		180}, //cap检测灵敏度级数4
    {15, 		20, 		150}, //cap检测灵敏度级数5
    {15, 		20, 		120}, //cap检测灵敏度级数6
    {15, 		20, 		 90}, //cap检测灵敏度级数7
    {10, 		15, 		 60}, //cap检测灵敏度级数8
    {10, 		15, 		 30}, //cap检测灵敏度级数9
};

int eartch_event_deal_init(void);

#define __this 		(&_ctmu_key)

static volatile u8 is_lpkey_active = 0;

//init io HZ
static void ctmu_port_init(u8 port)
{
    gpio_set_die(port, 0);
    gpio_set_dieh(port, 0);
    gpio_set_direction(port, 1);
    gpio_set_pull_down(port, 0);
    gpio_set_pull_up(port, 0);
}

static void lp_touch_key_send_cmd(enum CTMU_M2P_CMD cmd)
{
    M2P_CTMU_CMD = cmd;
    P11_M2P_INT_SET = BIT(M2P_CTMU_INDEX);
}
static u32 lp_touch_key_get_lrc_hz(void)
{
    ASSERT(__this->lrc_hz, "lrc_hz is ZERO");
    return __this->lrc_hz;
}

//=========================================//
//模块配置顺序: Select ctmu clk --> Enable --> Reset --> 配置参数 --> Run
static void __lpctmu_reset(u8 para_reset)
{
    if (LPCTM_TOUCH_CLK == 1) {
        LP_CTMU_CLK_SEL(LP_CTMU_CLK_SEL_250K);
    } else if (LPCTM_TOUCH_CLK == 2) {
        LP_CTMU_CLK_SEL(LP_CTMU_CLK_SEL_P11_SYS_CLK); //sel sys_clk 12M, 加快配置速度
    } else {
        LP_CTMU_CLK_SEL(LP_CTMU_CLK_SEL_32K);
    }
    LP_CTMU_MODULE_EN(1); 			//lctm en

    LP_CTMU_RUN(0); //lctm run (内部状态机复位)

    if (para_reset) {
        LP_CTMU_CFG_RESET(0); //lctm 配置参数reset
    }
    delay(100);
    LP_CTMU_CFG_RESET(1);           //lctm reset
}

#define IO_PORT_RESET_PORT_LDOIN   	LINDT_IN//IO编号
#define IO_RESET_PORTB_01 			11

void lp_touch_key_init(const struct lp_touch_key_platform_data *config)
{
    log_info("%s >>>>", __func__);

    ASSERT(config && (__this->init == 0));
    __this->config = config;
    if (LPCTM_TOUCH_CLK == 1) {
        __this->lrc_hz = 250 * 1000;
    } else if (LPCTM_TOUCH_CLK == 2) {
        __this->lrc_hz = 16 * 1000000;
    } else {
        __this->lrc_hz = 32 * 1000;
    }

    M2P_CTMU_MSG = 0;
    //长按复位检测
    u8 pinr_io;
    if (P33_CON_GET(P3_PINR_CON) & BIT(0)) {
        pinr_io = P33_CON_GET(P3_PORT_SEL0);
        if (pinr_io == IO_RESET_PORTB_01) {
            P33_CON_SET(P3_PINR_CON, 0, 1, 0);
            p33_tx_1byte(P3_PORT_SEL0, IO_PORT_RESET_PORT_LDOIN);
            P33_CON_SET(P3_PINR_CON, 2, 1, 1);
            P33_CON_SET(P3_PINR_CON, 0, 1, 1);
            log_info("reset pin change: old: %d, new: %d, P33_PINR_CON = 0x%x", pinr_io, P33_CON_GET(P3_PORT_SEL0), P33_CON_GET(P3_PINR_CON));
        }
    }

    u8 ch0_sensity = __this->config->ch0.sensitivity;

//==============================================================//
//                      CH0 初始化                              //
//==============================================================//
    __lpctmu_reset(1); //模块复位

    LP_CTMU_SWITCH_STABLE_TIME_SET();

    //CTMU 时基配置:
    u16 time_prd = 0;
    if (__this->lrc_hz) {
        time_prd = (__this->lrc_hz * CTMU_TIME_BASE) / 1000 - 1;
    } else {
        time_prd = CFG_M2P_CTMU_BASE_TIME_PRD;
    }

    LP_CTMU_TIMER_BASE_CONFIG(time_prd);
    log_info("LRC_HZ = %d, time_prd = %d", __this->lrc_hz, time_prd);

    if (__this->config->ch0.enable) {
        //PB1和PB2通道互换IO:
        if (__this->config->ch0.port == IO_PORTB_02) {
            LP_CTMU_IO_INV(1);
        } else {
            LP_CTMU_IO_INV(0);
            ASSERT(__this->config->ch0.port == IO_PORTB_01);
        }

        M2P_CTMU_MSG |= CTMU_INIT_CH0_ENABLE;

        ctmu_port_init(__this->config->ch0.port);

        //模拟参数配置:
        //LP_CTMU_CH0_CUR_SEL(CUR_SEL_1040UA);
        LP_CTMU_CH0_CUR_SEL(CUR_SEL_560UA);
        LP_CTMU_CH0_VH_SEL(VHSEL_075V);
        LP_CTMU_CH0_VL_SEL(VLSEL_030V);

        //通用配置:
        LP_CTMU_CH0_ENABLE(1); 			//通道0使能
        LP_CTMU_CH0_FILTER_LEVEL(1); 	//通道0滤波级数
        LP_CTMU_CH0_FILTER_EN(1); 		//通道0滤波使能
        LP_CTMU_CH0_EDGE_EN(1);  		//通道0边沿检测使能
        LP_CTMU_CH0_EDGE_TEMP_EN(1); 	//fixed
        LP_CTMU_CH0_FIRST_RISE_MASK(0); //屏蔽第一个上升沿

        //上升沿中断:
        LP_CTMU_CH0_FALL_PENDING_CLR();
        LP_CTMU_CH0_FALL_PENDING_IE(1);
        LP_CTMU_CH0_FALL_PENDING_SEL(0);

        //下升沿中断:
        LP_CTMU_CH0_RISE_PENDING_CLR();
        LP_CTMU_CH0_RISE_PENDING_IE(1);
        LP_CTMU_CH0_RISE_PENDING_SEL(0);

        //多击模式:
        LP_CTMU_CH0_SHORT_KEY_MODE(1);

#if CTMU_CH0_MODULE_DEBUG
        LP_CTMU_CH0_RES_PENDING_IE(1);
        M2P_CTMU_MSG |= CTMU_INIT_CH0_DEBUG;
#endif /* #if CTMU_CH0_MODULE_DEBUG */

        LP_CTMU_CH0_SHORT_KEY_TRIGGER(0);
        LP_CTMU_CH0_SHORT_PENDING_CLR();
        LP_CTMU_CH0_SHORT_PENDING_IE(0);
        LP_CTMU_CH0_LONG_PENDING_CLR();
        LP_CTMU_CH0_LONG_PENDING_IE(1);

        LP_CTMU_CH0_HOLD_PENDING_CLR();
        LP_CTMU_CH0_HOLD_PENDING_IE(1);
        LP_CTMU_CH0_HOLD_EN(1);
        LP_CTMU_CH0_KEY_MODE_ENABLE(1);

        //快速 & 低速扫描周期
        LP_CTMU_CH0_HIGH_SPEED_PRD(CFG_CTMU_CH0_HS_PERIOD_VALUE);
        LP_CTMU_CH0_LOW_SPEED_PRD(CFG_CTMU_CH0_LS_PERIOD_VALUE);

        //采样窗口时间
        LP_CTMU_CH0_DET_TIME(CFG_CTMU_CH0_WINDOW_TIME_VALUE);

        ASSERT(ch0_sensity < ARRAY_SIZE(ch0_sensitivity_table));

        //CH0阈值:
        LP_CTMU_CH0_TEMP_TH(ch0_sensitivity_table[ch0_sensity].cfg0);
        LP_CTMU_CH0_STABLE_TH(ch0_sensitivity_table[ch0_sensity].cfg1);
        LP_CTMU_CH0_EDGE_TH(ch0_sensitivity_table[ch0_sensity].cfg2);

        //CH0 按键时间:
        LP_CTMU_CH0_SHORT_KEY_TIME(CFG_M2P_CTMU_CH0_SHORT_TIME_VALUE);
        LP_CTMU_CH0_LONG_KEY_TIME(CFG_M2P_CTMU_CH0_LONG_TIME_VALUE);
        LP_CTMU_CH0_HOLD_KEY_TIME(CFG_M2P_CTMU_CH0_HOLD_TIME_VALUE);

        LP_CTMU_CH0_IE(1);

        //长按开机时间配置
        M2P_CTMU_CH0_SOFTOFF_LONG_TIMEL = (CFG_M2P_CTMU_CH0_SOFTOFF_LONG_TIME & 0xFF);
        M2P_CTMU_CH0_SOFTOFF_LONG_TIMEH = ((CFG_M2P_CTMU_CH0_SOFTOFF_LONG_TIME >> 8) & 0xFF);

        log_info("P11_LCTM_CON = 0x%x", P11_LCTM_CON);
        log_info("LCTM_MOD = 0x%x", LCTM_MOD);
        log_info("LCTM_TMR = 0x%x", LCTM_TMR);
        log_info("LCTM_TIME_BASE_H = 0x%x", LCTM_TIME_BASE_H);
        log_info("LCTM_TIME_BASE_L = 0x%x", LCTM_TIME_BASE_L);
        log_info("LCTM_CHL0_ANA = 0x%x", LCTM_CHL0_ANA);
        log_info("LCTM_CHL0_CON0 = 0x%x", LCTM_CHL0_CON0);
        log_info("LCTM_CHL0_CON1 = 0x%x", LCTM_CHL0_CON1);
        log_info("LCTM_CHL0_CON2 = 0x%x", LCTM_CHL0_CON2);
        log_info("LCTM_CHL0_CON3 = 0x%x", LCTM_CHL0_CON3);
        log_info("LCTM_CHL0_HS_PRD = 0x%x", LCTM_CHL0_HS_PRD);
        log_info("LCTM_CHL0_LS_PRD = 0x%x", LCTM_CHL0_LS_PRD);
        log_info("LCTM_CHL0_DET_TIME_H = 0x%x", LCTM_CHL0_DET_TIME_H);
        log_info("LCTM_CHL0_DET_TIME_L = 0x%x", LCTM_CHL0_DET_TIME_L);
        log_info("LCTM_CHL0_TEMP_H = 0x%x", LCTM_CHL0_TEMP_H);
        log_info("LCTM_CHL0_TEMP_L = 0x%x", LCTM_CHL0_TEMP_L);
        log_info("LCTM_CHL0_STA_H = 0x%x", LCTM_CHL0_STA_H);
        log_info("LCTM_CHL0_STA_L = 0x%x", LCTM_CHL0_STA_L);
        log_info("LCTM_CHL0_EDGE_H = 0x%x", LCTM_CHL0_EDGE_H);
        log_info("LCTM_CHL0_EDGE_L = 0x%x", LCTM_CHL0_EDGE_L);
        log_info("LCTM_CHL0_SHORT_H = 0x%x", LCTM_CHL0_SHORT_H);
        log_info("LCTM_CHL0_SHORT_L = 0x%x", LCTM_CHL0_SHORT_L);
        log_info("LCTM_CHL0_LONG_H = 0x%x", LCTM_CHL0_LONG_H);
        log_info("LCTM_CHL0_LONG_L = 0x%x", LCTM_CHL0_LONG_L);
        log_info("LCTM_CHL0_HOLD_H = 0x%x", LCTM_CHL0_HOLD_H);
        log_info("LCTM_CHL0_HOLD_L = 0x%x", LCTM_CHL0_HOLD_L);
#if TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE
        M2P_CTMU_CH0_RES_SEND = 1;
#else
        M2P_CTMU_CH0_RES_SEND = 0;
#endif
    }

    //CTMU 初始化命令
    lp_touch_key_send_cmd(CTMU_M2P_INIT);

    __this->softoff_mode = LP_TOUCH_SOFTOFF_MODE_ADVANCE;
    __this->init = 1;

#if TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE
    lp_touch_key_online_debug_init();
#endif /* #if TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE */
}
//正常运行的触摸回调函数
void p33_ctmu_key_event_irq_handler()
{
    u8 ctmu_event = P2M_CTMU_KEY_EVENT;

    u16 ch0_res = 0, ch1_res = 0, ch0_iir = 0;

    //log_debug("ctmu msg: 0x%x", ctmu_event);
    switch (ctmu_event) {
    case CTMU_P2M_CH0_RES_EVENT:
        ch0_res = LP_CTMU_CH0_RES_GET();
        ch0_iir = ((P2M_CTMU_CH0_H_IIR_VALUE << 8) | P2M_CTMU_CH0_L_IIR_VALUE);
        printf("ch0_res: %d, ch0_iir: %d\n", ch0_res, ch0_iir);
        break;
    case CTMU_P2M_CH0_SHORT_KEY_EVENT://短按
        printf("CH0: SHORT click\n");
        //__ctmu_short_click_handle();
        break;
    case CTMU_P2M_CH0_LONG_KEY_EVENT://长按
        printf("CH0: LONG click\n");

        break;
    case CTMU_P2M_CH0_HOLD_KEY_EVENT://按住不放
        printf("CH0: HOLD click\n");

        break;
    case CTMU_P2M_CH0_FALLING_EVENT://下降沿（按下）
        printf("CH0: FALLING\n");
        break;
    case CTMU_P2M_CH0_RAISING_EVENT://上升沿（释放）
        if (!(__this->ch_init & BIT(0))) {
            __this->ch_init |= BIT(0);
            /*load_p11_bank_code2ram(1, 1);*/
            return;
        }
        printf("CH0: RAISING\n");
        break;
    default:
        break;
    }
}
//正常运行的触摸禁能：正常运行不使用触摸按键则使用
void lp_touch_key_disable(void)
{
    log_debug("%s", __func__);
    while (!(__this->ch_init & BIT(0))) {
        asm volatile("nop");
    }

    P2M_CTMU_CTMU_WKUP_MSG &= (~(BIT(1)));
    lp_touch_key_send_cmd(CTMU_M2P_DISABLE);
    while (!(P2M_CTMU_CTMU_WKUP_MSG & BIT(1)));
    __this->softoff_mode = LP_TOUCH_SOFTOFF_MODE_LEGACY;
}

//正常运行的触摸使能：正常运行开启触摸按键
void lp_touch_key_enable(void)
{
    log_debug("%s", __func__);
    lp_touch_key_send_cmd(CTMU_M2P_ENABLE);
    __this->softoff_mode = LP_TOUCH_SOFTOFF_MODE_ADVANCE;
}

/***************************  以下是SDK提供的参考示例  ************************************/
#include "power_interface.h"
void lp_touch_key_wakeup_test(void)
{
    lp_touch_key_enable();
    power_set_soft_poweroff();
}


