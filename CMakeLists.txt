

cmake_minimum_required(VERSION 3.25) # specifies minimum required version of cmake 
project(nokia_extensible_system_monitoring_framework) # defines project name 
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # define where binaries should go 

add_executable(client.out src/client.c)
add_executable(server.out src/server.c)

