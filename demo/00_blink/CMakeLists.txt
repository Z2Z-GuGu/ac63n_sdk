cmake_minimum_required(VERSION 3.16)

# =============================================================
# AC632N (bd19 / q32s) 最小 blinky 工程，CMake 构建
#   构建： cd 00_blink && mkdir build && cd build && cmake .. && make
#   产出： build/00_blink.elf
#   说明：
#     - 通过 tool/linux/env.sh 导入工具链环境（JL_CORE=q32s / JL_CHIP_PLATFORM=bd19）
#     - 使用 project(NONE) 跳过 CMake 的 C 编译器探测，因为 q32s/clang+LLVMgold
#       这种 LTO 交叉工具链无法被 CMake 自带的探测流程识别
#     - 预处理生成链接脚本到 build/pre/sdk.ld
#     - 不做后处理（不生成下载脚本 / download 相关文件）
# =============================================================

# ---- SDK 根目录与关键路径 ----
set(SDK_ROOT "${CMAKE_CURRENT_LIST_DIR}/../..")
set(CORE_DIR "${SDK_ROOT}/core/bd19")
set(PRPH_DIR "${SDK_ROOT}/peripheral/bd19")

# ---- 通过 tool/linux/env.sh 导入工具链环境，解析出工具链路径 ----
execute_process(
  COMMAND bash -c "source ${SDK_ROOT}/tool/linux/env.sh >/dev/null 2>&1; printf '%s' $AC632N_TOOLCHAIN_PATH"
  OUTPUT_VARIABLE JL_TOOLCHAIN_PATH
  OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT JL_TOOLCHAIN_PATH)
  message(FATAL_ERROR "[00_blink] 无法通过 tool/linux/env.sh 解析工具链路径")
endif()
message(STATUS "[00_blink] AC632N_TOOLCHAIN_PATH = ${JL_TOOLCHAIN_PATH}")

set(TC_BIN     "${JL_TOOLCHAIN_PATH}/q32s/bin")
set(TC_SYS_INC "${JL_TOOLCHAIN_PATH}/q32s/include")
set(TC_SYS_LIB "${JL_TOOLCHAIN_PATH}/q32s/lib")

# 不启用任何语言，避免 CMake 探测/编译 q32s 编译器
project(00_blink NONE)

# ---- 编译参数（与原工程 bsp/AC632N/Makefile 保持一致）----
set(DEFINES
  -DSUPPORT_MS_EXTENSIONS
  -D__GCC_Q32S__
  -DCONFIG_OS_ENABLE=0
  -DCONFIG_FREE_RTOS_ENABLE
  -DCONFIG_RELEASE_ENABLE
  -DCONFIG_MMU_ENABLE
  -D__SHELL__)

set(CFLAGS_ARCH
  -flto
  -target q32s
  -integrated-as
  -fno-builtin
  -mllvm -inline-threshold=5
  -Oz
  -g
  -O0
  -Os
  -fallow-pointer-null
  -Wincompatible-pointer-types
  -Werror=implicit-function-declaration
  -Werror=macro-redefined
  -Werror=return-type
  -Werror=int-conversion
  -Wundef
  -fprefer-gnu-section
  -Wframe-larger-than=256
  -Wno-empty-body
  -Werror=undef
  -fms-extensions)

set(INCLUDES
  -I${CORE_DIR}/include
  -I${TC_SYS_INC})

# ---- 需要编译的源文件 ----
set(CORE_C_SRC
  ${CORE_DIR}/src/adc.c
  ${CORE_DIR}/src/boot.c
  ${CORE_DIR}/src/charge.c
  ${CORE_DIR}/src/chargestore_hw.c
  ${CORE_DIR}/src/crc16.c
  ${CORE_DIR}/src/debug.c
  ${CORE_DIR}/src/debug_uart.c
  ${CORE_DIR}/src/gpio.c
  ${CORE_DIR}/src/hwi.c
  ${CORE_DIR}/src/iic_eeprom_test.c
  ${CORE_DIR}/src/iic_hw.c
  ${CORE_DIR}/src/iic_soft.c
  ${CORE_DIR}/src/irflt.c
  ${CORE_DIR}/src/key_driver.c
  ${CORE_DIR}/src/mcpwm.c
  ${CORE_DIR}/src/plcnt.c
  ${CORE_DIR}/src/port_wkup.c
  ${CORE_DIR}/src/power_api.c
  ${CORE_DIR}/src/pwm_led.c
  ${CORE_DIR}/src/rtc.c
  ${CORE_DIR}/src/rtc_calculate.c
  ${CORE_DIR}/src/spi.c
  ${CORE_DIR}/src/spi_test.c
  ${CORE_DIR}/src/tick_timer.c
  ${CORE_DIR}/src/timer.c
  ${CORE_DIR}/src/timer_cap.c
  ${CORE_DIR}/src/timer_pwm.c
  ${CORE_DIR}/src/uart.c
  ${CORE_DIR}/src/wdt.c
  ${PRPH_DIR}/board/board_demo.c
  ${PRPH_DIR}/board/test.c
  ${CORE_DIR}/dual_bank_update.c
  ${CORE_DIR}/dual_bank_update_uart.c
  ${CORE_DIR}/msg.c
  ${CORE_DIR}/sdk_version.c
  ${CMAKE_CURRENT_LIST_DIR}/main.c)

set(CORE_ASM_SRC
  ${CORE_DIR}/src/startup.S)

# ---- 预处理生成链接脚本 sdk.ld（build/pre/sdk.ld）----
set(PRE_DIR  "${CMAKE_CURRENT_BINARY_DIR}/pre")
set(OBJ_DIR  "${CMAKE_CURRENT_BINARY_DIR}/obj")
set(SDK_LD  "${PRE_DIR}/sdk.ld")
set(SDK_MAP "${PRE_DIR}/sdk.map")
set(OBJS_TXT "${PRE_DIR}/objs.txt")

file(MAKE_DIRECTORY ${PRE_DIR} ${OBJ_DIR})

add_custom_command(
  OUTPUT  ${SDK_LD}
  COMMAND ${TC_BIN}/clang -target q32s -D__LD__ -E -P
          ${CORE_DIR}/src/sdk_ld.c -o ${SDK_LD}
  DEPENDS ${CORE_DIR}/src/sdk_ld.c
          ${CORE_DIR}/src/maskrom_stubs.ld
  COMMENT "GENERATE ${SDK_LD}"
  VERBATIM)

set(GENERATED_FILES ${SDK_LD})
set(ALL_OBJS)

# ---- 逐个源文件编译为 .o ----
foreach(src ${CORE_C_SRC} ${CORE_ASM_SRC})
  get_filename_component(name ${src} NAME)
  set(obj "${OBJ_DIR}/${name}.o")
  add_custom_command(
    OUTPUT  ${obj}
    COMMAND ${TC_BIN}/clang ${CFLAGS_ARCH} ${DEFINES} ${INCLUDES} -c ${src} -o ${obj}
    DEPENDS ${src}
    COMMENT "CC ${src}"
    VERBATIM)
  list(APPEND ALL_OBJS ${obj})
endforeach()

# 把 .o 列表写入 objs.txt（供 lto-wrapper 的 @file 使用）
string(REPLACE ";" "\n" ALL_OBJS_NL "${ALL_OBJS}")
file(WRITE ${OBJS_TXT} "${ALL_OBJS_NL}\n")

# ---- 链接参数（与原工程 Makefile 一致）----
set(SO_LIBS
  ${CORE_DIR}/liba/printf.a
  ${CORE_DIR}/liba/mkey.a
  ${CORE_DIR}/liba/resfile.a
  ${CORE_DIR}/liba/update.a
  ${CORE_DIR}/liba/vm.a
  ${CORE_DIR}/liba/chargestore.a
  ${CORE_DIR}/src/agreement.a
  ${CORE_DIR}/src/memory.a
  ${CORE_DIR}/src/sfc_flash.a
  ${CORE_DIR}/src/efuse.a
  ${CORE_DIR}/src/clock.a
  ${CORE_DIR}/src/power.a
  ${CORE_DIR}/src/cache.a)

set(LFLAGS
  --plugin-opt=-inline-threshold=5
  --plugin-opt=save-temps
  --plugin-opt=-inline-normal-into-special-section=true
  --plugin-opt=-dont-used-symbol-list=malloc,free,sprintf,printf,puts,putchar
  --plugin-opt=-warn-stack-size=256
  --sort-common
  --dont-complain-call-overflow
  --start-group
  ${SO_LIBS}
  --end-group
  -T ${SDK_LD}
  -M=${SDK_MAP}
  -flto
  --plugin-opt=-inline-threshold=5)

set(L_SYS_LIBS
  -L ${TC_SYS_LIB}
  ${TC_SYS_LIB}/libm.a
  ${TC_SYS_LIB}/libc.a
  ${TC_SYS_LIB}/libm.a
  ${TC_SYS_LIB}/libcompiler-rt.a)

set(OUT_ELF "${CMAKE_CURRENT_BINARY_DIR}/00_blink.elf")

# ---- 最终链接：用 lto-wrapper 生成 00_blink.elf ----
add_custom_command(
  OUTPUT  ${OUT_ELF}
  COMMAND ${TC_BIN}/lto-wrapper -o ${OUT_ELF} @${OBJS_TXT} ${LFLAGS} ${L_SYS_LIBS}
  DEPENDS ${ALL_OBJS} ${SDK_LD}
  COMMENT "LINK ${OUT_ELF}"
  VERBATIM)

add_custom_target(00_blink.elf ALL DEPENDS ${OUT_ELF})