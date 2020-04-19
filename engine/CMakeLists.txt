cmake_minimum_required(VERSION 3.9)
add_library(engine src/jack_client.cc src/fx_plugin_handler.cc src/controller.cc)
target_include_directories(engine PRIVATE
  ${Boost_INCLUDE_DIRS}
  ${JACK_INCLUDE_DIR}
  inc
  ../inc
  ../config_backend/inc
)
