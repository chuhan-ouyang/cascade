#include <fstream>
#include <iostream>
#include <chrono>
#include <vector>
#include <filesystem> 
#include <string>
#include <cstdlib>

void createFile(const std::filesystem::path& filePath, size_t fileSize) {
    std::ofstream file(filePath, std::ios::binary | std::ios::trunc);
    if (!file) {
        std::cerr << "File could not be opened for writing at " << filePath << ".\n";
        exit(1);
    }
    const std::string data = "cascade";
    size_t written = 0;
    while (written < fileSize) {
        file << data;
        written += data.size();
    }
    file.close();  
}

void read_test(uint32_t file_size, uint32_t runs, const std::filesystem::path& path) {
    std::ofstream result_file("read_result.txt");
    std::ofstream verify_file("read_verify.txt", std::ios::binary);
    double total_time = 0;
    for (int i = 0; i < runs; ++i) {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            std::cerr << "File could not be opened for reading at " << path << ".\n";
        }
        std::vector<char> buffer(file_size);
        auto start = std::chrono::high_resolution_clock::now();
        // TODO (chuhan): most efficient way?
        if (!file.read(buffer.data(), file_size)) {
            std::cerr << "Error reading file on run " << i << ".\n";
            file.close();
            continue;
        }
        auto end = std::chrono::high_resolution_clock::now();
        file.close();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        total_time += duration.count();
        result_file << "Run " << i << " : " << duration.count() << " ns\n";

        // correctness check for read
        verify_file.write(buffer.data(), buffer.size());
        verify_file.close();
    }
    double averageTime = total_time / runs;
    result_file << "Average time to read " << path 
              << " over " << runs << " runs: " << averageTime 
              << " \nFile size: " << file_size << " bytes\n";
}

void write_test(uint32_t file_size, uint32_t runs, const std::filesystem::path& path) {
    const std::string word = "cascade";
    size_t wordSize = word.length() + 1; // separator
    size_t repeatCount = file_size / wordSize;
    size_t totalSize = repeatCount * wordSize;
    std::vector<char> data;
    data.reserve(totalSize);  
    for (size_t i = 0; i < repeatCount; ++i) {
        data.insert(data.end(), word.begin(), word.end());
        data.push_back(' ');  
    }
    if (data.size() > file_size) { // truncate
        data.resize(file_size);
    }

    std::ofstream result_file("write_result.txt");
    double total_time = 0;
    for (int i = 0; i < runs; ++i) {
        std::ofstream file(path, std::ios::binary);
        auto start = std::chrono::high_resolution_clock::now();
        if (!file.write(data.data(), data.size())) {
            std::cerr << "Failed to write to file." << std::endl;
        } 
        auto end = std::chrono::high_resolution_clock::now();
        file.close();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        total_time += duration.count();
        result_file << "Run " << i << " : " << duration.count() << " ns\n";
    }
    double averageTime = total_time / runs;
    result_file << "Average time to write " << path 
              << " over " << runs << " runs: " << averageTime 
              << " \nFile size: " << file_size << " bytes\n";
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <read/write> <file size in KB>\n";
        return 1;
    }
    std::string operation = argv[1];  
    int fileSizeKB = std::atoi(argv[2]);  
    if (operation != "read" && operation != "write") {
        std::cerr << "Invalid operation. Please use 'read' or 'write'.\n";
        return 1;
    }
    if (fileSizeKB <= 0) {
        std::cerr << "Invalid file size. Please specify a positive integer for file size in KB.\n";
        return 1;
    }
    std::cout << "Operation: " << operation << "\n";
    std::cout << "File size: " << fileSizeKB << " KB\n";
    size_t file_size = fileSizeKB * 1024; 
    std::filesystem::path objp_path = "test/latest/pool1";
    std::filesystem::path read_path = objp_path / "read_test";
    std::filesystem::path write_path = objp_path / "write_test";
    if (operation == "read") {
        read_test(file_size, 1, read_path);
    } else if (operation == "write") {
        write_test(file_size, 1, write_path);
    }
    return 0;
}