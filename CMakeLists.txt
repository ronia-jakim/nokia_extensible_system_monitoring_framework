cmake_minimum_required(VERSION 3.22) # specifies minimum required version of cmake 
project(nokia_extensible_system_monitoring_framework) # defines project name 
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # define where binaries should go 

add_executable(server.out src/server/server.cpp)
add_executable(bash_client.out src/bash_client/client.c)
add_executable(fork_client.out src/fork_client_adventures/client.c)
