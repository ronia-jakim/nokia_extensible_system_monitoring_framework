#!/usr/bin/python3

import subprocess

PLUGIN_ID = 1

def execute_command(command):
    result = subprocess.run(command, shell=True, stdout=subprocess.PIPE)
    return result.stdout.decode()

while True:
    s = input()

    cmd = r''' echo "$(free -m | awk 'NR==2 {print $3}')" "$(date +'%H:%M:%S')" '''
    type = "float"

    if s == "GET_DATA":
        result = execute_command(cmd)

        parameter, time = result.split(' ')
        parameter_name = "CPU"
            

        print(f"{{Headers:{{{69},{PLUGIN_ID},Opis_DATA{{{parameter_name}: {type}}},LIST[{{{time},{parameter}}}]}}}}")
