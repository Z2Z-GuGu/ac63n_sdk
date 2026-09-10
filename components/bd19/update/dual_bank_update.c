#include "includes.h"
#include "printf.h"
#include "resfile.h"
#include "malloc.h"
#include "msg.h"

#include "dual_bank_updata_api.h"

typedef struct {
    u32 file_size;
    u32 file_crc;
    u16 max_pkt_len;
} dual_bank_start_info;

enum {
    DUAL_BANK_UPDATE_START,
    DUAL_BANK_UPDATE_DATA,
    DUAL_BANK_UPDATE_VERIFY,
};

void dual_bank_update_exit(void *param)
{

}

int dual_bank_write_complete_cb(void *priv)
{
    //rsp app current buffer write complete, can send next buffer (need user implement api to response app)
    return 0;
}

void dual_bank_cpu_reset(void *priv)
{
    cpu_reset();
}

int burn_boot_info_result_hdl(int err)
{
    if (err == 0) {
        //boot_info write ok, and rsp app update success (need user implement api to response app)
        /* sys_timeout_add(NULL, dual_bank_cpu_reset, 2000); */
        dual_bank_cpu_reset(NULL);
    } else {
        //boot_info write failed, rsp app update failed  (need user implement api to response app)
        dual_bank_update_exit(NULL);
    }
    return 0;
}

int dual_bank_verify_result_hdl(int res)
{
    if (res) {
        //flash verify success, write boot info
        dual_bank_update_burn_boot_info(burn_boot_info_result_hdl);
    } else {
        //rsp app flash verify failed
        dual_bank_update_exit(NULL);
    }
    return 0;
}

u32 dual_bank_update_deal(u8 msg_type, u8 *data, u32 len)
{
    u32 ret = 0;
    switch (msg_type) {
    case DUAL_BANK_UPDATE_START:
        dual_bank_start_info *info = (dual_bank_start_info *)data;
        ret = dual_bank_passive_update_init(info->file_crc, info->file_size, info->max_pkt_len, NULL);
        if (ret == 0) {
            ret = dual_bank_update_allow_check(info->file_size);
            if (ret == 0) {
                //alloc_check ok, rsp app can update (need user implement api to response app)
            } else {
                //alloc_check error ,rsp app flash size no enough (need user implement api to response app)
                dual_bank_update_exit(NULL);
            }
        } else {
            //cpu resource no enough and rsp app can not update (need user implement api to response app)
            dual_bank_update_exit(NULL);
        }
        break;

    case DUAL_BANK_UPDATE_DATA:
        ret = dual_bank_update_write(data, len, dual_bank_write_complete_cb);
        break;

    case DUAL_BANK_UPDATE_VERIFY:
        //if app calculate crc no use CRC16-CCITT Standard, then user should implement crc_init_hdl and crc_calc_hdl functions
        ret = dual_bank_update_verify(NULL, NULL, dual_bank_verify_result_hdl);
        break;
    }
    return ret;
}

// =============================================从预留区域获取升级文件例子===================================================== //
#define UPDATE_FILE_PATH	"app/UPDATE"
#define UPDATE_TMP_BUFFER	0x1000
extern u32 sdfile_cpu_addr2flash_addr(u32 offset);
extern int norflash_origin_read(u8 *buf, u32 offset, u32 len);
void dual_bank_update_demo(void)
{
    // 打开资源文件
    void *update_file = resfile_open(UPDATE_FILE_PATH);
    u8 *update_tmp_buffer = (u8 *) malloc(UPDATE_TMP_BUFFER);
    memset(update_tmp_buffer, 0, UPDATE_TMP_BUFFER);
    if (NULL == update_file) {
        printf("%s is not exist\n", UPDATE_FILE_PATH);
    }
    // 调用升级初始化函数
    dual_bank_start_info update_file_info = {0};
    update_file_info.file_size = resfile_get_len(update_file);
    u32 update_file_addr = sdfile_cpu_addr2flash_addr(resfile_get_addr(update_file));
    u16 update_file_crc = 0;
    // 文件crc校验
    for (u32 offset = 0, data_len = 0; offset < update_file_info.file_size;) {
        wdt_clear();
        data_len = ((update_file_info.file_size - offset) > UPDATE_TMP_BUFFER) ? UPDATE_TMP_BUFFER : (update_file_info.file_size - offset);
        norflash_origin_read(update_tmp_buffer, update_file_addr + offset, data_len);
        update_file_crc = CRC16_with_initval(update_tmp_buffer, data_len, update_file_crc);
        offset += data_len;
    }
    update_file_info.file_crc = update_file_crc;
    update_file_info.max_pkt_len = UPDATE_TMP_BUFFER;
    if (dual_bank_update_deal(DUAL_BANK_UPDATE_START, (u8 *) &update_file_info, sizeof(dual_bank_start_info))) {
        goto __dual_bank_update_demo_end;
    }

    // 调用写函数
    for (u32 offset = 0, data_len = 0; offset < update_file_info.file_size;) {
        wdt_clear();
        data_len = ((update_file_info.file_size - offset) > UPDATE_TMP_BUFFER) ? UPDATE_TMP_BUFFER : (update_file_info.file_size - offset);
        norflash_origin_read(update_tmp_buffer, update_file_addr + offset, data_len);
        if (dual_bank_update_deal(DUAL_BANK_UPDATE_DATA, update_tmp_buffer, data_len)) {
            goto __dual_bank_update_demo_end;
        }
        offset += data_len;
    }

    // 调用校验函数
    if (dual_bank_update_deal(DUAL_BANK_UPDATE_VERIFY, NULL, 0)) {
        goto __dual_bank_update_demo_end;
    }

__dual_bank_update_demo_end:
    if (update_tmp_buffer) {
        free(update_tmp_buffer);
    }
    if (update_file) {
        resfile_close(update_file);
    }
}

// =============================================从预留区域获取升级文件例子2===================================================== //
extern u32 get_target_update_max_size(void);
extern void set_update_file_verify_crc_info(u16 data_crc);
extern void set_update_file_size_info(u32 file_size);
static u8 update_file_buffer[UPDATE_TMP_BUFFER] = {0};
static u32 record_data_len = 0;
static u32 update_file_data_get(u8 *buffer[])
{
    u32 file_len = 0;
    u32 data_len = 0;

    *buffer = update_file_buffer;

    void *update_file = resfile_open(UPDATE_FILE_PATH);
    if (NULL == update_file) {
        goto __update_file_data_get_end;
    }
    file_len = resfile_get_len(update_file);
    if (record_data_len >= file_len) {
        goto __update_file_data_get_end;
    }

    u32 update_file_addr = sdfile_cpu_addr2flash_addr(resfile_get_addr(update_file));
    data_len = file_len > record_data_len ? UPDATE_TMP_BUFFER : (file_len - record_data_len);

    // 每次从文件中读取UPDATE_TMP_BUFFER这么多的数据
    norflash_origin_read(update_file_buffer, update_file_addr + record_data_len, data_len);

    record_data_len += data_len;

__update_file_data_get_end:
    if (update_file) {
        resfile_close(update_file);
    }
    return data_len;
}

void dual_bank_update_demo_2(void)
{
    u32 target_bank_addr = 0;
    u32 target_bank_max_size = 0;
    if (dual_bank_passive_update_init(0, 0, UPDATE_TMP_BUFFER, NULL)) {
        dual_bank_update_exit(NULL);
        return;
    }

    target_bank_max_size = get_target_update_max_size();
    // 设置上面获取的文件长度
    set_update_file_size_info(target_bank_max_size);
    // 把升级区域全部擦除
    if (dual_bank_update_allow_check(target_bank_max_size)) {
        dual_bank_update_exit(NULL);
        return;
    }

    record_data_len = 0;
    // 写入数据
    u16 update_file_crc = 0;
    u32 update_file_len = 0;
    while (update_file_len < target_bank_max_size) {
        u8 *update_data = NULL;
        u32 data_len = update_file_data_get(&update_data);
        if (data_len) {
            // 下载数据的过程中，需要统计文件长度
            update_file_len += data_len;
            // 只需要调用接口即可，地址不需要获取
            if (dual_bank_update_deal(DUAL_BANK_UPDATE_DATA, update_data, data_len)) {
                dual_bank_update_exit(NULL);
                return;
            }
            // 使用CRC16_with_initval接口可进行crc的累计计算
            update_file_crc = CRC16_with_initval(update_data, data_len, update_file_crc);
        } else {
            break;
        }
    }

    // 设置上面计算完的crc
    set_update_file_verify_crc_info(update_file_crc);
    // 设置上面统计的文件长度
    set_update_file_size_info(update_file_len);
    if (dual_bank_update_verify(NULL, NULL, dual_bank_verify_result_hdl)) {
        dual_bank_update_exit(NULL);
        return;
    }
}

