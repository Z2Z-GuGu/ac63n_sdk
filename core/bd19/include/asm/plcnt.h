#ifndef __PLCNT_H__
#define __PLCNT_H__

#include "typedef.h"
#include "asm/includes.h"

#define PLCNT_KEY_CH_MAX		3

struct touch_key_port {
    u16 press_delta;    //按下判决的阈值
    u8 port; 			//触摸按键IO
    u8 key_value; 		//按键返回值
};

struct touch_key_platform_data {
    u8 num; 	        //触摸按键个数
    const struct touch_key_port *port_list;
};

#define TOUCH_KEY_PLATFORM_DATA_BEGIN(data) \
		static const struct touch_key_platform_data data = {

#define TOUCH_KEY_PLATFORM_DATA_END() \
};

/**
 * @brief 异步的方式，轮询采集各个通道的放电计数值，获取上一通道的值，并启动当前通道的值
 */
void scan_capkey_async(void);

/**
 * @brief 同步的方式，一次采集完所有通道的放电计数值，采完再走
 */
void scan_capkey(void);

/**
 * @brief   引脚放电计数模块初始化
 * @parm _data 初始化的参数结构体地址 ： const struct touch_key_platform_data *
*/
int plcnt_init(void *_data);

/**
 * @brief 获取plcnt模块的对应的键值
 * @return 返回按键号
*/
u8 get_plcnt_key_value(void);



#endif


