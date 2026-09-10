#include "asm/includes.h"
#include "asm/rtc.h"
#include "asm/cpu.h"
#include "asm/power/p33.h"


#define YEAR        2000U
#define MONTH       1
#define DAY         1


/*******************************************************
				CONST
********************************************************/
const u8 month_tab1[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};		 ///<非闰年每月的天数
const u8 month_tab2[] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};		 ///<闰年每月的天数

/*----------------------------------------------------------------------------*/
/**@brief	判断闰年
   @param 	year:年份
   @return  真假
   @note	bool leapyear(u16 year)
*/
/*----------------------------------------------------------------------------*/
bool leapyear(u16 year)
{
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
        return true;
    } else {
        return false;
    }
}
/*----------------------------------------------------------------------------*/
/**@brief	年份换算为天数
   @param 	year:年份
   @return  当年天数
   @note	u16 year_to_day(u16 year)
*/
/*----------------------------------------------------------------------------*/
u16 year_to_day(u16 year)
{
    if (leapyear(year)) {
        return 366;
    } else {
        return 365;
    }
}

u16 month_to_day(u16 year, u8 month)
{
    if (leapyear(year)) {
        return month_tab2[month];
    } else {
        return month_tab1[month];
    }
}

void day_to_ymd(u16 day, struct sys_time *sys_time)
{
    u8 tmp = 0;
    while (day >= year_to_day(YEAR + tmp)) {
        day -= year_to_day(YEAR + tmp);
        tmp ++;
    }
    sys_time->year = YEAR + tmp;
    tmp = 0;
    while (day >= month_to_day(sys_time->year, MONTH + tmp)) {
        day -= month_to_day(sys_time->year, MONTH + tmp);
        tmp ++;
    }
    sys_time->month = MONTH + tmp;
    sys_time->day = DAY + day;
}

u16 ymd_to_day(struct sys_time *time)
{
    u16 tmp;
    u16 tmp_day = 0;
    //年->日，当年不算在内，所以是<
    for (tmp = YEAR; tmp < time->year; tmp ++) {
        tmp_day += year_to_day(tmp);
    }
    for (tmp = MONTH; tmp < time->month; tmp ++) {
        tmp_day += month_to_day(time->year, tmp);
    }
    tmp_day += (time->day - 1);						//日->日,当日不算在内，所以日应该减1
    return tmp_day;
}

u8 caculate_weekday_by_time(struct sys_time *r_time)
{
    u16 y = r_time->year;
    u8 m = r_time->month;
    u8 d = r_time->day;
    if (m == 1 || m == 2) {
        m += 12;
        y--;
    }
    return ((d + 2 * m + 3 * (m + 1) / 5 + y + y / 4 - y / 100 + y / 400) % 7 + 1); //1~7  星期1 ~星期7
}
