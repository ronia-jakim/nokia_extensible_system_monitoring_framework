#!/bin/bash

while true
do
  read -t 10 var

  if [ $var = "GET_DATA" ]
  then 
    mem_usage=$(free -m | awk 'NR==2 {print $3}')
    mem_total=$(free -m | awk 'NR==2 {print $2}')
    memory_info=$((mem_usage * 100 / mem_total))
    current_time=$(date +%s.%N)

    echo -e "$memory_info\n$current_time"
  fi 
done
