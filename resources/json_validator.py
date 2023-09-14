import json
import re
import sys

def remove_comments(json_string):
    pattern = r'(?<!http:)//.*?$|\'(?:\\.|[^\\\'])*\'|"(?:\\.|[^\\"])*"'
    regex = re.compile(pattern, re.DOTALL | re.MULTILINE)
    def _replacer(match):
        s = match.group(0)
        if s.startswith('/'):
            return ""
        return s
    return regex.sub(_replacer, json_string)

def count_json_keys(file_path):
    try:
        with open(file_path, 'r') as file:
            json_string = file.read()
            json_string = remove_comments(json_string)
            try:
                data = json.loads(json_string)
                key_count = len(data.keys())
                return key_count
            except json.JSONDecodeError as e:
                # Handle JSON decoding errors
                print(f"Error: in JSON file at line {e.lineno}, column {e.colno}: {e.msg}")
                exit(1)
    except FileNotFoundError:
        print(f"Error: File '{file_path}' not found.")
        exit(1)

# Check if the JSON file path is provided as a command-line argument
if len(sys.argv) < 2:
    print("Error: No file provided")
    exit(1)

# Extract the JSON file path from the command-line arguments
json_file_path = sys.argv[1]

# Call the function and print the result
keys_count = count_json_keys(json_file_path)
if keys_count is None or keys_count == 0:
    print(f"No keys found in '{json_file_path}' (empty json?)")
    exit(1)
    
print(f"Parsing '{json_file_path}' successful, no problems found")
