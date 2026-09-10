#include "includes.h"
#include "rtc.h"
#include "cpu.h"
#include "rtc_calculate.h"


static struct rtc_data *__this = NULL;

#define YEAR        2000U
#define MONTH       1
#define DAY         1

#define WRITE_ALARM     0
#define READ_ALARM      1

#define WRITE_RTC       2
#define READ_RTC        3

#define OS_ENTER_CRITICAL()  \
		CPU_CRITICAL_ENTER(); \

#define OS_EXIT_CRITICAL()  \
		CPU_CRITICAL_EXIT()

/*同步与互斥*/
#define RTC_ENTER_CRITICAL() MSYS_TO_P11_COMMON_CMD(CLOSE_RTC_INTERRUPT)
#define RTC_EXIT_CRITICAL()  MSYS_TO_P11_COMMON_CMD(OPEN_RTC_INTERRUPT)
#define RESUME_RTC()  		 RTC_EXIT_CRITICAL()

/*----------------------------------------------------------------------------*/
/**@brief 	读IRTC
   @param 	cmd：读指令 r_time：读出的日期
   @return  void
   @note  	void read_IRTC(u8 cmd, sstruct sys_time *r_time)
*/
/*----------------------------------------------------------------------------*/
static void read_IRTC(u8 cmd, struct sys_time *r_time)
{
    if (__this->clk_sel == CLK_SEL_LRC) {
        RTC_ENTER_CRITICAL();
        if (cmd == READ_RTC) {
            u16 r_day;

            r_day = M2P_RTC_DAT0;
            r_day <<= 8;
            r_day = M2P_RTC_DAT1 | r_day;

            day_to_ymd(r_day, r_time);

            r_time->hour = M2P_RTC_DAT2;
            r_time->min  = M2P_RTC_DAT3;
            r_time->sec  = M2P_RTC_DAT4;

        } else if (cmd == READ_ALARM) {
            u16 r_day;
            r_day = M2P_RTC_ALARM0;
            r_day <<= 8;
            r_day = M2P_RTC_ALARM1 | r_day;

            day_to_ymd(r_day, r_time);

            r_time->hour = M2P_RTC_ALARM2;
            r_time->min  = M2P_RTC_ALARM3;
            r_time->sec  = M2P_RTC_ALARM4;
        }
        RTC_EXIT_CRITICAL();

    } else {
        if (cmd == READ_RTC) {
            u16 r_day;
            /*read rtc need close irq*/
            OS_ENTER_CRITICAL();

            R3_RTC_CON0 |= BIT(4);
            while (!(R3_RTC_CON0 & BIT(5)));

            r_day = R3_RTC_DAT0;
            r_day <<= 8;
            r_day = R3_RTC_DAT1 | r_day;

            day_to_ymd(r_day, r_time);

            r_time->hour = R3_RTC_DAT2;
            r_time->min  = R3_RTC_DAT3;
            r_time->sec  = R3_RTC_DAT4;

            OS_EXIT_CRITICAL();

        } else if (cmd == READ_ALARM) {
            u16 r_day;
            r_day = R3_ALM_DAT0;
            r_day <<= 8;
            r_day = R3_ALM_DAT1 | r_day;

            day_to_ymd(r_day, r_time);

            r_time->hour = R3_ALM_DAT2;
            r_time->min  = R3_ALM_DAT3;
            r_time->sec  = R3_ALM_DAT4;
        }
    }
}

/*----------------------------------------------------------------------------*/
/**@brief 	写IRTC
   @param 	cmd：写指令 w_time:写入日期
   @return  void
   @note  	void write_IRTC(u8 cmd, sstruct sys_time *w_time)
*/
/*----------------------------------------------------------------------------*/
static void write_IRTC(u8 cmd, struct sys_time *w_time)
{
    if (__this->clk_sel == CLK_SEL_LRC) {
        RTC_ENTER_CRITICAL();

        if (cmd == WRITE_RTC) {
            u16 w_day;
            w_day = ymd_to_day(w_time);

            M2P_RTC_DAT0 = w_day >> 8;
            M2P_RTC_DAT1 = w_day & 0xff;
            M2P_RTC_DAT2 = w_time->hour;
            M2P_RTC_DAT3 = w_time->min;
            M2P_RTC_DAT4 = w_time->sec;

        } else if (cmd == WRITE_ALARM) {
            u16 w_day;
            w_day = ymd_to_day(w_time);
            M2P_RTC_ALARM0 = w_day >> 8;
            M2P_RTC_ALARM1 = w_day & 0xff;
            M2P_RTC_ALARM2 = w_time->hour;
            M2P_RTC_ALARM3 = w_time->min;
            M2P_RTC_ALARM4 = w_time->sec;
        }

        RTC_EXIT_CRITICAL();

    } else {

        if (cmd == WRITE_RTC) {
            u16 w_day;
            w_day = ymd_to_day(w_time);

            OS_ENTER_CRITICAL();

            R3_RTC_DAT0 = w_day >> 8;
            R3_RTC_DAT1 = w_day & 0xff;
            R3_RTC_DAT2 = w_time->hour;
            R3_RTC_DAT3 = w_time->min;
            R3_RTC_DAT4 = w_time->sec;

            OS_EXIT_CRITICAL();

        } else if (cmd == WRITE_ALARM) {
            u16 w_day;
            w_day = ymd_to_day(w_time);
            R3_ALM_DAT0 = w_day >> 8;
            R3_ALM_DAT1 = w_day & 0xff;
            R3_ALM_DAT2 = w_time->hour;
            R3_ALM_DAT3 = w_time->min;
            R3_ALM_DAT4 = w_time->sec;
        }
    }
}

void rtc_set_alarm_ctrl(u8 set_alarm)
{
    if (__this->clk_sel == CLK_SEL_LRC) {
        RTC_ENTER_CRITICAL();
        M2P_RTC_ALARM_EN = set_alarm;
        RTC_EXIT_CRITICAL();
    } else {
        if (set_alarm) {
            R3_ALM_CON |= BIT(0);
        } else {
            R3_ALM_CON &= ~BIT(0);
        }
    }
}

void rtc_write_time(struct sys_time *curr_time)
{
    rtc_set_alarm_ctrl(0);
    write_IRTC(WRITE_RTC, curr_time);
    rtc_set_alarm_ctrl(1);
}

void rtc_read_time(struct sys_time *curr_time)
{
    read_IRTC(READ_RTC, curr_time);
}

void rtc_write_alarm(struct sys_time *alarm_time)
{
    rtc_set_alarm_ctrl(0);
    write_IRTC(WRITE_ALARM, alarm_time);
    rtc_set_alarm_ctrl(1);
}

void rtc_read_alarm(struct sys_time *alarm_time)
{
    read_IRTC(READ_ALARM, alarm_time);
}


void __attribute__((weak)) alm_wakeup_isr(void)
{
    if (__this->cbfun) {
        __this->cbfun(0);
    }
}

enum {
    P2M_RTC_CMD_TRIM = 1,
    P2M_RTC_CMD_ALARM = 2,
};

extern u32 __get_lrc_hz(void);
static void p2m_rtc_cmd_trim(void *priv)
{
    log_info("rtc_trim");
    RTC_ENTER_CRITICAL();
    u32 lrc_hz = __get_lrc_hz();
    M2P_LRC_FEQL = lrc_hz & 0xff;
    M2P_LRC_FEQH = (lrc_hz & 0xff00) >> 8;
    RTC_EXIT_CRITICAL();
    if (P2M_SOFTOFF) {
        power_set_soft_poweroff();
    }
}

void rtc_wakup_source()
{

    if (__this->clk_sel == CLK_SEL_LRC) {
        u8 cmd = P2M_RTC_CMD;
        log_info("cmd: %d", cmd);
        if (cmd == P2M_RTC_CMD_ALARM) {
            rtc_set_alarm_ctrl(0);
            alm_wakeup_isr();
        } else if (cmd == P2M_RTC_CMD_TRIM) {
            p2m_rtc_cmd_trim(NULL);
            /* sys_timeout_add(NULL, p2m_rtc_cmd_trim, 10); */
        }
        RESUME_RTC();
    } else {
        u8 sfr = R3_WKUP_SRC;
        /* log_i("rtc_wkup_src is 0x%x", sfr); */

        if (sfr & BIT(R3_WKUP_SRC_ALM)) {
            rtc_set_alarm_ctrl(0);
            alm_wakeup_isr();
        }
        if (sfr & BIT(R3_WKUP_SRC_256HZ)) {
            TIME_256HZ_CPND(1);
        }
        if (sfr & BIT(R3_WKUP_SRC_64HZ)) {
            TIME_64HZ_CPND(1);
        }
        if (sfr & BIT(R3_WKUP_SRC_2HZ)) {
            TIME_2HZ_CPND(1);
        }
        if (sfr & BIT(R3_WKUP_SRC_1HZ)) {
            TIME_1HZ_CPND(1);
        }
    }
}

void rtc_time_dump(void)
{
    struct sys_time tmp_time;
    memset((u8 *)&tmp_time, 0, sizeof(tmp_time));
    rtc_read_time(&tmp_time);
    log_info("rtc_read_sys_time: %d-%d-%d %d:%d:%d",
             tmp_time.year,
             tmp_time.month,
             tmp_time.day,
             tmp_time.hour,
             tmp_time.min,
             tmp_time.sec);
}

extern u8 power_reset_flag;
extern bool __lrc_trace_ready();
extern void clr_wdt();
int rtc_init(struct rtc_data *arg)
{
    __this = arg;

    if (get_reset_source_value() & BIT(MSYS_P11_RST_RTC_WAKEUP)) {
        rtc_wakup_source();
    }

    if (__this->clk_sel == CLK_SEL_LRC) {

    } else if (__this->clk_sel == CLK_SEL_32K) {
        //32K_OSCO
        gpio_set_direction(IO_PORTB_08, 1);
        gpio_set_die(IO_PORTB_08, 0);
        gpio_set_dieh(IO_PORTB_08, 0);
        gpio_set_pull_up(IO_PORTB_08, 0);
        gpio_set_pull_down(IO_PORTB_08, 0);
        //32K_OSCI
        gpio_set_direction(IO_PORTB_09, 1);
        gpio_set_die(IO_PORTB_09, 0);
        gpio_set_dieh(IO_PORTB_09, 0);
        gpio_set_pull_up(IO_PORTB_09, 0);
        gpio_set_pull_down(IO_PORTB_09, 0);

        P3_OSL_CON |= (BIT(0));
    } else {
        CLOCK_KEEP(1);
    }
    ALM_CLK_SEL(__this->clk_sel);

    P33_CON_SET(P3_VLD_KEEP, 1, 1, 1);                              //连接到P33，否则会导致闹钟中断进不去

    if (power_reset_flag) {
        /* log_i(">>> rtc_alarm_reset"); */
        rtc_write_time(__this->default_sys_time);
        rtc_write_alarm(__this->default_alarm);

        if (__this->clk_sel == CLK_SEL_LRC) {
            M2P_RTC_TRIM_TIME = __this->trim_t;
            while (!__lrc_trace_ready()) {
                log_info("%d", __lrc_trace_ready());
                clr_wdt();
            }
            p2m_rtc_cmd_trim(NULL);
            MSYS_TO_P11_COMMON_CMD(OPEN_RTC_INIT);
        }
    }

    rtc_time_dump();

    return 0;
}


static struct sys_time def_sys_time = {  //初始一下当前时间
    .year = 1212,
    .month = 12,
    .day = 12,
    .hour = 12,
    .min = 12,
    .sec = 12,
};

static struct sys_time test_sys_time = {  //初始一下目标时间
    .year = 2021,
    .month = 1,
    .day = 1,
    .hour = 0,
    .min = 0,
    .sec = 0,
};

static struct sys_time def_alarm = {     //初始一下闹钟时间
    .year = 2050,
    .month = 1,
    .day = 1,
    .hour = 0,
    .min = 0,
    .sec = 0,
};

static struct sys_time test_alarm = {     //初始一下目标闹钟时间
    .year = 2050,
    .month = 1,
    .day = 1,
    .hour = 18,
    .min = 18,
    .sec = 18,
};

static struct rtc_data rtc_data = {
    .default_sys_time = &def_sys_time,
    .default_alarm = &def_alarm,
    .cbfun = NULL,                      //闹钟中断的回调函数,用户自行定义
    .clk_sel = CLK_SEL_LRC,
    .trim_t = 1,
};

void rtc_test_demo()
{
    struct sys_time tmp_time;
    memset((u8 *)&tmp_time, 0, sizeof(tmp_time));

    rtc_init(&rtc_data);				//初始化rtc

    rtc_read_time(&tmp_time);						//读当前rtc时间
    log_info("rtc_rtc_read_time_before: %d-%d-%d %d:%d:%d", tmp_time.year, tmp_time.month, tmp_time.day, tmp_time.hour, tmp_time.min, tmp_time.sec);	//打印读取时间值

    rtc_write_time(&test_sys_time); 		//修改rtc时间
    rtc_read_time(&tmp_time); 				//读修改后rtc时间
    log_info("rtc_rtc_read_time_after: %d-%d-%d %d:%d:%d", tmp_time.year, tmp_time.month, tmp_time.day, tmp_time.hour, tmp_time.min, tmp_time.sec);		//打印修改后时间值

    rtc_read_alarm(&tmp_time); 					//读当前alarm时间
    log_info("rtc_read_alarm_before: %d-%d-%d %d:%d:%d", tmp_time.year, tmp_time.month, tmp_time.day, tmp_time.hour, tmp_time.min, tmp_time.sec);	//打印读取闹钟时间值

    rtc_write_alarm(&test_alarm); 	//修改alarm时间
    rtc_read_alarm(&tmp_time); 		//读修改后alarm时间
    log_info("rtc_read_alarm_after: %d-%d-%d %d:%d:%d", tmp_time.year, tmp_time.month, tmp_time.day, tmp_time.hour, tmp_time.min, tmp_time.sec);		//打印修改后闹钟时间值
}




