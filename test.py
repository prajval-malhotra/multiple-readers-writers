import re
from collections import defaultdict

# File path to the log file
file_path = 'out'

# Counters for inserted and removed characters
inserted = defaultdict(int)
removed = defaultdict(int)

# Regex patterns for matching insert and remove lines
insert_pattern = re.compile(r'Insert (\d+) (\w) bytes')
remove_pattern = re.compile(r'Remove (\d+) (\w) bytes')

# Open and read the log file
with open(file_path, 'r') as file:
    for line in file:
        insert_match = insert_pattern.match(line)
        remove_match = remove_pattern.match(line)
        if '#' in line:
            continue
        if insert_match:
            count, char = insert_match.groups()
            inserted[char] += int(count)
        
        if remove_match:
            count, char = remove_match.groups()
            removed[char] += int(count)

# Initialize total differences counter
total_difference = 0

# Compare and print the results
for char in set(inserted.keys()).union(set(removed.keys())):
    inserted_count = inserted[char]
    removed_count = removed[char]
    difference = inserted_count - removed_count
    total_difference += abs(difference)  # Sum absolute differences
    
    print(f"Character: {char}")
    print(f"  Inserted: {inserted_count}")
    print(f"  Removed: {removed_count}")
    print(f"  Difference: {difference}")
    print()

# Print total differences
print(f"Total Difference Across All Characters: {total_difference}")