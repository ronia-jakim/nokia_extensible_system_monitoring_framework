#!/bin/bash

node="$1"
plugin_id="$2"

memory=$(free -m | awk 'NR==2 {print $3}')
timee=$(date +'%H:%M:%S')
cpu=$(top -b -n 1 | awk '/%Cpu/{print $2}')
timee2=$(date +'%H:%M:%S')

echo "{Headers:{$node,$plugin_id,Opis_DATA[{CPU: float},{RAM: float}],LIST[{$timee,$memory},{$timee2,$cpu}]}}"

