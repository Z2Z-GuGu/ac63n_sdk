cmake_minimum_required(VERSION 3.16)

# =============================================================
# LED 闪烁工程（CMake 构建，支持多芯片平台）
#   构建： cd 00_blink && mkdir build && cd build && cmake .. && make
#   产出： build/<工程名>.elf
#
#   平台选择（经纬度见下方 "芯片平台选择"）：
#       cmake ..                      # 弹菜单，让用户输入序号
#       cmake -DJL_PLATFORM_SEL=0 ..  # 直接指定 0=AC632N，跳过询问
#
#   SDK / 工具链路径优先取系统环境变量（由 tools/linux/init_toolchain.sh
#   写入 ~/.bashrc）：
#       AC63N_SDK_PATH       -> SDK 根目录
#       AC63N_TOOLCHAIN_PATH -> 工具链根目录（本工程再拼接 /${JL_CORE}）
#   环境变量缺失时回退为相对本工程位置推导，不硬编码绝对路径。
# =============================================================

project(00_blink NONE)

# -------------------------------------------------------------
# 芯片平台选择
#   默认每次 configure 都弹出菜单询问（cache 保持 ASK）；
#   命令行用 -DJL_PLATFORM_SEL=<序号> 传入时固定，不再询问。
# -------------------------------------------------------------
# 平台表：四个平行列表，按索引一一对应（避免分号嵌套展开问题）
set(JL_PLATFORM_IDS   "0"  "1")
set(JL_PLATFORM_NAMES "AC632N" "AC628N")
set(JL_PLATFORM_PLATS "bd19"  "br34")
set(JL_PLATFORM_CORES "q32s"  "pi32v2")

# 默认 ASK：每次都询问；传入具体序号则跳过询问
set(JL_PLATFORM_SEL "ASK" CACHE STRING
    "芯片平台序号（0=AC632N, 1=AC628N, ASK=每次询问）")

# 打印菜单
function(jl_platform_menu)
  list(LENGTH JL_PLATFORM_IDS _menu_cnt)
  math(EXPR _menu_max "${_menu_cnt}-1")
  message("")
  message("===========================================")
  message("  请选择芯片平台")
  foreach(_i RANGE ${_menu_max})
    list(GET JL_PLATFORM_IDS   ${_i} _m_idx)
    list(GET JL_PLATFORM_NAMES ${_i} _m_name)
    message("     ${_m_idx} - ${_m_name}")
  endforeach()
  message("===========================================")
endfunction()

# 根据序号匹配平台目录与工具链（ok=FALSE 表示序号无匹配）
function(jl_platform_apply _sel _out_ok)
  set(_ok FALSE)
  list(LENGTH JL_PLATFORM_IDS _apply_cnt)
  math(EXPR _apply_max "${_apply_cnt}-1")
  foreach(_i RANGE ${_apply_max})
    list(GET JL_PLATFORM_IDS   ${_i} _a_idx)
    list(GET JL_PLATFORM_PLATS ${_i} _a_plat)
    list(GET JL_PLATFORM_CORES ${_i} _a_core)
    if("${_sel}" STREQUAL "${_a_idx}")
      set(JL_CHIP_PLATFORM "${_a_plat}" PARENT_SCOPE)
      set(JL_CORE          "${_a_core}" PARENT_SCOPE)
      set(_ok TRUE)
      break()
    endif()
  endforeach()
  set(${_out_ok} ${_ok} PARENT_SCOPE)
endfunction()

if(JL_PLATFORM_SEL STREQUAL "ASK" OR NOT JL_PLATFORM_SEL)
  # --- 交互模式：弹菜单并从终端读一次选择 ---
  jl_platform_menu()
  execute_process(
    COMMAND bash -c "read -rp '    请输入序号: ' sel && echo \"\$sel\""
    OUTPUT_VARIABLE _sel
    RESULT_VARIABLE _rc
  )
  string(STRIP "${_sel}" _sel)
  jl_platform_apply("${_sel}" _ok)
  if(NOT _ok)
    message(WARNING "[00_blink] 未读取到有效序号，回退默认平台 AC632N (bd19/q32s)")
    set(JL_CHIP_PLATFORM "bd19")
    set(JL_CORE          "q32s")
  endif()
else()
  # --- 命令行 / 缓存已指定具体序号 ---
  jl_platform_apply("${JL_PLATFORM_SEL}" _ok)
  if(NOT _ok)
    message(FATAL_ERROR "[00_blink] 无效的 JL_PLATFORM_SEL=${JL_PLATFORM_SEL}，"
                        "请使用 0（AC632N）或 1（AC628N）")
  endif()
endif()

# 导出平台/内核环境变量（供外围辅助脚本/自定义命令使用）
set(ENV{JL_CHIP_PLATFORM} ${JL_CHIP_PLATFORM})
set(ENV{JL_CORE}          ${JL_CORE})

# -------------------------------------------------------------
# SDK / 工具链路径：优先读系统环境变量，缺失时回退相对推导
# -------------------------------------------------------------
if(DEFINED ENV{AC63N_SDK_PATH} AND NOT "$ENV{AC63N_SDK_PATH}" STREQUAL "")
  set(JL_SDK_ROOT "$ENV{AC63N_SDK_PATH}")
else()
  set(JL_SDK_ROOT "${CMAKE_CURRENT_LIST_DIR}/../..")
endif()

if(DEFINED ENV{AC63N_TOOLCHAIN_PATH} AND NOT "$ENV{AC63N_TOOLCHAIN_PATH}" STREQUAL "")
  set(JL_TOOLCHAIN_ROOT "$ENV{AC63N_TOOLCHAIN_PATH}/${JL_CORE}")
else()
  set(JL_TOOLCHAIN_ROOT "${JL_SDK_ROOT}/tools/linux/toolchain/${JL_CORE}")
endif()

set(JL_REGISTER_TOP_DIR  "${CMAKE_SOURCE_DIR}")
set(TC_BIN               "${JL_TOOLCHAIN_ROOT}/bin")
set(TC_SYS_INC           "${JL_TOOLCHAIN_ROOT}/include")
set(TC_SYS_LIB           "${JL_TOOLCHAIN_ROOT}/lib")

if(NOT EXISTS "${TC_BIN}/clang")
  message(FATAL_ERROR "[00_blink] 未找到工具链 ${JL_TOOLCHAIN_ROOT}\n"
                      "请先执行 source ${JL_SDK_ROOT}/tools/linux/init_toolchain.sh 下载/导入工具链")
endif()
message(STATUS "[00_blink] 平台        = ${JL_CHIP_PLATFORM}, 内核 = ${JL_CORE}")
message(STATUS "[00_blink] JL_SDK_ROOT       = ${JL_SDK_ROOT}")
message(STATUS "[00_blink] JL_TOOLCHAIN_ROOT = ${JL_TOOLCHAIN_ROOT}")

# ---- 子模块注册辅助函数 ----
# 每个 peripheral/components 叶子子模块的 CMakeLists 只需调用一次：
#     jl_register(<模块名>)
# 它会自动收集该目录下 *.c / *.S（源文件）、*.a（预编译库），
# 并把该目录加入头文件搜索路径，最终统一在“注册顶层目录”上汇总。
function(jl_register _name)
  if(NOT JL_REGISTER_TOP_DIR)
    message(FATAL_ERROR "jl_register 需在 demo 顶层 CMakeLists 中先设置 JL_REGISTER_TOP_DIR")
  endif()
  file(GLOB _srcs "${CMAKE_CURRENT_LIST_DIR}/*.c" "${CMAKE_CURRENT_LIST_DIR}/*.S")
  file(GLOB _libs "${CMAKE_CURRENT_LIST_DIR}/*.a")
  set_property(DIRECTORY ${JL_REGISTER_TOP_DIR} APPEND PROPERTY JL_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}")
  foreach(_s IN LISTS _srcs)
    set_property(DIRECTORY ${JL_REGISTER_TOP_DIR} APPEND PROPERTY JL_SOURCES "${_s}")
  endforeach()
  foreach(_l IN LISTS _libs)
    set_property(DIRECTORY ${JL_REGISTER_TOP_DIR} APPEND PROPERTY JL_LIBS "${_l}")
  endforeach()
endfunction()

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
  -target ${JL_CORE}
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

# ---- 加载 SDK 各模块（逐级 add_subdirectory，均在 build/sdk 下生成构建目录）----
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

# ---- 链接参数（与原工程 Makefile 一致）----
set(LFLAGS
  --plugin-opt=-inline-threshold=5
  --plugin-opt=save-temps
  --plugin-opt=-inline-normal-into-special-section=true
  --plugin-opt=-dont-used-symbol-list=malloc,free,sprintf,printf,puts,putchar
  --plugin-opt=-warn-stack-size=256
  --sort-common
  --dont-complain-call-overflow
  --start-group
  ${JL_LIBS}
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

add_custom_target(00_blink ALL DEPENDS ${OUT_ELF})