#!/bin/bash

# Pobieranie informacji o zużyciu pamięci RAM i czasie wykonania testu
memory_info=$(free -m | awk 'NR==2 {print $3}')
current_time=$(date +%s.%N)


# Przekazywanie wartości do programu w C za pomocą potoku
echo -e "$memory_info\n$current_time" | ./formatowanie.out
