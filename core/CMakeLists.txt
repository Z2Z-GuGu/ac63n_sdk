# =============================================================
# SDK core：CPU / 启动 / 内核 相关（不直接操作具体外设寄存器）
# 通过 JL_CHIP_PLATFORM 选择平台子目录
# =============================================================
# 公共头文件（typedef、bsp_malloc 等与芯片无关）
set_property(DIRECTORY ${JL_REGISTER_TOP_DIR} APPEND PROPERTY JL_INCLUDE_DIRS
  "${CMAKE_CURRENT_LIST_DIR}/common/include")

# 平台子目录（依据 cross-platform 变量 JL_CHIP_PLATFORM）
add_subdirectory(${JL_CHIP_PLATFORM})