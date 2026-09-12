# =============================================================
# ac63n_sdk_import.cmake —— 第二层：SDK 与工具链导入、平台选择
#
# 由项目根目录 CMakeLists.txt 以 include() 方式导入（共享作用域），
# 完成以下职责：
#   1) SDK 根目录取本文件相对路径（tools/cmake 的上级上级）；
#      工具链位置依据 AC63N_TOOLS_PATH/toolchain（或回退 SDK 内默认目录）；
#   2) cmake .. 时弹出平台选择菜单，确定
#       JL_PLATFORM_NAME / JL_CHIP_PLATFORM / JL_CORE；
#   3) 定义 SDK 子模块注册函数 jl_register()；
#   4) include 所选芯片平台的三层编译配置文件
#       ${JL_SDK_ROOT}/core/<平台>/scripts/platform_<平台>.cmake。
# =============================================================

# -------------------------------------------------------------
# 一、SDK 根目录（本文件位于 tools/cmake/，其上级上级即 SDK 根）
#     JL_TOOLCHAIN_ROOT 依赖 JL_CORE，需待平台选定后在第三部分计算。
# -------------------------------------------------------------
set(JL_SDK_ROOT "${CMAKE_CURRENT_LIST_DIR}/../..")   # SDK 根目录

# -------------------------------------------------------------
# 二、芯片平台选择（每次 cmake .. 弹菜单询问）
# -------------------------------------------------------------
set(JL_PLATFORM_IDS   "0"  "1")
set(JL_PLATFORM_NAMES "AC632N" "AC628N")
set(JL_PLATFORM_PLATS "bd19"  "br34")
set(JL_PLATFORM_CORES "q32s"  "pi32v2")

set(JL_PLATFORM_SEL "ASK" CACHE STRING
    "芯片平台序号（0=AC632N, 1=AC628N, ASK=每次询问）")

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

function(jl_platform_apply _sel _out_ok)
  set(_ok FALSE)
  list(LENGTH JL_PLATFORM_IDS _apply_cnt)
  math(EXPR _apply_max "${_apply_cnt}-1")
  foreach(_i RANGE ${_apply_max})
    list(GET JL_PLATFORM_IDS   ${_i} _a_idx)
    list(GET JL_PLATFORM_NAMES ${_i} _a_name)
    list(GET JL_PLATFORM_PLATS ${_i} _a_plat)
    list(GET JL_PLATFORM_CORES ${_i} _a_core)
    if("${_sel}" STREQUAL "${_a_idx}")
      set(JL_PLATFORM_NAME   "${_a_name}" PARENT_SCOPE)
      set(JL_CHIP_PLATFORM   "${_a_plat}" PARENT_SCOPE)
      set(JL_CORE            "${_a_core}" PARENT_SCOPE)
      set(_ok TRUE)
      break()
    endif()
  endforeach()
  set(${_out_ok} ${_ok} PARENT_SCOPE)
endfunction()

# 未显式指定（ASK 或为空）时，读取一次选择
if(JL_PLATFORM_SEL STREQUAL "ASK" OR JL_PLATFORM_SEL STREQUAL "")
  jl_platform_menu()
  execute_process(
    COMMAND bash -c "read -rp '    请输入序号: ' sel && echo \"\$sel\""
    OUTPUT_VARIABLE _sel
    RESULT_VARIABLE _rc
  )
  string(STRIP "${_sel}" _sel)
  jl_platform_apply("${_sel}" _ok)
  if(NOT _ok)
    message(WARNING "[sdk_import] 未读取到有效序号，回退默认平台 AC632N (bd19/q32s)")
    set(JL_PLATFORM_NAME   "AC632N")
    set(JL_CHIP_PLATFORM   "bd19")
    set(JL_CORE            "q32s")
  endif()
else()
  # 命令行 / 缓存已指定具体序号（如 -DJL_PLATFORM_SEL=0）
  jl_platform_apply("${JL_PLATFORM_SEL}" _ok)
  if(NOT _ok)
    message(FATAL_ERROR "[sdk_import] 无效的 JL_PLATFORM_SEL=${JL_PLATFORM_SEL}，"
                        "请使用 0（AC632N）或 1（AC628N）")
  endif()
endif()

# 导出平台/内核环境变量（供外围辅助脚本/自定义命令使用）
set(ENV{JL_CHIP_PLATFORM} ${JL_CHIP_PLATFORM})
set(ENV{JL_CORE}          ${JL_CORE})

# -------------------------------------------------------------
# 三、工具链路径（依赖已选定的 JL_CORE）+ 存在性检查
# -------------------------------------------------------------
if(DEFINED ENV{AC63N_TOOLS_PATH} AND NOT "$ENV{AC63N_TOOLS_PATH}" STREQUAL "")
  set(JL_TOOLCHAIN_ROOT "$ENV{AC63N_TOOLS_PATH}/toolchain/${JL_CORE}")
else()
  set(JL_TOOLCHAIN_ROOT "${JL_SDK_ROOT}/tools/linux/toolchain/${JL_CORE}")
endif()

set(TC_BIN     "${JL_TOOLCHAIN_ROOT}/bin")
set(TC_SYS_INC "${JL_TOOLCHAIN_ROOT}/include")
set(TC_SYS_LIB "${JL_TOOLCHAIN_ROOT}/lib")

if(NOT EXISTS "${TC_BIN}/clang")
  message(FATAL_ERROR "[sdk_import] 未找到工具链 ${JL_TOOLCHAIN_ROOT}\n"
                      "请先执行 source ${JL_SDK_ROOT}/tools/linux/init_toolchain.sh 下载/导入工具链")
endif()

message(STATUS "[sdk_import] 平台 = ${JL_PLATFORM_NAME}, 芯片 = ${JL_CHIP_PLATFORM}, 内核 = ${JL_CORE}")
message(STATUS "[sdk_import] JL_SDK_ROOT       = ${JL_SDK_ROOT}")
message(STATUS "[sdk_import] JL_TOOLCHAIN_ROOT = ${JL_TOOLCHAIN_ROOT}")

# -------------------------------------------------------------
# 四、子模块注册辅助函数（供 SDK 各叶子子模块 CMakeLists 调用）
# -------------------------------------------------------------
function(jl_register _name)
  if(NOT JL_REGISTER_TOP_DIR)
    message(FATAL_ERROR "jl_register 需在顶层 CMakeLists 中先设置 JL_REGISTER_TOP_DIR")
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

# -------------------------------------------------------------
# 五、加载所选芯片平台的编译配置（第三层）
# -------------------------------------------------------------
set(_platform_cmake "${JL_SDK_ROOT}/core/${JL_CHIP_PLATFORM}/scripts/platform_${JL_CHIP_PLATFORM}.cmake")
if(NOT EXISTS "${_platform_cmake}")
  message(FATAL_ERROR "[sdk_import] 未找到平台编译配置 ${_platform_cmake}")
endif()
include("${_platform_cmake}")

# 本 demo 提供强 main()（main.c），屏蔽 boot 库中的 int main()
list(APPEND DEFINES -DJL_DISABLE_BOOT_MAIN)