#include "dual_bank_update_uart.h"
#include "dual_bank_update_loop.h"
#include "asm/includes.h"

// 命令
#define CMD_UPDATE_START    0x01
#define CMD_UPDATE_READ     0x02
#define CMD_UPDATE_END      0x03
#define CMD_SEND_UPDATE_LEN 0x04
#define CMD_KEEP_ALIVE      0x05

#define RETRY_TIME			4

static protocal_frame_t protocal_frame __attribute__((aligned(4)));
static volatile u32 uart_file_offset = 0;
static volatile u8 dual_bank_update_flag = 0;
static volatile u16 rx_cnt;
static int (*uart_update_resume_hdl)(void *priv) = NULL;
static int (*uart_update_sleep_hdl)(void *priv) = NULL;

static u32 update_baudrate = 9600;

enum {
    SEEK_SET = 0x0,
    SEEK_CUR = 0x1,
    SEEK_END = 0x2,
};

enum {
    CMD_UART_UPDATE_START = 0x1,
    CMD_UART_UPDATE_READ,
    CMD_UART_UPDATE_END,
    CMD_UART_UPDATE_UPDATE_LEN,
    CMD_UART_JEEP_ALIVE,
    CMD_UART_UPDATE_READY,
};

enum {
    CMD_UART_UPDATE_FLAG_NONE,
    CMD_UART_UPDATE_FLAG_RECV_DATA,
    CMD_UART_UPDATE_FLAG_RETRY,
    CMD_UART_UPDATE_FLAG_INIT_ERR = 0xff,
};

static bool uart_send_packet(u8 *buf, u16 length)
{
    bool ret = TRUE;
    u16 crc;
    u8 *buffer;

    buffer = (u8 *)&protocal_frame;
    protocal_frame.data.mark0 = SYNC_MARK0;
    protocal_frame.data.mark1 = SYNC_MARK1;
    protocal_frame.data.length = length;
    memcpy((char *)&buffer[4], buf, length);
    crc = CRC16(buffer, length + SYNC_SIZE - 2);
    memcpy(buffer + 4 + length, &crc, 2);
    log_debug("tx(%d)", length + SYNC_SIZE);
    log_debug_hexdump((u8 *)&protocal_frame, length + SYNC_SIZE);
    uart_tx_buf(dual_uart_config.id, (u8 *)&protocal_frame, length + SYNC_SIZE);
    return ret;
}

static bool uart_update_cmd(u8 cmd, u8 *buf, u32 len)
{
    u8 *pbuf, i;
    //for (i = 0; i < RETRY_TIME; i++)
    {
        pbuf = protocal_frame.data.data;
        pbuf[0] = cmd;
        if (buf) {
            memcpy(pbuf + 1, buf, len);
        }
        uart_send_packet(pbuf, len + 1);
    }
    return TRUE;
}

static void uart_loop_rx_handler(u8 *buf, u16 len)
{
    u16 crc, crc0, i, ch;
    for (i = 0; i < len; i++) {
        ch = buf[i];
__recheck:
        if (rx_cnt == 0) {
            if (ch == SYNC_MARK0)	 {
                protocal_frame.raw_data[rx_cnt++] = ch;
            }
        } else if (rx_cnt == 1) {
            protocal_frame.raw_data[rx_cnt++] = ch;
            if (ch != SYNC_MARK1) {
                rx_cnt = 0;
                goto __recheck;
            }
        } else if (rx_cnt < 4) {
            protocal_frame.raw_data[rx_cnt++] = ch;
        } else {
            protocal_frame.raw_data[rx_cnt++] = ch;
            if (rx_cnt == (protocal_frame.data.length + SYNC_SIZE)) {
                log_debug("rx(%d)", rx_cnt);
                log_debug_hexdump((u8 *)&protocal_frame, rx_cnt);
                rx_cnt = 0;
                crc = CRC16(protocal_frame.raw_data, protocal_frame.data.length + SYNC_SIZE - 2);
                memcpy(&crc0, &protocal_frame.raw_data[protocal_frame.data.length + SYNC_SIZE - 2], 2);
                if (CMD_UART_UPDATE_FLAG_INIT_ERR != dual_bank_update_flag) {
                    if (crc0 == crc) {
                        dual_bank_update_flag = CMD_UART_UPDATE_FLAG_RECV_DATA;
                    } else {
                        dual_bank_update_flag = CMD_UART_UPDATE_FLAG_RETRY;
                    }
                }
                if (uart_update_resume_hdl && (CMD_UART_UPDATE_READ == protocal_frame.data.data[0])) {
                    uart_update_resume_hdl(NULL);
                }
            }
        }
    }
}

static void uart_update_state_cbk(int type, u32 state, void *priv)
{
    update_ret_code_t *ret_code = (update_ret_code_t *)	priv;
    if (ret_code) {
        log_info("state:%x err:%x\n", ret_code->stu, ret_code->err_code);
    }

    switch (state) {
    case UPDATE_CH_EXIT:
        uart_file_offset = 0;
        update_baudrate = dual_uart_config.baudrate;
        uart_close(dual_uart_config.id, &dual_uart_config);
        if ((0 == ret_code->stu) && (0 == ret_code->err_code)) {
            cpu_reset();
        }
        break;
    default:
        break;
    }
}

static void uart_update_hdl_register(int (*resume_hdl)(void *priv), int (*sleep_hdl)(void *priv))
{
    uart_update_resume_hdl = resume_hdl;
    uart_update_sleep_hdl = sleep_hdl;
}

static u16 uart_f_open(void)
{
    return 1;
}

static u32 uart_dev_receive_data(void *buf, u32 relen, u32 addr)
{
    u8 i;
    struct file_info file_cmd;
    for (i = 0; i < RETRY_TIME; i++) {
        if (i > 0) {
            putchar('r');
        }
        file_cmd.cmd = CMD_UPDATE_READ;
        file_cmd.addr = addr;
        file_cmd.len = relen;
        uart_send_packet((u8 *)&file_cmd, sizeof(file_cmd));
        if (uart_update_sleep_hdl) {
            if (uart_update_sleep_hdl(NULL)) {
                log_error("uart_time_out\n");
                return -1;
            }
            if (CMD_UART_UPDATE_FLAG_RETRY == dual_bank_update_flag) {
                // crc错误，重发
                dual_bank_update_flag = CMD_UART_UPDATE_FLAG_NONE;
                continue;
            }
        }
        memcpy(&file_cmd, protocal_frame.data.data, sizeof(file_cmd));
        if ((file_cmd.cmd != CMD_UPDATE_READ) || (file_cmd.addr != addr) || (file_cmd.len != relen)) {
            continue;
        }
        memcpy(buf, &protocal_frame.data.data[sizeof(file_cmd)], protocal_frame.data.length - sizeof(file_cmd));
        return (protocal_frame.data.length - sizeof(file_cmd));
    }

    if (i == RETRY_TIME) {
        log_error("receive data is err\n");
        relen = -1;
    }
    putchar('R');
    return relen;
}

static u16 uart_f_read(void *handle, void *buf, u32 relen)
{
    u32 len;
    log_info("%s\n", __func__);
    len = uart_dev_receive_data(buf, relen, uart_file_offset);
    if ((u32) - 1 == len) {
        log_error("%s err\n", __func__);
        return -1;
    }
    uart_file_offset += len;
    return 0;
}

static int uart_f_seek(void *fp, u8 type, u32 offset)
{
    if (type == SEEK_SET) {
        uart_file_offset = offset;
    } else if (type == SEEK_CUR) {
        uart_file_offset += offset;
    }
    return 0;
}

static u16 uart_f_stop(u8 err)
{
    uart_update_cmd(CMD_UPDATE_END, &err, 1);
    update_baudrate = 9600;
    return 0;
}

static const update_op_api_t uart_ch_update_op = {
    .ch_init = uart_update_hdl_register,
    .f_open  = uart_f_open,
    .f_read  = uart_f_read,
    .f_seek  = uart_f_seek,
    .f_stop  = uart_f_stop,
};

extern int time_out_msec(u32 msec, u8 *condition);
extern void uart_update_set_baud(int id, u32 baudrate);
static void uart_update_recv(u8 cmd, u8 *buf, u32 len)
{
    u32 baudrate = 9600;
    switch (cmd) {
    case CMD_UPDATE_START:
        memcpy(&baudrate, buf, 4);
        log_info("CMD_UPDATE_START:%d\n", baudrate);
        if (update_baudrate != baudrate) {
            update_baudrate = baudrate;
            uart_update_set_baud(dual_uart_config.id, baudrate);
            time_out_msec(500, NULL);
            uart_update_cmd(CMD_UPDATE_START, (u8 *)&update_baudrate, 4);
        } else {
            update_mode_info_t info = {
                .type = UART_UPDATA,
                .state_cbk = uart_update_state_cbk,
                .p_op_api = &uart_ch_update_op,
                .en = 1,
            };
            if (active_update(&info)) {
                dual_bank_update_init();
            }
        }
        break;
    }
}

void update_download_opt(void)
{
    if (CMD_UART_UPDATE_FLAG_RECV_DATA == dual_bank_update_flag) {
        switch (protocal_frame.data.data[0]) {
        case CMD_UART_UPDATE_START:
            uart_update_recv(protocal_frame.data.data[0], &protocal_frame.data.data[1], protocal_frame.data.length - 1);
            break;
        case CMD_UART_UPDATE_READ:
            break;
        case CMD_UART_UPDATE_END:
            break;
        case CMD_UART_UPDATE_UPDATE_LEN:
            break;
        case CMD_UART_JEEP_ALIVE:
            break;
        case CMD_UART_UPDATE_READY:
            uart_update_cmd(CMD_UPDATE_START, NULL, 0);
            break;
        default:
            log_error("unknow cmd...\n");
        }
        dual_bank_update_flag = CMD_UART_UPDATE_FLAG_NONE;
    }
}

void dual_bank_update_init(void)
{
    log_info("dual bank uart%d init!\n", dual_uart_config.id);
    rx_cnt = 0;
    if (dual_uart_config.id != uart_init(&dual_uart_config)) {
        log_error("%s err\n", __func__);
        dual_bank_update_flag = CMD_UART_UPDATE_FLAG_INIT_ERR;
        return;
    }
    uart_set_rx_irq_handler(dual_uart_config.id, uart_loop_rx_handler);
}

