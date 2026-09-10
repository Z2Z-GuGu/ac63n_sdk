# =============================================================
# platform_bd19.cmake —— AC632N（bd19 / q32s）编译与链接参数
#
# 第三层：特定芯片编译选项文件，由 ac63n_sdk_import.cmake 选定
# 平台后 include。只负责定义本平台独有的参数，公共链接外壳
# （--start-group / -T / -M 等）交由顶层 CMakeLists 组装。
#
# 提供变量：
#   DEFINES        预定义宏
#   CFLAGS_ARCH    编译参数
#   JL_LINK_OPT    链接用平台专用插件参数
# =============================================================

set(DEFINES
  -DAC632N
  -DSUPPORT_MS_EXTENSIONS
  -D__GCC_Q32S__
  -DCONFIG_OS_ENABLE=0
  -DCONFIG_FREE_RTOS_ENABLE
  -DCONFIG_RELEASE_ENABLE
  -DCONFIG_MMU_ENABLE
  -D__SHELL__)

set(CFLAGS_ARCH
  -target q32s
  -integrated-as
  -flto
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

# 链接平台专用参数（不含 --start-group / -T / -M 公共外壳）
set(JL_LINK_OPT
  --plugin-opt=-inline-threshold=5
  --plugin-opt=save-temps
  --plugin-opt=-inline-normal-into-special-section=true
  --plugin-opt=-dont-used-symbol-list=malloc,free,sprintf,printf,puts,putchar
  --plugin-opt=-warn-stack-size=256
  -flto
  --plugin-opt=-inline-threshold=5)