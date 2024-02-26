import os

def create_dummy_file(file_path, size_in_kb):
    with open(file_path, 'w') as f:
        word = "cascade\n"
        bytes_written = 0
        while bytes_written < size_in_kb * 1024:
            f.write(word)
            bytes_written += len(word)

def main():
    folder_path = 'test/latest/pool1'
    file_name = 'dummy_file250MBv1.txt'
    file_path = os.path.join(folder_path, file_name)
    size_in_kb = 250000 # 100MB
    
    # Create directory if it doesn't exist
    os.makedirs(folder_path, exist_ok=True)
    
    # Create dummy file
    create_dummy_file(file_path, size_in_kb)
    print(f"Dummy file '{file_name}' created in '{folder_path}'")

if __name__ == "__main__":
    main()