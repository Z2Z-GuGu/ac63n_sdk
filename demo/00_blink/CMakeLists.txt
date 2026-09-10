cmake_minimum_required(VERSION 3.16)

# =============================================================
# LED 闪烁工程（顶层 CMakeLists）—— 支持多芯片平台
#
# 三层结构：
#   顶层 CMakeLists.txt                 导入 SDK、组装编译/链接
#    ├── ac63n_sdk_import.cmake         路径解析 + 平台选择 + jl_register
#   └── core/<平台>/scripts/platform_*.cmake   芯片专用编译参数
#
#   构建： cd 00_blink && mkdir build && cd build && cmake .. && make
#      cmake ..                     # 弹菜单选择平台
#      cmake -DJL_PLATFORM_SEL=0 .. # 直接指定 0=AC632N / 1=AC628N
#   产出： build/00_blink.elf
# =============================================================

project(00_blink NONE)

# SDK 子模块汇总目录（jl_register 上报到本目录）
set(JL_REGISTER_TOP_DIR "${CMAKE_SOURCE_DIR}")

# ---- 导入 SDK：解析路径 + 选择平台 + 载入平台编译参数 ----
include(${CMAKE_CURRENT_LIST_DIR}/ac63n_sdk_import.cmake)

# 本 demo 提供强 main()（main.c），屏蔽 boot 库中的 int main()
list(APPEND DEFINES -DJL_DISABLE_BOOT_MAIN)

# ---- 加载 SDK 各模块（逐级 add_subdirectory）----
add_subdirectory(${JL_SDK_ROOT}/core        ${CMAKE_CURRENT_BINARY_DIR}/sdk/core)
add_subdirectory(${JL_SDK_ROOT}/peripheral  ${CMAKE_CURRENT_BINARY_DIR}/sdk/peripheral)
add_subdirectory(${JL_SDK_ROOT}/components  ${CMAKE_CURRENT_BINARY_DIR}/sdk/components)
add_subdirectory(${JL_SDK_ROOT}/misc        ${CMAKE_CURRENT_BINARY_DIR}/sdk/misc)

# ---- 收集所有子模块注册的结果 ----
get_property(JL_SOURCES      DIRECTORY ${JL_REGISTER_TOP_DIR} PROPERTY JL_SOURCES)
get_property(JL_INCLUDE_DIRS DIRECTORY ${JL_REGISTER_TOP_DIR} PROPERTY JL_INCLUDE_DIRS)
get_property(JL_LIBS         DIRECTORY ${JL_REGISTER_TOP_DIR} PROPERTY JL_LIBS)

# 加上本 demo 的入口（LED 闪烁 main.c）
list(APPEND JL_SOURCES "${CMAKE_CURRENT_LIST_DIR}/main.c")

# ---- 头文件搜索路径（各子模块目录 + 系统头文件）----
set(INCLUDES)
foreach(_d IN LISTS JL_INCLUDE_DIRS)
  list(APPEND INCLUDES "-I${_d}")
endforeach()
list(APPEND INCLUDES "-I${TC_SYS_INC}")

# ---- 预处理生成链接脚本 sdk.ld（build/pre/sdk.ld）----
set(PRE_DIR   "${CMAKE_CURRENT_BINARY_DIR}/pre")
set(OBJ_DIR   "${CMAKE_CURRENT_BINARY_DIR}/obj")
set(SDK_LD    "${PRE_DIR}/sdk.ld")
set(SDK_MAP   "${PRE_DIR}/sdk.map")
set(OBJS_TXT  "${PRE_DIR}/objs.txt")
set(MASKROM_DIR "${JL_SDK_ROOT}/peripheral/${JL_CHIP_PLATFORM}/maskrom")

file(MAKE_DIRECTORY ${PRE_DIR} ${OBJ_DIR})

add_custom_command(
  OUTPUT  ${SDK_LD}
  COMMAND ${TC_BIN}/clang -target ${JL_CORE} -I${MASKROM_DIR} -D__LD__ -E -P
          ${JL_SDK_ROOT}/core/${JL_CHIP_PLATFORM}/scripts/sdk_ld.c -o ${SDK_LD}
  DEPENDS ${JL_SDK_ROOT}/core/${JL_CHIP_PLATFORM}/scripts/sdk_ld.c
          ${MASKROM_DIR}/maskrom_stubs.ld
  COMMENT "GENERATE ${SDK_LD}"
  VERBATIM)

# ---- 逐个源文件编译为 .o ----
set(ALL_OBJS)
foreach(src IN LISTS JL_SOURCES)
  get_filename_component(_name "${src}" NAME)
  string(REPLACE ".S" ".o" _objname "${_name}")
  string(REPLACE ".c" ".o" _objname "${_objname}")
  set(_obj "${OBJ_DIR}/${_objname}")
  add_custom_command(
    OUTPUT  ${_obj}
    COMMAND ${TC_BIN}/clang ${CFLAGS_ARCH} ${DEFINES} ${INCLUDES} -c ${src} -o ${_obj}
    DEPENDS ${src}
    COMMENT "CC ${src}"
    VERBATIM)
  list(APPEND ALL_OBJS ${_obj})
endforeach()

string(REPLACE ";" "\n" ALL_OBJS_NL "${ALL_OBJS}")
file(WRITE ${OBJS_TXT} "${ALL_OBJS_NL}\n")

# ---- 链接参数：平台专用插件参数 + 公共外壳 ----
set(LFLAGS
  ${JL_LINK_OPT}
  --sort-common
  --dont-complain-call-overflow
  --start-group
  ${JL_LIBS}
  --end-group
  -T ${SDK_LD}
  -M=${SDK_MAP})

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

add_custom_target(00_blink ALL DEPENDS ${OUT_ELF})