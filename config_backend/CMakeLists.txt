cmake_minimum_required(VERSION 3.9)
add_library(config_backend src/configuration_backend.cc)
target_include_directories(config_backend PRIVATE
  ${Boost_INCLUDE_DIRS}
  inc/
)
