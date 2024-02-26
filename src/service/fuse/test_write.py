import os

def create_dummy_file(file_path, size_in_kb):
    with open(file_path, 'w') as f:
        word = "cascade\n"
        bytes_written = 0
        while bytes_written < size_in_kb * 1024 * 1024:
            f.write(word)
            bytes_written += len(word)

def main():
    folder_path = 'test/latest/pool1'
    file_path = os.path.join(folder_path, file_name)
    size_in_mb = 1000000 # 1Gb
    file_name = f"dummy_file_{size_in_mb}MB.txt"
    
    # Create directory if it doesn't exist
    os.makedirs(folder_path, exist_ok=True)
    
    # Create dummy file
    create_dummy_file(file_path, size_in_mb)
    print(f"Dummy file '{file_name}' created in '{folder_path}'")

if __name__ == "__main__":
    main()