

cmake_minimum_required(VERSION 3.25) # specifies minimum required version of cmake 
project(nokia_extensible_system_monitoring_framework) # defines project name 
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # define where binaries should go 

add_executable(node.out src/computing_node/node.c)
add_executable(monitor.out src/system_monitor/monitor.c)

