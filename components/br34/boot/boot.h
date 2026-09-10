#ifndef __BOOT_H__
#define __BOOT_H__


struct vm_info {
    u8 align;		  			//from uboot, 按 n * 256 对齐
    u32 vm_saddr;				//from sdfile, flash addr
    u32 vm_res;					//reverse_bytes
    u32 vm_size;		  		//from sdfile
};

struct sfc_info {
    u32 sfc_base_addr;  		//flash memory addr, from uboot
    u32 app_addr;  				//cpu logic addr, from uboot
};


struct bt_mac_addr {
    u8 value[6];
    u16 value_crc;
};

typedef struct _boot_info {
    struct vm_info vm;
    struct sfc_info sfc;
    u32 flash_size; 		//from uboot
    u16 chip_id; 			//from uboot
    u16 trim_value; 		//from uboot
    struct bt_mac_addr mac;
} BOOT_INFO;

//=====================================
struct flash_head {
    u16 crc;
    u16 size4burner;
    u8 vid[4]; 	//u32 vm_eaddr;
    u32 FlashSize;
    u8 FsVersion;  //flash文件结构
    u8 align; 	//对齐 n * 256
    u8 res;
    u8 SpecialOptFlag;
    u8 pid[16];
};

typedef struct boot_device_info {
    struct flash_head *fs_info;
    struct sfc_info sfc;
    u16 chip_id; 			//from uboot
    u16 trim_value; 		//from uboot
    u8 bt_mac_addr[8];
} BOOT_DEVICE_INFO;


void system_init(void);		// 系统初始化（时钟/端口/关键外设）

extern BOOT_INFO boot_info;

/* ============ 系统引导相关的外部符号 ============ */
extern void check_upgrade();
extern void board_init();
extern void debug_uart_early_init();
extern void exception_irq_handler();
extern void vm_init_app();
extern int  app_version_check();

#endif

