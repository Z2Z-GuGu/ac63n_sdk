#include "includes.h"

/*-----------------------------------------------------------*/


#ifdef SUPPORT_MS_EXTENSIONS
/* #pragma bss_seg(".debug_bss") */
/* #pragma data_seg(".debug_data") */
/* #pragma const_seg(".debug_const") */
/* #pragma code_seg(".debug_code") */
/* #pragma str_literal_override(".debug_code") */
#endif

/*-----------------------------------------------------------*/
#define _MPU_WR_SFR_EN 	        do{JL_MPU->WREN = 0xE7;}while(0)
#define _MPU_WR_EN			    JL_MPU->WREN
#define _DEBUG_MSG_CLR			JL_MPU->MSG_CLR

#define _DCU_CON                JL_DCU->CON
#define _DCU_EMU_CON            JL_DCU->EMU_CON
#define _DCU_EMU_MSG            JL_DCU->EMU_MSG
#define _DCU_EMU_ID             JL_DCU->EMU_ID
#define _DCU_CMD_CON            JL_DCU->CMD_CON
#define _DCU_CMD_BEG            JL_DCU->CMD_BEG
#define _DCU_CMD_END            JL_DCU->CMD_END
#define _DCU_CNT_RACK           JL_DCU->CNT_RACK
#define _DCU_CNT_UNACK          JL_DCU->CNT_UNACK

#define _DSP_PC_LIML0			j32CPU(0)->LIM_PC0_L
#define _DSP_PC_LIMH0			j32CPU(0)->LIM_PC0_H
#define _DSP_PC_LIML1			j32CPU(0)->LIM_PC1_L
#define _DSP_PC_LIMH1			j32CPU(0)->LIM_PC1_H
#define _DSP_PC_LIML2			j32CPU(0)->LIM_PC2_L
#define _DSP_PC_LIMH2			j32CPU(0)->LIM_PC2_H
#define _EMU_CON				j32CPU(0)->EMU_CON
#define _EMU_MSG				j32CPU(0)->EMU_MSG
#define _EMU_SSP_H              j32CPU(0)->EMU_SSP_H
#define _EMU_SSP_L              j32CPU(0)->EMU_SSP_L
#define _EMU_USP_H              j32CPU(0)->EMU_USP_H
#define _EMU_USP_L              j32CPU(0)->EMU_USP_L
#define _ETM_CON                j32CPU(0)->ETM_CON
#define _ESU_CON                j32CPU(0)->ESU_CON

#define _ICU_CON                j32CPU_icu(0)->CON
#define _ICU_EMU_CON            j32CPU_icu(0)->EMU_CON
#define _ICU_EMU_MSG            j32CPU_icu(0)->EMU_MSG
#define _ICU_EMU_ID             j32CPU_icu(0)->EMU_ID
#define _ICU_CMD_CON            j32CPU_icu(0)->CMD_CON
#define _ICU_CMD_BEG            j32CPU_icu(0)->CMD_BEG
#define _ICU_CMD_END            j32CPU_icu(0)->CMD_END
#define _ICU_CNT_RACK           j32CPU_icu(0)->CNT_RACK
#define _ICU_CNT_UNACK          j32CPU_icu(0)->CNT_UNACK

/* #if  1 */
/* #define log_d       printf */
/* #define debug_log 	printf */
/* #define log_info 	printf */
/* #define log_error 	printf */
/* #else */
/* #define log_d(...) */
/* #define debug_log(...) */
/* #define log_info(...) */
/* #define log_error(...) */
/* #endif */

extern int text_begin;
extern int text_end;
extern int data_code_pc_limit_begin;
extern int data_code_pc_limit_end;
extern u32 _cpu0_sstack_begin;
extern u32 _cpu0_sstack_end;

/*
 *
 */
#if 1  //add by dengyulin

struct dev_id_str {
    char name[16];
    u8 id ;
};

const struct dev_id_str dev_id_list[] = {
    {"DBG_REV",             0x00},
    {"DBG_ALNK0",           0x01},
    {"DBG_ALNK1",           0x02},
    {"DBG_AUDIO",           0x03},
    {"DBG_SPDIF_D",         0x04},
    {"DBG_SPDIF_I",         0x05},
    {"DBG_ISP",             0x06},
    {"DBG_PAP",             0x07},
    {"DBG_PLNK",            0x08},
    {"DBG_SBC",             0x09},
    {"DBG_AAC",             0x0a},
    {"DBG_AES",             0x0b},
    {"DBG_SD0",             0x0c},
    {"DBG_SD1",             0x0d},
    {"DBG_SPI0",            0x0e},
    {"DBG_SPI1",            0x0f},
    {"DBG_SPI2",            0x10},
    {"DBG_UART0W",          0x11},
    {"DBG_UART0R",          0x12},
    {"DBG_UART1W",          0x13},
    {"DBG_UART1R",          0x14},
    {"DBG_UART2W",          0x15},
    {"DBG_UART2R",          0x16},
    {"DBG_CTM",             0x17},
    {"DBG_AXI_M0",          0x80},
    {"DBG_AXI_M1",          0x81},
    {"DBG_AXI_M2",          0x82},
    {"DBG_AXI_M3",          0x83},
    {"DBG_AXI_M4",          0x84},
    {"DBG_AXI_M5",          0x85},
    {"DBG_AXI_M6",          0x86},
    {"DBG_AXI_M7",          0x87},
    {"DBG_AXI_M8",          0x88},
    {"DBG_AXI_M9",          0x89},
    {"DBG_AXI_MA",          0x8a},
    {"DBG_AXI_MB",          0x8b},
    {"DBG_AXI_MC",          0x8c},
    {"DBG_AXI_MD",          0x8d},
    {"DBG_AXI_ME",          0x8e},
    {"DBG_AXI_MF",          0x8f},
    {"DBG_USB",             0xa0},
//  {"DBG_FM       ",          0xa1},
    {"DBG_BT",              0xa2},
    {"DBG_FFT",             0xa3},
    {"DBG_EQ",              0xa4},
    {"DBG_FIR",             0xa5},
    {"DBG_CPU0_WR",           0xf0},
    {"DBG_CPU0_RD",           0xf1},
    {"DBG_CPU0_IF",           0xf2},
    {"DBG_SDTAP",           0xff},
    {"MSG_NULL",            0xff}
};

char *get_dev_name(u32 id)
{
    int i ;

    for (i = 0; i < ARRAY_SIZE(dev_id_list); i++) {
        if (dev_id_list[i].id == id) {
            return ((char *)dev_id_list[i].name);
        }
    }
    return NULL;
}

u32 get_dev_id(char *name)
{
    int i ;

    for (i = 0; i < ARRAY_SIZE(dev_id_list); i++) {
        if (!memcmp(dev_id_list[i].name, name, strlen(name))) {
            return dev_id_list[i].id;
        }
    }

    return -1;
}


#define UART_BUF        JL_UART2->BUF
#define UART_CON        JL_UART2->CON0
#define UART_BAUD       JL_UART2->BAUD

#define AT_RAM      //AT(.volatile_ram_code)
AT_RAM
void sputchar(char c)
{
    UART_BUF = c;
    __asm__ volatile("csync");
    while ((UART_CON & BIT(15)) == 0);
    UART_CON |= BIT(13);
}

AT_RAM
static void sput_hex(u32 dat, u8 len)
{
    for (s32 i = len - 1; i >= 0; i--) {
        u8 val = 0xf & (dat >> (i * 4));
        if (val > 9) {
            sputchar(val - 10 + 'A');
        } else {
            sputchar(val + '0');
        }
    }
}

AT_RAM
_INLINE_
static int sputs(const char *s)
{
    char c;
    const char *ser = s;
    while (1) {
        c = *s;
        if (c) {
            sputchar(c);
            s++;
        } else {
            break;
        }
    }
    return (int)s - (int)ser;
}

AT_RAM
static void sput_u8hex(u8 dat)
{
    sputchar('0');
    sputchar('x');
    sput_hex(dat, 2);
    sputchar(' ');
}

AT_RAM
static void sput_u16hex(u16 dat)
{
    sputchar('0');
    sputchar('x');
    sput_hex(dat, 4);
    sputchar(' ');
}

AT_RAM
void sput_u32hex(u32 dat)
{
    sputchar('0');
    sputchar('x');
    sput_hex(dat, 8);
    sputchar(' ');
}

AT_RAM
void sput_u64hex(u64 dat)
{
    sputchar('0');
    sputchar('x');
    sput_hex(dat >> 32, 8);
    sput_hex(dat, 8);
    sputchar(' ');
}

AT_RAM
void sput_buf(const u8 *buf, int len)
{
    for (int i = 0; i < len; i++) {
        if ((i % 16) == 0) {
            sputchar('\n') ;
        }
        sput_hex(buf[i], 2);
        sputchar(' ');
    }
    sputchar('\n') ;
}
/*-----------------------------------------------------------*/



static char *const emu_msg[32] = {
    "sys excption",		//31
    "icache excption",		//30
    "dcache excption",		//29
    "reserved",		//28

    "reserved",		//27
    "reserved",		//26
    "reserved",		//25
    "reserved",		//24

    "reserved",		//23
    "reserved",		//22
    "reserved",		//21
    "fpu_inv_err",	//20

    "fpu_inf_err",	//19
    "fpu_tiny",		//18
    "fpu_huge_err",	//17
    "fpu_ine_err",	//16

    "reserved",		//15
    "reserved",		//14
    "reserved",		//13
    "reserved",		//12

    "reserved",		//11
    "reserved",		//10
    "reserved",		//9
    "etm check point 0 err ",		//8

    "reserved",		//7
    "reserved",		//6
    "reserved",		//5
    "pc_limit",		//4

    "stack overflow err",		//3
    "div0_err",		//2
    "illegal_err",	//1
    "misalign_err",	//0
};

static char *const    hcore_emu_msg0[32] = {
    "cpu3 instruction fetch   hmem excption  ",		    //31
    "cpu2 instruction fetch   hmem excption  ",		    //30
    "cpu1 instruction fetch   hmem excption  ",		    //29
    "cpu0 instruction fetch   hmem excption  ",		    //28
    "cpu3 read    hmem excption  ",    //27
    "cpu2 read    hmem excption  ",    //26
    "cpu1 read    hmem excption  ",    //25
    "cpu0 read    hmem excption  ",    //24
    "cpu3 write   hmem excption  ",		    //23
    "cpu2 write   hmem excption  ",		    //22
    "cpu1 write   hmem excption  ",		    //21
    "cpu0 write   hmem excption  ",		    //20
    "reserved",	//19
    "reserved",	//18
    "reserved",	    //17
    "reserved",		    //16

    "reserver ",		    //15
    "reserved",		    //14
    "reserved",	    //13
    "reserver ",		    //12
    "reserved",		    //11
    "reserved",	    //10
    "reserved",	    //9
    "fir access  hmem excption  ",         //8
    "eq  access  hmem excption  ",         //7
    "fft access hmem excption  ",		            //6
    "bt access  hmem excption  ",         //5
    "axi read hmem excption  ",	//4
    "axi write hmem excption  ",	//3
    "lg1 access hmem excption  ",	//2
    "lg0 access hmem excption  ",	    //1
    "sdtap access hmem excption  ",		    // 0

};


static char *const  hcore_emu_msg1[32] = {
    "cpu3 instruction fetch   mmu excption  ",		    //31
    "cpu2 instruction fetch   mmu excption  ",		    //30
    "cpu1 instruction fetch   mmu excption  ",		    //29
    "cpu0 instruction fetch   mmu excption  ",		    //28
    "cpu3 read    mmu excption  ",    //27
    "cpu2 read    mmu excption  ",    //26
    "cpu1 read    mmu excption  ",    //25
    "cpu0 read    mmu excption  ",    //24
    "cpu3 write   mmu excption  ",		    //23
    "cpu2 write   mmu excption  ",		    //22
    "cpu1 write   mmu excption  ",		    //21
    "cpu0 write   mmu excption  ",		    //20
    "reserved",	//19
    "reserved",	//18
    "reserved",	    //17
    "reserved",		    //16

    "reserver ",		    //15
    "reserved",		    //14
    "reserved",	    //13
    "reserver ",		    //12
    "reserved",		    //11
    "reserved",	    //10
    "reserved",	    //9
    "fir access  mmu excption  ",         //8
    "eq  access  mmu excption  ",         //7
    "fft access mmu excption  ",		  //6
    "bt access  mmu excption  ",         //5
    "axi read mmu excption  ",	//4
    "axi write mmu excption  ",	//3
    "lg1 access mmu excption  ",	//2
    "lg0 access mmu excption  ",	    //1
    "sdtap access mmu excption  ",		    // 0
};


static char *const    hcore_emu_msg2[32] = {
    "reserved",		    //31
    "reserved",    //30
    "reserved",		    //29
    "reserved",		    //28
    "reserved",		    //27
    "reserved",	    //26
    "reserved",	    //25
    "reserved",	        //24
    "reserved",         //23
    "reserved",         //22
    "reserved",         //21
    "reserved",		            //20
    "reserved",	//19
    "reserved",	//18
    "reserved",	    //17
    "reserved",		    //16

    "reserver",		    //15
    "reserver",    //14
    "reserver",		    //13
    "reserver",		    //12
    "reserved",		    //11
    "reserved",	    //10
    "reserved",	    //9
    "reserved",         //8
    "reserved",         //7
    "dcu_write invalid",         //6
    "dcu_emu_err",		            //5
    "ilock_err",	//4
    "mpu_err",	//3
    "csfr_read invalid",	//2
    "csfr_write invalid",	    //1
    "hsb emu err ",		    // 0
};

static char *const hsb_emu_msg0[32] = {
    "reserved",		    //31
    "reserved",    //30
    "reserved",		    //29
    "reserved",		    //28
    "reserved",		    //27
    "reserved",	    //26
    "reserved",	    //25
    "reserved",	        //24
    "reserved",         //23
    "reserved",         //22
    "reserved",         //21
    "reserved",		            //20
    "reserved",	//19
    "reserved",	//18
    "reserved",	    //17
    "reserved",		    //16

    "reserved",		    //15
    "reserved",    //14
    "reserved",		    //13
    "reserved",		    //12
    "reserved",		    //11
    "reserved",	    //10
    "reserved",	    //9
    "reserved",	        //8
    "anc ram access error",         //7
    "sbc ram access error",         //6
    "watchdog time out",         //5
    "cpu read axi reserved memory ",	    //4
    "cpu write axi reserved memory ",	    //3
    "cpu read hsb sfr reserved memory ",	    //2
    "cpu write hsb sfr reserved memory ",	    //1
    "lsb emu excption",		    //0
} ;




static char *const lsb_emu_msg0[32] = {

    "reserved",		    //31
    "reserved",    //30
    "reserved",		    //29
    "reserved",		    //28
    "reserved",		    //27
    "reserved",	    //26
    "reserved",	    //25
    "reserved",	        //24
    "reserved",         //23
    "reserved",         //22
    "reserved",         //21
    "reserved",		            //20
    "reserved",	//19
    "reserved",	//18
    "reserved",	    //17
    "reserved",		    //16
    "reserved",		    //15
    "reserved",    //14
    "reserved",		    //13
    "reserved",		    //12
    "reserved",		    //11
    "reserved",	    //10
    "reserved",	    //9
    "reserved",	        //8
    "reserved",         //7
    "reserved",	    //6
    "reserved",	    //5
    "reserved",	        //4
    "reserved",         //3
    "reserved",         //2
    "cpu read lsb sfr reserved memory ",	    //1
    "cpu write lsb sfr reserved memory ",	    //0
} ;


static char *const  icache_emu_msg[32] = {
    "reserver ",            //31
    "reserver ",            //30
    "reserver ",            //29
    "reserver ",            //28
    "reserver ",            //27
    "reserver ",            //26
    "reserver ",            //25
    "reserver ",            //24
    "reserver ",            //23
    "reserver ",            //22
    "reserver ",            //21
    "reserver ",            //20
    "reserver ",            //19
    "reserver ",            //18
    "reserver ",            //17
    "reserver ",            //16
    "reserver ",            //15
    "reserver ",            //14
    "reserver ",            //13
    "reserver ",            //12
    "reserver ",            //11
    "reserver ",            //10
    "reserver ",            //9
    "reserver ",            //8
    "reserver ",            //7
    "reserver ",            //6
    "icache emu error -> icmd_lock_err",  //5
    "icache emu error -> icmd_wkst_err", //4
    "icache emu error -> islv_inv     ", //3
    "icache emu error -> ireq_rack_inv", //2
    "icache emu error -> iway_lock_err", //1
    "icache emu error -> iway_rhit_err", //2

};

static char *const  dcache_emu_msg[32] = {
    "reserver ",            //31
    "reserver ",            //30
    "reserver ",            //29
    "reserver ",            //28
    "reserver ",            //27
    "reserver ",            //26
    "reserver ",            //25
    "reserver ",            //24
    "reserver ",            //23
    "reserver ",            //22
    "reserver ",            //21
    "reserver ",            //20
    "reserver ",            //19
    "reserver ",            //18
    "reserver ",            //17
    "reserver ",            //16
    "reserver ",            //15
    "reserver ",            //14
    "reserver ",            //13
    "reserver ",            //12
    "reserver ",            //11
    "reserver ",            //10
    "reserver ",            //9
    "reserver ",            //8
    "reserver ",            //7
    "reserver ",            //6
    "dcache emu error -> icmd_lock_err",  //5
    "dcache emu error -> icmd_wkst_err", //4
    "dcache emu error -> islv_inv     ", //3
    "dcache emu error -> ireq_rack_inv", //2
    "dcache emu error -> iway_lock_err", //1
    "dcache emu error -> iway_rhit_err", //2

};




/********************************* DEBUG PART **********************************/
static void debug_enter_critical()
{
    while (!(_MPU_WR_EN	& BIT(0))) {
        _MPU_WR_SFR_EN;
    }
}

static void debug_exit_critical()
{
    while (_MPU_WR_EN & BIT(0)) {
        _MPU_WR_SFR_EN;
    }
}

static void pc_rang_limit0(void *low_addr, void *high_addr)
{
    _DSP_PC_LIML0 = (u32)low_addr;
    _DSP_PC_LIMH0 = (u32)high_addr;
}

static void pc_rang_limit1(void *low_addr, void *high_addr)
{
    _DSP_PC_LIML1 = (u32)low_addr;
    _DSP_PC_LIMH1 = (u32)high_addr;
}

static void pc_rang_limit2(void *low_addr, void *high_addr)
{
    _DSP_PC_LIML2 = (u32)low_addr;
    _DSP_PC_LIMH2 = (u32)high_addr;
}

static u32 flag1;
static u32 suspend_cnt1;


/******************************** EMU PART  *****************************/
void emu_misalign_enable(u8 enable)
{
    if (enable) {
        _EMU_CON |= BIT(0);
    } else {
        _EMU_CON &= ~BIT(0);
    }
}

void emu_stack_limit_set(u8 mode, u32 limit_l, u32 limit_h)
{
    if (mode) {
        _EMU_SSP_H = limit_h;
        _EMU_SSP_L = limit_l;
        log_debug("SSP_H : 0x%x", _EMU_SSP_H);
        log_debug("SSP_L : 0x%x", _EMU_SSP_L);
    } else {
        _EMU_USP_H = limit_h;
        _EMU_USP_L = limit_l;
        log_debug("USP_H : 0x%x", _EMU_USP_H);
        log_debug("USP_L : 0x%x", _EMU_USP_L);
    }
}

void etm_pc_trace_enable(u8 enable)
{
    if (enable) {
        _ETM_CON |= BIT(0);
    } else {
        _ETM_CON &= ~BIT(0);
    }
}


/**************************************************/

#define STACK_MAGIC     0x5a5a5a5a
extern u32 stack_magic[4];
extern u32 stack_magic0[4] ;
#define TRIGGER         __asm__ volatile ("trigger")

u32 data_magic[16] sec(.data_magic) ;
AT_RAM
static void trace_call_stack()
{
    for (int c = 0; c < CPU_CORE_NUM; c++) {
        sputs("\r\nCPU trace: ");
        sput_u32hex(j32CPU(c)->ETM_PC3);
        sputs("-->");
        sput_u32hex(j32CPU(c)->ETM_PC2);
        sputs("-->");
        sput_u32hex(j32CPU(c)->ETM_PC1);
        sputs("-->");
        sput_u32hex(j32CPU(c)->ETM_PC0);
        sputs("\r\n");
        /* log_debug("CPU%d %x --> %x --> %x --> %x\n", c, */
        /* j32CPU(c)->ETM_PC3, j32CPU(c)->ETM_PC2, */
        /* j32CPU(c)->ETM_PC1, j32CPU(c)->ETM_PC0); */
    }
}

//A:PC4 5    B:USB   C:PB12  D:PB67
#define SDTAP_NON   (0x00)
#define SDTAP_A     (0x01)
#define SDTAP_B     (0x02)
#define SDTAP_C     (0x03)
#define SDTAP_D     (0x04)


#define     SDTAP_SEL   SDTAP_B
/* #define     SDTAP_SEL   SDTAP_NON */

AT_RAM
static void sdtap_init(u32 ch)
{
    JL_IOMAP->CON0 &= ~(BIT(17) | BIT(18));
    switch (ch) {
    case SDTAP_A :
        /* JL_PORTC->DIR |= (BIT(4) | BIT(5)); */
        break;
    case SDTAP_B :
        JL_IOMAP->CON0 |= (BIT(17));
        JL_USB->CON0 = (BIT(0));//USB_PHY_ON
        JL_USB_IO->CON0 = BIT(12); //USB_IO_MODE
        JL_USB_IO->CON0 |= BIT(3) | BIT(2) | (0xf << 8); //dir die
        break;
    case SDTAP_C:
        JL_IOMAP->CON0 |= (BIT(18));
        JL_PORTB->DIR |= (BIT(1) | BIT(2));
        break;
    case SDTAP_D:
        JL_IOMAP->CON0 |= (BIT(17) | BIT(18));
        JL_PORTB->DIR |= (BIT(6) | BIT(7));
        break;
    default :
        return;
    }
    void wait_if_breakpoint(void);
    void the_debug_isr(void);
    request_irq(0, 0, the_debug_isr, 0);
    JL_SDTAP->CON |= (BIT(2) | BIT(0));
    j32CPU(0)->BPCON |= (1u << 31);
}

AT_RAM
void __attribute__((weak)) exception_analyze_release()
{
    P3_PCNT_SET0 = 0xab;
    cpu_reset();
}

extern const int config_asser     ;

sec(.volatile_ram_code)
void debug_exception_analyze(unsigned int *sp)
{
#ifdef CONFIG_RELEASE_ENABLE
    exception_analyze_release();
#else
    asm("trigger ") ;

    /* if (config_asser) { */
    /* TRIGGER; */
    unsigned int reti = sp[16];
    unsigned int rete = sp[17];
    unsigned int retx = sp[18];
    unsigned int rets = sp[19];
    unsigned int psr  = sp[20];
    unsigned int icfg = sp[21];
    unsigned int usp  = sp[22];
    unsigned int ssp  = sp[23];

    int i;

    log_error("_EMU_CON = 0x%x \n", _EMU_CON) ;
    log_error("_EMU_MSG = 0x%x \n", _EMU_MSG) ;
    /* log_error("JL_CEMU->MSG0 = 0x%x ", JL_CEMU->MSG0) ; */
    /* log_error("JL_CEMU->MSG1 = 0x%x ", JL_CEMU->MSG1) ; */
    /* log_error("JL_CEMU->MSG2 = 0x%x ", JL_CEMU->MSG2) ; */
    trace_call_stack();

    sputs("\r\nXX\r\n");
    sput_u32hex(stack_magic[0]);
    if (stack_magic[0] != STACK_MAGIC) {
        sputs("\r\nStack Magic destory!\n");
    }

    sput_u32hex(stack_magic0[0]);
    if (stack_magic0[0] != STACK_MAGIC) {
        sputs("\r\nStack Magic0 destory!\n");
    }

    sput_u32hex(data_magic[0]);
    /* if (data_magic[0] != -1) {
       sputs("\r\nZero Magic destory!");
       } */
    sputs("\r\nusp : ");
    sput_u32hex(usp);
    sputs("\r\nssp : ");
    sput_u32hex(ssp);
    sputs("\r\nsp : ");
    sput_u32hex((u32)sp);
    sputs("\r\nStack : ");
    extern int _stack_end;
    sput_u32hex((u32)&_stack_end);
    sputs("\r\nrets : ");
    sput_u32hex(rets);
    sputs("\r\nreti : ");
    sput_u32hex(reti);
    sputs("\r\nretx : ");
    sput_u32hex(retx);
    sputs("\r\nrete : ");
    sput_u32hex(rete);
    sputs("\r\npsr : ");
    sput_u32hex(psr);
    sputs("\r\nicfg : ");
    sput_u32hex(icfg);


    /* u8 *ptr = (u8 *)reti - 16 ;
       for (i = 0; i < 32; i++) {
       sput_u8hex(ptr[i]);
       } */
    /* sputs("\n--i--- \n "); */
    /* sput_buf((u8 *)reti - 0x20, 0x40); */

    /* sputs("\n--r--- \n "); */
    /* sput_buf((u8 *)rets - 0x20, 0x40); */

    /* ptr = (u8 *)rets - 16 ;
       for (i = 0; i < 32; i++) {
       sput_u8hex(ptr[i]);<]
       } */



#if 0
    sputs("cache tag:\n");
    sput_buf((void *)(0x58000UL), 0x800);

    while (!(JL_DSP->CON & BIT(5))); //cache工作状态：0-工作中，1-没工作
    JL_DSP->CON &= ~BIT(8); /*spi cache disable */
    sputs("cache ram:\n");
    sput_buf((void *)(0x1c000UL), 16 * 1024UL);

    //log_debug("cache buf:\n");
    //	put_buf((void *)(0x10000UL),16*1024UL);
    JL_DSP->CON |= BIT(8);
#endif

    //EMU query sequence
    // CPU -> HCORE -> HSB -> LSB
    for (i = 0; i < 32; i++) {
        if (_EMU_MSG & BIT(i)) {
            sputs("\r\n[0-CPU] emu err msg : ");
            sputs(emu_msg[31 - i]);
        }
    }

    //system
    if (_EMU_MSG & BIT(31)) {
        for (i = 0; i < 32; i++) {
            if (JL_CEMU->MSG0 & BIT(i)) {
                sputs("\r\n[1-HCORE] hmem err msg : ");
                sputs(hcore_emu_msg0[31 - i]);
                if ((i == 4)  || (i == 3)  || (i == 2) || (i == 1)) {
                    sputs("	at dev ");
                    sputs(get_dev_name(JL_CEMU->ID)) ;

                }
            }
        }

        for (i = 0; i < 32; i++) {
            if (JL_CEMU->MSG1 & BIT(i)) {
                sputs("\r\n[1-HCORE] mmu err msg : ");
                sputs(hcore_emu_msg1[31 - i]);
                if ((i == 4) || (i == 3) || (i == 2) || (i == 1)) {
                    sputs("	at dev  ");
                    sputs(get_dev_name(JL_CEMU->ID)) ;
                }
            }
        }

        for (i = 0; i < 32; i++) {
            if (JL_CEMU->MSG2 & BIT(i)) {
                sputs("\r\n[1-HCORE] emu err msg : ");
                sputs(hcore_emu_msg2[31 - i]);
                if ((i == 6) || (i == 3) || (i == 2) || (i == 1)) {
                    sputs("	at dev  ");
                    sputs(get_dev_name(JL_CEMU->ID)) ;
                    if (i == 3) {
                        sputs("	mpu limit : ");
                        sput_u32hex((JL_CEMU->ID & 0xf00) >> 8);
                        sputs(get_dev_name(JL_CEMU->ID & 0xff)) ;
                    }
                }
            }
        }

        //HSB
        if (JL_CEMU->MSG2 & BIT(0)) {
            for (i = 0; i < 32; i++) {
                if (JL_HEMU->MSG0 & BIT(i)) {
                    sputs("\r\n[2-HSB] emu err msg : ");
                    sputs(hsb_emu_msg0[31 - i]);
                    if ((i == 1) || (i == 2) || (i == 3) || (i == 4) || (i == 6) || (i == 7)) {
                        sputs("	at dev  ");
                        sputs(get_dev_name(JL_HEMU->ID));
                    }

                }
            }

            //LSB
            if (JL_HEMU->MSG0 & BIT(0))
                for (i = 0; i < 32; i++) {
                    if (JL_LEMU->MSG0 & BIT(i)) {
                        sputs("\r\n[3-LSB] emu err msg : ");
                        sputs(lsb_emu_msg0[31 - i]);
                        sputs("	at dev  ") ;
                        sputs(get_dev_name(JL_LEMU->ID)) ;
                    }
                }
        }
    }

    if (_EMU_MSG & BIT(30)) {
        for (i = 0 ; i < 32 ; i++) {
            if (_ICU_EMU_MSG & BIT(i)) {
                sputs("\r\n[X-ICACHE] emu err msg : ");
                sputs(icache_emu_msg[31 - i]);
                if (i == 3) {
                    sputs("	at dev ");
                    sputs(get_dev_name(_ICU_EMU_ID)) ;
                }
            }

        }
    }

    if (_EMU_MSG & BIT(29)) {
        for (i = 0 ; i < 32 ; i++) {
            if (_DCU_EMU_MSG & BIT(i)) {
                sputs("\r\n[Y-DCACHE] emu err msg : ");
                sputs(dcache_emu_msg[31 - i]);
                if (i == 3) {
                    sputs("	at dev ");
                    sputs(get_dev_name(_DCU_EMU_ID)) ;
                }
            }

        }
    }



    for (i = 0; i < 16; ++i) {
        sputs("\r\n");
        sput_hex(i, 1);
        sputchar('=');
        sput_u32hex(sp[i]);
    }

    for (i = 0; i < ARRAY_SIZE(stack_magic); i++) {
        if (stack_magic[i] != 0x5a5a5a5a) {
            sputs("\r\nstack_magic");
            sput_u8hex(i);
            sput_u32hex(stack_magic[i]);
        }
    }

    for (i = 0; i < ARRAY_SIZE(stack_magic0); i++) {
        if (stack_magic0[i] != 0x5a5a5a5a) {
            sputs("\r\nstack_magic0");
            sput_u8hex(i);
            sput_u32hex(stack_magic0[i]);
        }
    }

    for (i = 0; i < ARRAY_SIZE(data_magic); i++) {
        if (data_magic[i] != 0xffffffff) {
            sputs("\r\ndata_magic");
            sput_u8hex(i);
            sput_u32hex(data_magic[i]);
        }
    }

#if 0
    extern int data_code_pc_limit_begin,  _data_code_mid, data_code_pc_limit_end;

    int data_code_len = (void *)&data_code_pc_limit_end - (void *)&_data_code_mid;
    u8 *code_ptr = (void *)&_data_code_mid;
    u8 *data_ptr = (u8 *)sizeof(data_magic);

    /* for (i = 0; i < data_code_len; i++) {
       if (code_ptr[i] != data_ptr[i]) {
       sputs("\r\n code_magic");
       sput_u8hex(i);
       sput_u32hex(data_ptr[i]);
       }
       } */

    /* extern int _nv_pre_begin, _nv_pre_end; */

    /* sputs("\r----------data-------- \n"); */
    /* sput_buf((u8 *)&data_code_pc_limit_begin, (u32)((u32)&data_code_pc_limit_end - (u32)&data_code_pc_limit_begin)); */


    /* sputs("\r---------nv--------- \n");
       sput_buf((u8 *)&_nv_pre_begin, (u32)((u32)&_nv_pre_end - (u32)&_nv_pre_begin)); */

    //把ssp, usp 的一部分内容也打印出来
    sputs("\r\nusp = ");
    //sput_u32hex(usp);
    /* sput_buf((u8 *)usp, 2048); */
    sputs("\r\nssp = ");
    //sput_u32hex(ssp);
#endif
    /* sput_buf((u8 *)ssp, 1024); */
    /* ASSERT_R(0); */
    sdtap_init(SDTAP_B);
    while (1);
#endif
    /* } else { */
    /*     exception_analyze_release(); */
    /* } */
}


static void exception_cpu_ex_over_limit_error(void)
{
    //range outside
    TRIGGER;
    TRIGGER;
    void (*foo)(void);
    foo = (void (*)(void))0x18001;
    foo();
}

static void exception_cpu_store_error(void)
{
    log_debug(">>>>exception_cpu_store_error:");
    //cpu_store_rang_limit_set((void *)0x100, (void *)0x104 - 1, 1);

    /* *((u32 *)0x100) = 1; */

    /* TRIGGER; */
    /* TRIGGER; */
    log_debug(">>>>ready\n");
    TRIGGER;
    TRIGGER;
    *((u32 *)0x108) = 1;
    log_debug(">>>>pass\n");
}


__attribute__((noinline)) static int div0_test(int c)
{
    ___trig;
    int a;
    int b = 3;
    a = c;
    b = b - c;
    log_debug(" %d / %d = %d", a, b, a / b);
    ___trig;
    return a;
}


volatile u8  etm_point_w_8  ;// = (volatile u8 *)0x4100001;
volatile u16 etm_point_w_16 ;// = (volatile u16 *)0x4100002;
volatile int etm_point_w_32 ;// = (volatile int *)0x4100004;
volatile u8  etm_point_r_8  ;// = (volatile u8 *)0x4100011;
volatile u16 etm_point_r_16 ;// = (volatile u16 *)0x4100012;
volatile int etm_point_r_32 ;// = (volatile int *)0x4100014;
__attribute__((noinline)) void watch_point_test()
{
    /* for (int c = 0; 1 < 2; c++) {                                                */
    /*     log_debug("etm_con %x %x\r\n", j32CPU(c)->ETM_CON, &j32CPU(c)->ETM_CON);    */
    /*     log_debug("wp0_adrh %x %x\r\n", j32CPU(c)->WP0_ADRH, &j32CPU(c)->WP0_ADRH); */
    /*     log_debug("wp0_adrl %x %x\r\n", j32CPU(c)->WP0_ADRL, &j32CPU(c)->WP0_ADRL); */
    /*     log_debug("wp0_DATH %x %x\r\n", j32CPU(c)->WP0_DATH, &j32CPU(c)->WP0_DATH); */
    /*     log_debug("wp0_DATl %x %x\r\n", j32CPU(c)->WP0_DATL, &j32CPU(c)->WP0_DATL); */
    /* }                                                                            */

    int i = 0;
    while (1) {
        log_debug("cpu id %d\n", current_cpu_id());
        /* __asm__ volatile ("csync");   */
        /* __asm__ volatile ("lockset"); */
        /* __asm__ volatile ("csync");   */
        etm_point_w_32 = i;
        /* __asm__ volatile ("csync"); */
        /* __asm__ volatile("idle");   */
        etm_point_w_16 = i;
        /* __asm__ volatile ("csync"); */
        /* __asm__ volatile("idle");   */
        etm_point_w_8 = i;
        /* __asm__ volatile ("csync"); */
        /* __asm__ volatile ("lockclr"); */
        /* __asm__ volatile ("csync"); */

        /* __asm__ volatile ("csync"); */
        __asm__ volatile("idle");
        log_debug("i= %d\n", i);
        i++;
        /* os_time_dly(100); */

        log_debug("cpu id %d\n", current_cpu_id());
        /* __asm__ volatile ("csync"); */
        /* __asm__ volatile("idle");   */
        log_debug("%s() %d\n\r", __func__, etm_point_r_8);
        /* __asm__ volatile ("csync"); */
        /* __asm__ volatile("idle");   */
        log_debug("%s() %d\n\r", __func__, etm_point_r_16);
        /* __asm__ volatile ("csync"); */
        /* __asm__ volatile("idle");   */
        log_debug("%s() %d\n\r", __func__, etm_point_r_32);
    }
}


void debug_sfr_test()
{
    log_debug("debug sfr test");
#if 0
    //pc limit test
//#define PC_LIMIT_ADDR  	0x10102a1  //flash 地址限制
#define PC_LIMIT_ADDR  	0x50000	//ram地址限制
    ((void(*)(void))PC_LIMIT_ADDR)();
#endif

//非对齐访问
#if 0
    //miss aline
    volatile u32 *p = (volatile u32 *)0x4001;
    //编译器会编译为对齐
    *p = 0x12345678;
    int v = 0x12345678;
    log_debug("%x %x  ", p, *p) ;
    asm("trigger ") ;
#if 0
    //write
    __asm__ volatile("[%0]= %1"
                     ::"r"(p), "r"(v));
#endif

#if 1
    //read
    __asm__ volatile("%0 = [%1]"
                     : "=r"(v)
                     : "r"(p));
    log_debug("mis aline %x\n", *p);
#endif

#endif

//除0异常
#if 0
    //div 0
    int a = 3;
    div0_test(1);
    div0_test(3);  //div0
#endif

#if 0
//非法指令异常
    //u8 data[] = {0x80, 0x00}; //rts
    /* u8 data[] = {0x07, 0x00};   //进入SDTAP中断的指令,SDTAP的中断优先级大于异常中断 */
    /* u8 data[] = {0x30, 0xe4, 0x00, 0x00};  //该指令会触发simd异常 */
    u8 data[] = {0xAB, 0xCD, 0xDF, 0xCB};  //乱编的指令
    typedef void (*FUN)(void);
    FUN fun = (FUN)data;
//    memcpy(fun, data, 2);
    ___trig;
    fun();
#endif

#if 0
//触发dsp取指令地址出错, if_err, 需要同时开inv_expt和if_inv才能触发
//#define PC_ADDR  	0x25000  //reserved 跳转到区域
#define PC_ADDR  	0xB0000
    ((void(*)(void))PC_ADDR)();

#endif

#if 0
//触发dsp取数据地址出错, of_err, 需要同时开inv_expt和of_inv才能触发
//#define DATA_ADDR  	0x25000  //reserved 跳转到区域
#define DATA_ADDR  	0x50000  //reserved 跳转到区域
//#define DATA_ADDR  	0x3000000  //跳转到flash mapping外的区域不会触发异常
    u32 *p = (u32 *)DATA_ADDR;
    log_debug("data = 0x%x", *p);
#endif

#if 1
    extern int data_code_pc_limit_begin,  _data_code_mid, data_code_pc_limit_end;
//触发dsp写数据地址出错, 需要同时开inv_expt和ex_inv才能触发
#define DATA_ADDR  	&data_code_pc_limit_begin  //reserved 跳转到区域
//#define DATA_ADDR  	0x3000000  //跳转到flash mapping外的区域不会触发异常
    u32 *p = (u32 *)DATA_ADDR;
    *p = 0x12345678;
#endif
}

static void mpu_privilege(int idx, u8 type, u8 did, u8 x, u8 r, u8 w)
{
    u8 i;

    switch (type) {
    case 'C':
        log_info("C[%d] : x(%d) : r(%d) : w(%d) ", idx, x, r, w);
        JL_MPU->CON[idx] |= ((x) ? MPU_XEN : 0) | ((r) ? MPU_REN : 0) | ((w) ? MPU_WEN : 0);
        break;
    case 'P':
        log_info("P[%d] : r(%d) : w(%d) ", idx, r, w);
        JL_MPU->CON[idx] |= ((r) ? MPU_PREN : 0) | ((w) ? MPU_PWEN : 0);
        break;
    case '0':
    case '1':
    case '2':
        i = type - '0';
        log_info("PID[%d][%d] : did 0x%x : r(%d) : w(%d) ", idx, i, did, r, w);
        JL_MPU->CID[idx] |= MPU_IDx_cfg(i, did);
        JL_MPU->CON[idx] |= MPU_IDx_pen(i, ((r) ? 1 : 0), ((w) ? 1 : 0));
        break;
    default:
        break;
    }
}

// [MPU format]
// begin | end | privilege | inv | pid0_privilege |
int __parser(int idx, const char *format, va_list argptr)
{
    u8 type = 0;
    u8 did = 0;
    u8 privilege = 0;

    while (*format) {
        switch (*format) {
        case 'C':
            mpu_privilege(idx, type, did, privilege & BIT(2), privilege & BIT(1), privilege & BIT(0));
            did = 0;
            type = *format;
            privilege = 0;
            break;
        case 'P':
            mpu_privilege(idx, type, did, privilege & BIT(2), privilege & BIT(1), privilege & BIT(0));
            did = 0;
            type = *format;
            privilege = 0;
            break;
        case '0':
        case '1':
        case '2':
        case '3':
            mpu_privilege(idx, type, did, privilege & BIT(2), privilege & BIT(1), privilege & BIT(0));
            did = va_arg(argptr, int);
            type = *format;
            privilege = 0;
            break;

        case 'w':
            privilege |= BIT(0);
            break;
        case 'r':
            privilege |= BIT(1);
            break;
        case 'x':
            privilege |= BIT(2);
            break;

        default:
            break;
        }
        format++;
    }
    mpu_privilege(idx, type, did, privilege & BIT(2), privilege & BIT(1), privilege & BIT(0));

    return 1;
}

#define MPU_MAX_SIZE    4

void mpu_set(int idx, u32 begin, u32 end, u32 inv, const char *format, ...)
{
    if (idx > (MPU_MAX_SIZE - 1)) {
        log_error("MPU idx overflow %d", idx);
        return;
    }

    debug_enter_critical();

    JL_MPU->BEG[idx] = begin;
    JL_MPU->END[idx] = end;

    JL_MPU->CON[idx] = 0;
    JL_MPU->CID[idx] = 0;

    if (inv) {
        JL_MPU->CON[idx] = MPU_INV ;
    }

    va_list argptr;
    va_start(argptr, format);

    __parser(idx, format, argptr);

    va_end(argptr);

    debug_exit_critical();

    //echo
    log_info("JL_MPU->CON[%d] 0x%x", idx, JL_MPU->CON[idx]);
    log_info("JL_MPU->CID[%d] 0x%x", idx, JL_MPU->CID[idx]);
    log_info("JL_MPU->BEG[%d] 0x%x", idx, JL_MPU->BEG[idx]);
    log_info("JL_MPU->END[%d] 0x%x", idx, JL_MPU->END[idx]);

}


static void mpu_test(void)
{
    u32 begin, end;
    u8 inv;
    u32 cpu_privilege;
    u32 pid_privilege[4];

    //case 1 - 范围内允许CPU (r+x)，允许所有外设(w + r)，除ID0 外设；
    begin           = 0x0;
    end             = 0x1000;
    inv             = 0;        //
    mpu_set(0, begin, end, inv, "CxrwPrw0rw", get_dev_id("DBG_USB"));

    log_info("JL_MPU->CON[0] 0x%x", JL_MPU->CON[0]);
    log_info("JL_MPU->CID[0] 0x%x", JL_MPU->CID[0]);
    log_info("JL_MPU->BEG[0] 0x%x", JL_MPU->BEG[0]);
    log_info("JL_MPU->END[0] 0x%x", JL_MPU->END[0]);

    //case 2 - 范围内仅外设ID0(w + r)；
    begin           = 0x0;
    end             = 0x1000;
    inv             = 0;        //
    mpu_set(1, begin, end, inv, "0rw", get_dev_id("DBG_BT"));

    log_info("JL_MPU->CON[1] 0x%x", JL_MPU->CON[1]);
    log_info("JL_MPU->CID[1] 0x%x", JL_MPU->CID[1]);
    log_info("JL_MPU->BEG[1] 0x%x", JL_MPU->BEG[1]);
    log_info("JL_MPU->END[1] 0x%x", JL_MPU->END[1]);

    //case 3 - 范围内CPU(r)，外设ID1(w + r)；
    begin           = 0x0;
    end             = 0x1000;
    inv             = 0;        //
    mpu_set(2, begin, end, inv, "Cr1wr", get_dev_id("DBG_BT"));

    log_info("JL_MPU->CON[2] 0x%x", JL_MPU->CON[2]);
    log_info("JL_MPU->CID[2] 0x%x", JL_MPU->CID[2]);
    log_info("JL_MPU->BEG[2] 0x%x", JL_MPU->BEG[2]);
    log_info("JL_MPU->END[2] 0x%x", JL_MPU->END[2]);

    //case 4 - 范围内仅外设ID2(r)，ID3(w + r)；
    begin           = 0x0;
    end             = 0x1000;
    inv             = 0;        //
    mpu_set(3, begin, end, inv, "0r2rw", get_dev_id("DBG_AAC"), get_dev_id("DBG_SBC"));

    log_info("JL_MPU->CON[3] 0x%x", JL_MPU->CON[3]);
    log_info("JL_MPU->CID[3] 0x%x", JL_MPU->CID[3]);
    log_info("JL_MPU->BEG[3] 0x%x", JL_MPU->BEG[3]);
    log_info("JL_MPU->END[3] 0x%x", JL_MPU->END[3]);

    while (1);
}




void ram_limit_set(u32 num, u32 begin, u32 end, u32 con, u32  id, u32 inv)
{
    if (num > 3) {
        log_error("MPU idx overflow %d", num);
        return;
    }

    debug_enter_critical();

    JL_MPU->BEG[num] = begin ;
    JL_MPU->END[num]  = end ;
    JL_MPU->CID[num] = id ;

    if (inv == 1) {
        JL_MPU->CON[num] |= con | MPU_INV ;
    } else {

        JL_MPU->CON[num] |= con ;
    }

    debug_exit_critical();
}

static void mpu_diasble(void)
{
    /* if (idx > (MPU_MAX_SIZE - 1)) {
        log_error("MPU idx overflow %d", idx);
        return;
    } */
    u8 idx;

    debug_enter_critical();

    for (idx = 0; idx < (MPU_MAX_SIZE - 1); idx++) {
        JL_MPU->CON[idx] = 0;
        JL_MPU->CID[idx] = 0;

        JL_MPU->BEG[idx] = 0;
        JL_MPU->END[idx] = 0;

    }
    debug_exit_critical();

    //echo
    log_info("JL_MPU->CON[%d] 0x%x", idx, JL_MPU->CON[idx]);
    log_info("JL_MPU->CID[%d] 0x%x", idx, JL_MPU->CID[idx]);
    log_info("JL_MPU->BEG[%d] 0x%x", idx, JL_MPU->BEG[idx]);
    log_info("JL_MPU->END[%d] 0x%x", idx, JL_MPU->END[idx]);

}
void ram_protect_close(void)
{
    pc_rang_limit0((void *)0, (void *)0x1ffffff);
    pc_rang_limit1((void *)0, (void *)0x1ffffff);
    mpu_diasble();
    //cpu_protect_ram_suspend();
    //prp_protect_ram_suspend();
    //perpheral_access_invalid_enable(0);
    //peripheral_cpu_access_invalid_enable(0);
    //cpu_load_data_invalid_enable(0);
    //cpu_store_invalid_enable(0);
    //cpu_load_ins_invalid_enable(0);
    //icache_exception_enable(0);
    //peripheral_access_MMU_enable(0);
    //pc_rang_limit0((void *)0, (void *)0x1ffffff);
    _EMU_CON = 0x3;

}

#define EMU_MISALIGN_EN     (1 << 0)
#define EMU_ILLEGAL_EN      (1 << 1)
#define EMU_DIV0_EN         (1 << 2)
#define EMU_SP_OV_EN        (1 << 3)
#define EMU_PC_LIMIT_EN     (1 << 4)
#define EMU_WP0_ERR_EN      (1 << 8)
#define EMU_FPU_INE_EN      (1 << 16)
#define EMU_FPU_HUGE_EN     (1 << 17)
#define EMU_FPU_TINY_EN     (1 << 18)
#define EMU_FPU_INF_EN      (1 << 19)
#define EMU_FPU_INV_EN      (1 << 20)
#define EMU_DCU_ERR_EN      (1 << 29)
#define EMU_ICU_ERR_EN      (1 << 30)
#define EMU_SYS_ERR_EN      (1 << 31)

#define ETM_PC_TRACE_EN     (1 << 0)
#define ETM_WP_ERR_MODE     (1 << 1)
#define ETM_WP_DREV         (1 << 8)

void debug_init()
{
    /*void cpu_test_init(void);
    cpu_test_init();*/
    log_debug(">>> debug init:");
    //debug_mod_en(0) ;
    //CPU execute protect
    _EMU_CON = EMU_MISALIGN_EN | EMU_ILLEGAL_EN | EMU_DIV0_EN | EMU_PC_LIMIT_EN |
#ifdef CONFIG_FLOAT_DEBUG_ENABLE
               EMU_FPU_INV_EN  | EMU_FPU_INF_EN  |
#endif
               EMU_SYS_ERR_EN;
    log_info("_EMU_CON : 0x%x", _EMU_CON);

    _ETM_CON = ETM_PC_TRACE_EN;
    log_info("_ETM_CON : 0x%x", _ETM_CON);

    log_info("JL_CEMU->CON0 : 0x%x", JL_CEMU->CON0);
    log_info("JL_CEMU->CON1 : 0x%x", JL_CEMU->CON1);
    log_info("JL_CEMU->CON2 : 0x%x", JL_CEMU->CON2);

    log_info("JL_HEMU->CON0 : 0x%x", JL_HEMU->CON0);
    log_info("JL_HEMU->CON1 : 0x%x", JL_HEMU->CON1);
    log_info("JL_HEMU->CON2 : 0x%x", JL_HEMU->CON2);

    log_info("JL_LEMU->CON0 : 0x%x", JL_LEMU->CON0);
    log_info("JL_LEMU->CON1 : 0x%x", JL_LEMU->CON1);
    log_info("JL_LEMU->CON2 : 0x%x", JL_LEMU->CON2);

    log_info("_ICU_EMU_CON : 0x%x", _ICU_EMU_CON);
    log_info("_DCU_EMU_CON : 0x%x", _DCU_EMU_CON);

    ////maskrom code range
    pc_rang_limit2((void *)0x100000, (void *)0x110000);

    //ram code
    pc_rang_limit1(&data_code_pc_limit_begin, &data_code_pc_limit_end);

    //flash code range
    pc_rang_limit0(&text_begin, &text_end);

    //_EMU_CON |= BIT(4) ;

    log_debug("pc_limit0: 0x%x ~ 0x%x",  _DSP_PC_LIML0 & 0x1ffffff, _DSP_PC_LIMH0 & 0x1ffffff);
    log_debug("pc_limit1: 0x%x ~ 0x%x",  _DSP_PC_LIML1 & 0x1ffffff, _DSP_PC_LIMH1 & 0x1ffffff);
    log_debug("pc_limit2: 0x%x ~ 0x%x",  _DSP_PC_LIML2 & 0x1ffffff, _DSP_PC_LIMH2 & 0x1ffffff);

    /* u32 nv_pre_len = (u32)&_nv_pre_end - (u32)&_nv_pre_begin; */
    /* memset((u8 *)&_nv_pre_begin, 0xff, nv_pre_len); */
    /* put_buf((u8 *)&_nv_pre_begin, nv_pre_len); */
    memset(data_magic, 0xff, sizeof(data_magic));
    /* log_debug("CPU STORE  Limit0: 0x%x ~ 0x%x", &_cpu_store_begin, &_cpu_store_end); */
    /* log_debug("PRP STORE  Limit0: 0x%x ~ 0x%x", &_prp_store_begin, &_prp_store_end); */
    //cpu store protect

//"0rw", get_dev_id("DBG_BT")
    //RAM code
    mpu_set(0, (u32)&data_code_pc_limit_begin, (u32)&data_code_pc_limit_end - 1, 0, "Cxrw0rw1rw", get_dev_id("DBG_FFT"), get_dev_id("DBG_EQ"));
//	extern u32 dac_data_begin , dac_data_end ;


//	mpu_set(1, (u32)&dac_data_begin, (u32)&dac_data_end - 1, 0, "Crw0r",get_dev_id("DBG_AUDIO"));
    /* u16 *ptr = (u32 *)0xa01; */

    /* ptr[0] = 1; */


    //MMU
    /* u16 *ptr = (u32 *)0x001; */

    /* ptr[0] = 1; */

    //HMEM
    /* u16 *ptr = (u32 *)0x30000; */
    /* u16 *ptr = (u32 *)0x110000; */

    /* ptr[0] = 1; */

    //CSFR
    /* j32CPU(0)->REV8b[0] = 1; */
    /* JL_DCU1->CON = 1; */

    //HSFR
    /* log_info("JL_PSRAM->CON 0x%x", JL_PSRAM->CON); */
    /* JL_PSRAM->CON = 1; */

    //LSFR
    /* log_info("JL_DMA->PRI0 0x%x", JL_DMA->PRI0); */
    /* JL_DMA->PRI0 = 1; */
    /* log_info("JL_DMA->RESERVED0[0] 0x%x", JL_DMA->RESERVED0[0]); */
    /* JL_DMA->RESERVED0[0] = 1; */


    /* mpu_test(); */

    /* IcuEmuEnable(); */
    /* DcuEmuMessage(); */
}


#if 0   //br23 FPGA debug test

extern void exception_irq_handler();
AT_RAM
___interrupt
void debug_irq(void)
{
    /* sput_u32hex(JL_MPU->MSG); */
    log_debug("JL_MPU->MSG = 0x%x", JL_MPU->MSG);
    log_debug("JL_MPU->PRP_WR_LIMIT_MSG = 0x%x", JL_MPU->PRP_WR_LIMIT_MSG);
    log_debug("JL_MPU->LSB_WR_LIMIT_CH = 0x%x", JL_MPU->LSB_WR_LIMIT_CH);
}
AT_RAM
void my_debug_test(void)
{
    log_debug("\n\nmy_debug_test \n\n");

    /* request_irq(1, 2, exception_irq_handler, 0); */
    request_irq(1, 2, debug_irq, 0);

    log_debug("JL_MPU->MSG = 0x%x\n", JL_MPU->MSG);

//访问cache
#if 0
    //使能
    JL_DSP->CON |= BIT(9) | BIT(8); //使能CACHE
    JL_DSP->CON |= BIT(30);         //使能CACHE中断
    log_debug("JL_DSP->CON = 0x%x\n", JL_DSP->CON);
    //操作
    u8 pp_data;
    u8 *pp;
    pp = (u8 *)0xf8000;
    *pp = 0x5a;
    pp_data = *pp;
#endif

//访问SBC_RAM
#if 0
    //使能
    JL_SBC->CON0 |= BIT(0);
    log_debug("JL_DSP->CON = 0x%x\n", JL_DSP->CON);
    //操作
    u8 pp_data;
    u8 *pp;
    pp = (u8 *)0x30100;
    *pp = 0x5a;
    pp_data = *pp;
#endif

//读空白区
#if 0
    //使能中断
    JL_DSP->CON |= BIT(26);
    JL_DSP->CON |= BIT(24);
    log_debug("JL_DSP->CON = 0x%x\n", JL_DSP->CON);
    //操作
    u32 volatile pp_data = 0;
    u32 volatile *pp;
    pp = (u32 *)0x31000;
    pp_data = (u32) * pp;
#endif

//写空白区
#if 0
    //使能中断
    JL_DSP->CON |= BIT(25);
    JL_DSP->CON |= BIT(24);
    log_debug("JL_DSP->CON = 0x%x\n", JL_DSP->CON);
    //操作
    u8 *pp;
    pp = (u8 *)0x31000;
    *pp = 0x5a;
#endif

//CPU空白区取指令
#if 0
    //使能中断
    JL_DSP->CON |= BIT(27);
    JL_DSP->CON |= BIT(24);
    log_debug("JL_DSP->CON = 0x%x\n", JL_DSP->CON);
    //操作
    ((void(*)(void))0x31000)();
#endif

//外设访问空白区
#if 0
    //使能中断
    JL_DSP->CON |= BIT(28);
    JL_DSP->CON |= BIT(24);
    log_debug("JL_DSP->CON = 0x%x\n", JL_DSP->CON);
    //操作
    JL_SPI2->CON |= BIT(0);
    JL_SPI2->ADR = (u32)0x31000;
    JL_SPI2->CNT = 32;
#endif

//外设读外部memory  打印函数要用AT_RAM的
#if 0
    //关掉spi cache
    JL_DSP->CON &= ~BIT(8);
    //使能中断
    JL_DSP->CON |= BIT(28);
    JL_DSP->CON |= BIT(24);
    sput_u32hex(JL_DSP->CON);
    //操作
    JL_SPI2->CON |= BIT(0);
    JL_SPI2->ADR = (u32)0x1000000;
    JL_SPI2->CNT = 32;
#endif

//CPU在限制范围内或外写数据
#if 0
    u8 test_ram[64];
    JL_MPU->DSP_BF_CON &= ~BIT(10);
    //设置范围
    JL_MPU->DSP_EX_LIMH = (u32)(test_ram + 32);
    JL_MPU->DSP_EX_LIML = (u32)test_ram;
    //使能
    JL_MPU->DSP_BF_CON |= BIT(11);    //在范围内触发
    /* JL_MPU->DSP_BF_CON &= ~BIT(11);   //在范围外触发 */
    JL_MPU->DSP_BF_CON |= BIT(10);
    log_debug("JL_DSP->CON = 0x%x\n", JL_DSP->CON);
    log_debug("JL_MPU->DSP_BF_CON = 0x%x\n", JL_MPU->DSP_BF_CON);
    //操作
    test_ram[0] = 0x5a;
#endif

//CPU在限制范围内或外取指令
#if 0
    //设置范围
    JL_MPU->DSP_PC_LIMH0 = (u32)0x31040;
    JL_MPU->DSP_PC_LIML0 = (u32)0x31020;

    JL_MPU->DSP_PC_LIMH1 = (u32)0x31080;
    JL_MPU->DSP_PC_LIML1 = (u32)0x31060;
    log_debug("JL_DSP->CON = 0x%x\n", JL_DSP->CON);
    log_debug("JL_MPU->DSP_BF_CON = 0x%x\n", JL_MPU->DSP_BF_CON);
#endif

//PRP在限制范围内或外写数据
#if 0
    u8 test_ram[64];
    JL_MPU->DSP_BF_CON &= ~BIT(8);
    //设置范围
    JL_MPU->PRP_EX_LIMH = (u32)(test_ram + 32);
    JL_MPU->PRP_EX_LIML = (u32)test_ram;
    //使能
    JL_MPU->DSP_BF_CON |= BIT(9);    //在范围内触发
    /* JL_MPU->DSP_BF_CON &= ~BIT(9);   //在范围外触发 */
    JL_MPU->DSP_BF_CON |= BIT(8);
    log_debug("JL_DSP->CON = 0x%x\n", JL_DSP->CON);
    log_debug("JL_MPU->DSP_BF_CON = 0x%x\n", JL_MPU->DSP_BF_CON);
    //操作
    JL_SPI2->CON |= BIT(0);
    JL_SPI2->CON |= BIT(12);
    JL_SPI2->ADR = (u32)test_ram;
    JL_SPI2->CNT = 32;
#endif

//看门狗
    while (1);

}
#endif


#else

___interrupt
void debug_irq(void)
{
    asm("trigger") ;
    log_info("debug_irq \r\n") ;
    while (1) {} ;

    /* sput_u32hex(JL_MPU->MSG); */
//    log_debug("JL_MPU->MSG = 0x%x\n", JL_MPU->MSG);
//    log_debug("JL_MPU->PRP_WR_LIMIT_MSG = 0x%x\n", JL_MPU->PRP_WR_LIMIT_MSG);
//    log_debug("JL_MPU->LSB_WR_LIMIT_CH = 0x%x\n", JL_MPU->LSB_WR_LIMIT_CH);
}

void debug_init()
{

    request_irq(1, 2, debug_irq, 0);

}

void exception_analyze(unsigned int *sp)
{


}

void debug_disable()
{
    _EMU_CON = 0;
}
#endif

