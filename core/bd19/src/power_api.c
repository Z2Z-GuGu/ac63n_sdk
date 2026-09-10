#include "typedef.h"
#include "asm/power_interface.h"
#include "asm/wdt.h"

extern u8 sys_low_power_request;
extern u32 lowpower_usec;

void sys_power_down(u32 usec)
{
    if (usec == -2) {
        wdt_close();
    }

    if (!sys_low_power_request) {
        lowpower_usec = usec;
        low_power_sys_request(NULL);
        wdt_clear();
    }
}

void sys_softoff()
{
    power_set_soft_poweroff();
}

void lowpower_mode(u8 mode)
{
    if (mode == 0) {
        sys_power_down(-2);
    } else if (mode == 1) {
        sys_softoff();
    }
}

