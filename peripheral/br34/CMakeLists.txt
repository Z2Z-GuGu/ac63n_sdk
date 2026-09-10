# =============================================================
# peripheral/<平台>：遍历该平台下所有外设子模块并逐个 add_subdirectory
# 每个子模块的 CMakeLists.txt 只需调用一次 jl_register(<模块名>)
# =============================================================
file(GLOB _pl_entries "${CMAKE_CURRENT_LIST_DIR}/*")
foreach(_pl_e IN LISTS _pl_entries)
  if(IS_DIRECTORY "${_pl_e}" AND EXISTS "${_pl_e}/CMakeLists.txt")
    get_filename_component(_pl_n "${_pl_e}" NAME)
    add_subdirectory("${_pl_n}")
  endif()
endforeach()