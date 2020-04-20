cmake_minimum_required(VERSION 3.9)
project(awesome-fxd VERSION 0.0.1 DESCRIPTION "Audio effect engine")

include(FetchContent)
FetchContent_Declare(
  beasthttp
  GIT_REPOSITORY https://github.com/0xdead4ead/BeastHttp.git
  GIT_TAG        dev
)
FetchContent_MakeAvailable(beasthttp)

FetchContent_Declare(
  nlohmann
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        develop
)
FetchContent_MakeAvailable(nlohmann)

add_executable(awesome-fxd src/main.cc)
target_compile_features(awesome-fxd PRIVATE cxx_std_17)
target_include_directories(
  awesome-fxd
  PRIVATE
  inc
  engine/inc
  config_backend/inc
  config_backend/src
  ${beasthttp_SOURCE_DIR}/BeastHttp/include
)

add_definitions(-DBOOST_ASIO_SEPARATE_COMPILATION=1)
add_definitions(-DBOOST_BEAST_SEPARATE_COMPILATION=1)
add_library(
  asio STATIC
  ${beasthttp_SOURCE_DIR}/BeastHttp/static/asio.cpp
)

add_library(
  beast STATIC
  ${beasthttp_SOURCE_DIR}/BeastHttp/static/beast.cpp
)

add_subdirectory(engine)
add_subdirectory(config_backend)
add_subdirectory(effects)
find_package(Boost 1.70 COMPONENTS system program_options filesystem thread REQUIRED)
find_library(JACK_LIBRARY NAMES jack)
target_link_libraries(awesome-fxd
  engine
  config_backend
  dl
  pthread
  ${Boost_LIBRARIES}
  ${JACK_LIBRARY}
  asio
  beast
  nlohmann_json::nlohmann_json
)

