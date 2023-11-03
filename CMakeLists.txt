

cmake_minimum_required(VERSION 3.25) # specifies minimum required version of cmake 
project(nokia_extensible_system_monitoring_framework) # defines project name 

add_executable(node computing_node/node.c)
add_executable(monitor system_monitor/monitor.c)

