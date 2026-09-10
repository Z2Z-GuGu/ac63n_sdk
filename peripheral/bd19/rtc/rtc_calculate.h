#ifndef __RTC_CALCULATE_H__
#define __RTC_CALCULATE_H__

#include "typedef.h"


/**
 * @brief 判断是否闰年
 *
 * @param year : 年份
 *
 * @return 0:否 1:是
 */
bool leapyear(u16 year);

/**
 * @brief 年份换算天数
 *
 * @param year : 年份
 *
 * @return 该年天数
 */
u16 year_to_day(u16 year);

/**
 * @brief 月份换算天数
 *
 * @param year : 年份
 * @param month : 月份
 *
 * @return 该月天数
 */
u16 month_to_day(u16 year, u8 month);

/**
 * @brief 总天数转换为年月日
 *
 * @param day : 总天数
 * @param sys_time : 存放转换的年月日的结构体
 */
void day_to_ymd(u16 day, struct sys_time *sys_time);


/**
 * @brief 年月日转换为总天数
 *
 * @param time : 需转换的年月日结构体
 *
 * @return
 */
u16 ymd_to_day(struct sys_time *time);

/**
 * @brief 计算当天为星期几
 *
 * @param r_time : 需要计算的年月日
 *
 * @return 星期几
 */
u8 caculate_weekday_by_time(struct sys_time *r_time);

#endif	//__RTC_CALCULATE_H__

