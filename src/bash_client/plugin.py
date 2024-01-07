import subprocess
import sys

PLUGIN_ID = 1

def execute_command(command):
    result = subprocess.run(command, shell=True, capture_output=True, text=True)
    return result.stdout.strip()

def process_file(guid, node):
    file_path = "parametry.txt" 
    with open(file_path, "r", encoding='utf-8-sig') as file:
        lines = file.readlines()

    for line in lines:
        parts = line.split(' ', 3)
        file_num = int(parts[0])
        type=str(parts[1])
        parameter_name = str(parts[2])
        command = parts[3].strip()

        if file_num == guid:
            result = execute_command(command)
            parameter, time = result.split(' ')
            
            print(f"{{Headers:{{{node},{PLUGIN_ID},Opis_DATA{{{parameter_name}: {type}}},LIST[{{{time},{parameter}}}]}}}}")
            break;

def main():
    guid = int(sys.argv[1])
    node = int(sys.argv[2])
    process_file(guid, node)

if __name__ == "__main__":
    main()
