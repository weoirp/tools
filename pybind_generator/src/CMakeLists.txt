cmake_minimum_required(VERSION 2.8)

file(GLOB_RECURSE project_headers
               "*.h"
               "*.hpp")

file(GLOB_RECURSE project_sources
               "*.cc"
               "*.cpp")

set(SRC_LISTS ${project_headers} ${project_sources})

source_group("source" FILES ${SRC_LISTS})                      
  
include_directories(.)

add_executable(${Application} ${SRC_LISTS})

target_link_libraries(${Application} libclang)