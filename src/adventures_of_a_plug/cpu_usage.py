#!/usr/bin/python3

import subprocess

PLUGIN_ID = 1

def execute_command(command):
    result = subprocess.run(command, shell=True, stdout=subprocess.PIPE)
    return result.stdout.decode()

while True:
    s = input()

    cmd = r'''mpstat| awk '$12 ~ /[0-9.]+/ { print 100-$12,strftime("\n%H:%M:%S") }' '''
    type = "int"

    if s == "GET_DATA":
        result = execute_command(cmd)

        parameter, time = result.split(' ')
        parameter_name = "CPU"
            
        print(f"{{Headers:{{{69},{PLUGIN_ID},Opis_DATA{{{parameter_name}: {type}}},LIST[{{{time},{parameter}}}]}}}}")

