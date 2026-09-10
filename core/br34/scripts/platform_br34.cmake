# =============================================================
# platform_br34.cmake —— AC638N（br34 / pi32v2）编译与链接参数
#
# 第三层：特定芯片编译选项文件，由 ac63n_sdk_import.cmake 选定
# 平台后 include。公共链接外壳由顶层 CMakeLists 组装。
#
# 提供变量：
#   DEFINES        预定义宏
#   CFLAGS_ARCH    编译参数
#   JL_LINK_OPT    链接用平台专用插件参数
# =============================================================

set(DEFINES
  -DAC628N
  -DSUPPORT_MS_EXTENSIONS
  -D__GCC_PI32V2__
  -DCONFIG_OS_ENABLE=0
  -DCONFIG_FREE_RTOS_ENABLE
  -DCONFIG_RELEASE_ENABLE
  -DCONFIG_MMU_ENABLE
  -D__SHELL__)

set(CFLAGS_ARCH
  -target pi32v2
  -mcpu=r3
  -integrated-as
  -flto
  -Wuninitialized
  -Wno-invalid-noreturn
  -fno-common
  -Oz
  -g
  -fallow-pointer-null
  -fprefer-gnu-section
  -Wno-shift-negative-value
  -Werror=implicit-function-declaration
  -Werror=macro-redefined
  -Werror=return-type
  -Werror=int-conversion
  -Werror=incompatible-pointer-types
  -Werror=undef
  -fms-extensions)

# 链接平台专用参数（不含 --start-group / -T / -M 公共外壳）
set(JL_LINK_OPT
  --plugin-opt=-pi32v2-always-use-itblock=false
  --plugin-opt=-enable-ipra=true
  --plugin-opt=-pi32v2-merge-max-offset=4096
  --plugin-opt=-pi32v2-enable-simd=true
  --plugin-opt=mcpu=r3
  --plugin-opt=-enable-movable-region=true
  --plugin-opt=-movable-region-section-prefix=movable.slot.
  --plugin-opt=-global-merge-on-const
  --plugin-opt=-inline-threshold=10
  --plugin-opt=-inline-normal-into-special-section=true
  --plugin-opt=-dont-used-symbol-list=malloc,free,sprintf,printf,puts,putchar
  --plugin-opt=save-temps
  --plugin-opt=-pi32v2-enable-rep-memop
  --plugin-opt=mcpu=r3
  --plugin-opt=-mattr=+fprev1)