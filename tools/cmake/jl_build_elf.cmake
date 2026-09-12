# =============================================================
# jl_build_elf —— 通用编译/链接流程（适用于各 demo 工程）
#
# 封装了：
#   1. 预处理生成链接脚本 sdk.ld（build/pre/sdk.ld）
#   2. 逐个编译源文件为 .o
#   3. 用 lto-wrapper 链接生成 <工程名>.elf
#
# 调用示例（demo 顶层 CMakeLists）：
#   include(${JL_SDK_ROOT}/tools/cmake/jl_build_elf.cmake)
#   jl_build_elf(00_blink)
#
# 依赖（须在调用前于顶层作用域设定）：
#   JL_SDK_ROOT  JL_CHIP_PLATFORM  JL_CORE
#   TC_BIN  TC_SYS_INC  TC_SYS_LIB
#   JL_SOURCES  JL_INCLUDE_DIRS  JL_LIBS
#   CFLAGS_ARCH  DEFINES  JL_LINK_OPT
# =============================================================

function(jl_build_elf _target)
  # ---- 头文件搜索路径（各子模块目录 + 系统头文件）----
  set(_inc)
  foreach(_d IN LISTS JL_INCLUDE_DIRS)
    list(APPEND _inc "-I${_d}")
  endforeach()
  list(APPEND _inc "-I${TC_SYS_INC}")

  # ---- 预处理生成链接脚本 sdk.ld（build/pre/sdk.ld）----
  set(_pre       "${CMAKE_CURRENT_BINARY_DIR}/pre")
  set(_obj_dir   "${CMAKE_CURRENT_BINARY_DIR}/obj")
  set(_ld        "${_pre}/sdk.ld")
  set(_map       "${_pre}/sdk.map")
  set(_objs_txt  "${_pre}/objs.txt")
  set(_maskrom   "${JL_SDK_ROOT}/peripheral/${JL_CHIP_PLATFORM}/maskrom")

  file(MAKE_DIRECTORY ${_pre} ${_obj_dir})

  add_custom_command(
    OUTPUT  ${_ld}
    COMMAND ${TC_BIN}/clang -target ${JL_CORE} -I${_maskrom} -D__LD__ -E -P
            ${JL_SDK_ROOT}/core/${JL_CHIP_PLATFORM}/scripts/sdk_ld.c -o ${_ld}
    DEPENDS ${JL_SDK_ROOT}/core/${JL_CHIP_PLATFORM}/scripts/sdk_ld.c
            ${_maskrom}/maskrom_stubs.ld
    COMMENT "GENERATE ${_ld}"
    VERBATIM)

  # ---- 预编译：拷贝对应平台的 isd_config.ini 到 build/pre ----
  set(_extra_depends)
  set(_isd_src "${JL_SDK_ROOT}/pre-build/${JL_CHIP_PLATFORM}/isd_config.ini")
  if(EXISTS "${_isd_src}")
    set(_isd_out "${_pre}/isd_config.ini")
    add_custom_command(
      OUTPUT  ${_isd_out}
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_isd_src}" "${_isd_out}"
      DEPENDS "${_isd_src}"
      COMMENT "COPY ${_isd_src}"
      VERBATIM)
    list(APPEND _extra_depends ${_isd_out})
  endif()

  # ---- 逐个源文件编译为 .o ----
  set(_objs)
  foreach(_src IN LISTS JL_SOURCES)
    get_filename_component(_name "${_src}" NAME)
    string(REPLACE ".S" ".o" _objname "${_name}")
    string(REPLACE ".c" ".o" _objname "${_objname}")
    set(_o "${_obj_dir}/${_objname}")
    add_custom_command(
      OUTPUT  ${_o}
      COMMAND ${TC_BIN}/clang ${CFLAGS_ARCH} ${DEFINES} ${_inc} -c ${_src} -o ${_o}
      DEPENDS ${_src}
      COMMENT "CC ${_src}"
      VERBATIM)
    list(APPEND _objs ${_o})
  endforeach()

  string(REPLACE ";" "\n" _objs_nl "${_objs}")
  file(WRITE ${_objs_txt} "${_objs_nl}\n")

  # ---- 链接参数：平台专用插件参数 + 公共外壳 ----
  set(_lflags
    ${JL_LINK_OPT}
    --sort-common
    --dont-complain-call-overflow
    --start-group
    ${JL_LIBS}
    --end-group
    -T ${_ld}
    -M=${_map})
  set(_sys_libs
    -L ${TC_SYS_LIB}
    ${TC_SYS_LIB}/libm.a
    ${TC_SYS_LIB}/libc.a
    ${TC_SYS_LIB}/libm.a
    ${TC_SYS_LIB}/libcompiler-rt.a)

  set(_elf "${CMAKE_CURRENT_BINARY_DIR}/${_target}.elf")

  # ---- 最终链接：用 lto-wrapper 生成 <工程名>.elf ----
  add_custom_command(
    OUTPUT  ${_elf}
    COMMAND ${TC_BIN}/lto-wrapper -o ${_elf} @${_objs_txt} ${_lflags} ${_sys_libs}
    DEPENDS ${_objs} ${_ld}
    COMMENT "LINK ${_elf}"
    VERBATIM)

  add_custom_target(${_target} ALL DEPENDS ${_elf} ${_extra_depends})

  # ---- make flash：调用 jlisp 生成下载固件 <工程名>_dl.elf ----
  #   命令：<jlisp> flash <工程名>.elf --isd_config <pre/isd_config.ini> --out <工程名>_dl.elf
  set(_jlisp "${JL_SDK_ROOT}/tools/linux/jlisp")
  if(EXISTS "${_jlisp}")
    set(_flash_out "${CMAKE_CURRENT_BINARY_DIR}/${_target}_dl.elf")
    add_custom_target(flash
      COMMAND ${_jlisp} flash ${_elf}
              --isd_config "${_pre}/isd_config.ini"
              --out "${_flash_out}"
      DEPENDS ${_elf} ${_extra_depends}
      COMMENT "FLASH ${_flash_out}"
      VERBATIM)
    message(STATUS "[jl_build_elf] 已启用 make flash（jlisp=${_jlisp}）")
  else()
    message(STATUS "[jl_build_elf] 未找到 ${_jlisp}，跳过 make flash（可运行 tools/linux/init_toolchain.sh 下载）")
  endif()
endfunction()