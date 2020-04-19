cmake_minimum_required(VERSION 3.9)
project(awesome-fxd VERSION 0.0.1 DESCRIPTION "Audio effect engine")
add_executable(awesome-fxd src/main.cc)
target_compile_features(awesome-fxd PRIVATE cxx_std_17)
target_include_directories(awesome-fxd PRIVATE inc engine/inc config_backend/inc)
add_subdirectory(engine)
add_subdirectory(config_backend)
add_subdirectory(effects)
find_package(Boost 1.64.0 COMPONENTS system program_options filesystem REQUIRED)
find_library(JACK_LIBRARY NAMES jack)
target_link_libraries(awesome-fxd
  engine
  config_backend
  dl
  pthread
  ${Boost_LIBRARIES}
  ${JACK_LIBRARY}
)

