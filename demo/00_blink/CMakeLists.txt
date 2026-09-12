cmake_minimum_required(VERSION 3.16)

get_filename_component(PROJECT_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
project(${PROJECT_NAME} NONE)

set(SDK_PATH "$ENV{AC63N_SDK_PATH}")

# SDK 子模块汇总目录（jl_register 上报到本目录）
set(JL_REGISTER_TOP_DIR "${CMAKE_SOURCE_DIR}")

# ---- 导入 SDK：解析路径 + 选择平台 + 载入平台编译参数 ----
# include(${CMAKE_CURRENT_LIST_DIR}/ac63n_sdk_import.cmake)
include(${SDK_PATH}/tools/cmake/ac63n_sdk_import.cmake)

# ---- 加载 SDK 各模块（逐级 add_subdirectory）----
add_subdirectory(${SDK_PATH}/core        ${CMAKE_CURRENT_BINARY_DIR}/sdk/core)
add_subdirectory(${SDK_PATH}/peripheral  ${CMAKE_CURRENT_BINARY_DIR}/sdk/peripheral)
add_subdirectory(${SDK_PATH}/components  ${CMAKE_CURRENT_BINARY_DIR}/sdk/components)
add_subdirectory(${SDK_PATH}/misc        ${CMAKE_CURRENT_BINARY_DIR}/sdk/misc)

# ---- 收集所有子模块注册的结果 ----
get_property(JL_SOURCES      DIRECTORY ${JL_REGISTER_TOP_DIR} PROPERTY JL_SOURCES)
get_property(JL_INCLUDE_DIRS DIRECTORY ${JL_REGISTER_TOP_DIR} PROPERTY JL_INCLUDE_DIRS)
get_property(JL_LIBS         DIRECTORY ${JL_REGISTER_TOP_DIR} PROPERTY JL_LIBS)

# 加上本 demo 的入口（LED 闪烁 main.c）
list(APPEND JL_SOURCES "${CMAKE_CURRENT_LIST_DIR}/main.c")

# ---- 通用编译/链接流程 ----
# （预处理 sdk.ld + 编译 .o + 链接 .elf，封装于 SDK tools/cmake/jl_build_elf.cmake，
#   适用于所有 demo 工程）
include(${SDK_PATH}/tools/cmake/jl_build_elf.cmake)

jl_build_elf(${PROJECT_NAME})