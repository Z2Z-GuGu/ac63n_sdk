#ifndef __DUAL_BANK_UPDATE_UART_H__
#define __DUAL_BANK_UPDATE_UART_H__

#include "typedef.h"

#define DUAL_BANK_UPDATE_BY_UFW    0//双备份模块宏

#define MSG_UART_UPDATE_READY       0x1
#define MSG_UART_UPDATE_START       0x2
#define MSG_UART_UPDATE_START_RSP   0X3
#define MSG_UART_UPDATE_READ_RSP    0x4

#define PROTOCAL_SIZE       528
#define SYNC_SIZE           6
#define SYNC_MARK0          0xAA
#define SYNC_MARK1          0x55

typedef union {
    u8 raw_data[PROTOCAL_SIZE + SYNC_SIZE];
    struct {
        u8 mark0;
        u8 mark1;
        u16 length;
        u8 data[PROTOCAL_SIZE + 2]; // 最后CRC16
    } data;
} protocal_frame_t;

struct file_info {
    u8 cmd;
    u32 addr;
    u32 len;
} __attribute__((packed));

extern const struct uart_platform_data dual_uart_config;
void update_download_opt(void);
void dual_bank_update_init(void);

#endif
