#include <fstream>
#include <iostream>
#include <chrono>
#include <vector>
#include <filesystem> 
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <cstdint>  
#include <cstring>  

void read_test(uint32_t file_size, uint32_t runs, const std::filesystem::path& path) {
    std::filesystem::create_directories("perf");
    std::filesystem::path read_path = path / "read_test";
    std::ofstream perf_file("perf/read_perf.csv", std::ios::binary);
    std::ofstream verify_file("perf/read_verify.txt", std::ios::binary);
    double total_time = 0;
    for (int i = 0; i < runs; ++i) {
        std::ifstream file(read_path, std::ios::binary);
        char *buffer = static_cast<char*>(malloc(file_size + 1));
        auto start = std::chrono::high_resolution_clock::now();
        file.read(buffer, file_size);
        auto end = std::chrono::high_resolution_clock::now();
        file.close();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        total_time += duration.count();
        perf_file << "Run " << i + 1 << " : " << duration.count() << " ns\n";
        verify_file.write(reinterpret_cast<const char*>(buffer), file_size);
        verify_file.close();
        usleep(10000);
    }
    double averageTime = total_time / runs;
    perf_file << "Average time to read " << read_path 
              << " over " << runs << " runs: " << averageTime << " ns"
              << " \nFile size: " << file_size << " bytes\n";
}

void write_test(uint32_t file_size, uint32_t runs, const std::filesystem::path& path) {
    std::filesystem::create_directories("perf");
    std::ofstream perf_file("perf/write_perf.csv", std::ios::binary);
    std::filesystem::path write_path = path / "write_test";
    char *buffer = static_cast<char*>(malloc(file_size + 1));
    for (size_t i = 0; i < file_size; i++) {
        buffer[i] = '1';
    }
    double total_time = 0;
    for (int i = 0; i < runs; ++i) {
        std::ofstream file(write_path, std::ios::binary);
        auto start = std::chrono::high_resolution_clock::now();
        file.write(buffer, file_size);
        auto end = std::chrono::high_resolution_clock::now();
        file.close();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        total_time += duration.count();
        perf_file << "Run " << i + 1 << " : " << duration.count() << " ns\n";
        usleep(10000);
    }
    double averageTime = total_time / runs;
    perf_file << "Average time to write " << write_path 
              << " over " << runs << " runs: " << averageTime << " ns"
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
    if (operation == "read") {
        read_test(file_size, 1, objp_path);
    } else if (operation == "write") {
        write_test(file_size, 1, objp_path);
    }
    return 0;
}