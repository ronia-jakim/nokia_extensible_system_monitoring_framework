import subprocess
import sys

PLUGIN_ID = 1

def execute_command(command):
    result = subprocess.run(command, shell=True, capture_output=True, text=True)
    return result.stdout.strip()

def process_file(guid, node):
    file_path = "parametry.txt" 
    with open(file_path, "r", encoding='utf-8-sig') as file:
        command_template = file.read().strip()
    command = command_template.replace('{node}', str(node))
    command = command.replace('{plugin_id}', str(PLUGIN_ID))
    result = execute_command(command)
    print(result)

def main():
    guid = int(sys.argv[1])
    node = int(sys.argv[2])
    process_file(guid, node)

if __name__ == "__main__":
    main()

