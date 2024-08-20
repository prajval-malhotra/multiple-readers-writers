from collections import defaultdict

def parse_data(log_data):
    inserted = defaultdict(int)
    removed = defaultdict(int)

    for line in log_data.splitlines():
        if "Insert" in line:
            parts = line.split()
            count = int(parts[1])
            char_type = parts[2]
            inserted[char_type] += count
        elif "Remove" in line:
            parts = line.split()
            count = int(parts[1])
            char_type = parts[2]
            removed[char_type] += count

    return inserted, removed

def calculate_difference(inserted, removed):
    all_chars = set(inserted.keys()).union(set(removed.keys()))
    differences = {}

    for char in all_chars:
        inserted_count = inserted[char]
        removed_count = removed[char]
        differences[char] = inserted_count - removed_count

    return differences

def print_results(inserted, removed, differences):
    print("Character\tInserted\tRemoved\tDifference")
    for char in sorted(differences.keys()):
        print(f"{char}\t\t{inserted[char]}\t\t{removed[char]}\t\t{differences[char]}")
    
    total_inserted = sum(inserted.values())
    total_removed = sum(removed.values())
    total_difference = total_inserted - total_removed
    
    print(f"\nTotal Inserted: {total_inserted}")
    print(f"Total Removed: {total_removed}")
    print(f"Total Difference: {total_difference}")

# Read the file
file_path = "out"

with open(file_path, 'r') as file:
    log_data = file.read()

inserted, removed = parse_data(log_data)
differences = calculate_difference(inserted, removed)
print_results(inserted, removed, differences)
