#include "includes.h"

extern char __VERSION_BEGIN[];
extern char __VERSION_END[];

const char *sdk_version(void)
{
    return "AC63_GP_MCU_v1.4.1_2025-08-06";
}

int app_version_check()
{
    char *version;

    printf("================= SDK Version    %s     ===============\n", sdk_version());
    for (version = __VERSION_BEGIN; version < __VERSION_END;) {
        printf("%s\n", version);
        version += strlen(version) + 1;
    }
    puts("=======================================\n");

    return 0;
}

