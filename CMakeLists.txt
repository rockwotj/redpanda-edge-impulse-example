cmake_minimum_required(VERSION 3.22)

project(
  MyTransform
  VERSION 0.1
  LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -stdlib=libc++")

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(
  redpanda-transform-sdk
  DOWNLOAD_EXTRACT_TIMESTAMP true
  SOURCE_SUBDIR              src/transform-sdk/cpp
  URL                        https://github.com/redpanda-data/redpanda/archive/refs/heads/dev.tar.gz
)

FetchContent_Declare(
  json
  DOWNLOAD_EXTRACT_TIMESTAMP true
  URL                        https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)

FetchContent_MakeAvailable(redpanda-transform-sdk json)

add_executable(
  app
  src/transform.cc
)
target_link_libraries(
  app
  Redpanda::transform_sdk
  nlohmann_json::nlohmann_json
)
# Force POSIX environment for WASI, as the porting logic can't auto detect WASI should be posix
# 
# Also WASI doesn't support CLOCK_PROCESS_CPUTIME_ID, so alias that with CLOCK_MONOTONIC
#
# It would be great if EdgeImpulse's SDK had first class support for WASI :smile:
add_definitions(-DEI_PORTING_POSIX -DCLOCK_PROCESS_CPUTIME_ID=CLOCK_MONOTONIC)
# Add EdgeImpulse SDK
add_subdirectory(my_edge_impulse_project)
