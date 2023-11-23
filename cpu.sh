#!/usr/bin/env bash

# Pobieranie informacji o zużyciu procesora i czasie wykonania testu
cpu_info=$(top -b -n 1 | awk '/%Cpu/{print $2}')
current_time=$(date +%s.%N)

# Przekazywanie wartości do programu w C za pomocą potoku
echo -e "$cpu_info\n$current_time" 
