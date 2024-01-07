#!/bin/bash

while true
do
  read -t 10 var

  if [ $var = "GET_DATA" ]
  then 
    cpu_info=$(top -b -i -n 1 | awk 'NR==8 {print $9}')
    current_time=$(date +%s.%N)
    
    echo -e "$cpu_info\n$current_time"
  fi
done
