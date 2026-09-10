# =============================================================
# core/<平台>：该芯片的头部头文件 + 启动代码
#
# 注意：
#   - scripts/sdk_ld.c 是链接脚本的“源文件”，由 demo 顶层 CMakeLists
#     预处理生成 sdk.ld（build/pre/sdk.ld），因此这里不把它当作普通源文件
#     交给 jl_register 编译。
#   - 芯片 IO 映射头文件（io_imap.h / io_omap.h）已归到外设 peripheral/<平台>/gpio
#     目录下，由 gpio 子模块的 jl_register(gpio) 登记其头文件路径，AC632N.h 通过
#     该路径引用，因此此处不再声明。
# =============================================================
# 芯片头部头文件目录（AC632N.h / cpu.h / includes.h ...）
set_property(DIRECTORY ${JL_REGISTER_TOP_DIR} APPEND PROPERTY JL_INCLUDE_DIRS
  "${CMAKE_CURRENT_LIST_DIR}/include")

# 启动代码
set_property(DIRECTORY ${JL_REGISTER_TOP_DIR} APPEND PROPERTY JL_SOURCES
  "${CMAKE_CURRENT_LIST_DIR}/startup/startup.S")