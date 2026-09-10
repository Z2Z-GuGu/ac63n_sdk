#ifndef _DUAL_BANK_UPDATE_LOOP_H_
#define _DUAL_BANK_UPDATE_LOOP_H_

#include "typedef.h"

#define UPDATA_MAGIC            (0x5A00)        //防止CRC == 0 的情况
typedef enum {
    UPDATA_NON = UPDATA_MAGIC,
    UPDATA_READY,
    UPDATA_SUCC,
    UPDATA_PARM_ERR,
    UPDATA_DEV_ERR,
    UPDATA_KEY_ERR,
} UPDATA_RESULT;

typedef enum {
    USB_UPDATA = UPDATA_MAGIC,      //0x5A00
    SD0_UPDATA,                     //0x5A01
    SD1_UPDATA,
    PC_UPDATA,
    UART_UPDATA,
    BT_UPDATA,
    BLE_APP_UPDATA,
    SPP_APP_UPDATA,
    DUAL_BANK_UPDATA,
    BLE_TEST_UPDATA,
    NORFLASH_UPDATA,
    //NOTE:以上的定义不要调整,新升级方式在此添加,注意加在USER_NORFLASH_UFW_UPDATA之前;
    USER_NORFLASH_UFW_UPDATA,

    NON_DEV = 0xFFFF,
} UPDATA_TYPE;

typedef enum _UPDATE_STATE_T {
    UPDATE_TASK_INIT,
    UPDATE_CH_INIT,
    UPDATE_CH_SUCESS_REPORT,
    UPDATE_CH_EXIT,
} UPDATE_STATE_T;

typedef struct _ret_code {
    int stu;
    u8 err_code;
} update_ret_code_t;

typedef struct _update_op_api_t {
    void (*ch_init)(int (*resume_hdl)(void *priv), int (*sleep_hdl)(void *priv));
    u16(*f_open)(void);
    u16(*f_read)(void *fp, void *buff, u32 len);
    int (*f_seek)(void *fp, u8 type, u32 offset);
    u16(*f_stop)(u8 err);
    int (*notify_update_content_size)(void *priv, u32 size);
    void (*ch_exit)(void *priv);
} update_op_api_t;

typedef struct _update_mode_info_t {
    s32 type;
    void (*state_cbk)(int type, u32 status, void *priv);
    const update_op_api_t *p_op_api;
    // u8 task_en;
    u8 en;
} update_mode_info_t;

int active_update(update_mode_info_t *info);

#endif
