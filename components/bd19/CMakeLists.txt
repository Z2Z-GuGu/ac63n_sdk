# =============================================================
# components/<平台>：遍历该平台下所有功能子模块并逐个 add_subdirectory
# 每个子模块的 CMakeLists.txt 只需调用一次 jl_register(<模块名>)
# =============================================================
file(GLOB _cp_entries "${CMAKE_CURRENT_LIST_DIR}/*")
foreach(_cp_e IN LISTS _cp_entries)
  if(IS_DIRECTORY "${_cp_e}" AND EXISTS "${_cp_e}/CMakeLists.txt")
    get_filename_component(_cp_n "${_cp_e}" NAME)
    add_subdirectory("${_cp_n}")
  endif()
endforeach()