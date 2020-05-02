cmake_minimum_required(VERSION 3.9)
project(effects VERSION 0.0.1 DESCRIPTION "Various DSP effects")

include_directories(../inc)

file(GLOB plugins "src/*.cc")

message("Generating targets for plugins:")
foreach(file ${plugins})
  get_filename_component(plugin ${file} NAME_WE)
  message("  Found " ${plugin})
  add_library(${plugin} SHARED ${file})
  set_target_properties(${plugin} PROPERTIES SOVERSION 1)
  install(TARGETS ${plugin} LIBRARY DESTINATION lib)
  list(APPEND alleffects "${plugin} ")
endforeach()

separate_arguments(alleffects)
add_custom_target(effects DEPENDS ${alleffects})

