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

void record_timestamp(std::vector<std::chrono::nanoseconds> timestamps, std::string& file_name, uint32_t file_size, uint32_t runs) {
    std::ofstream perf_file("perf/" + file_name, std::ios::binary);
    int64_t total_time = 0;
    for (size_t i = 0; i < timestamps.size(); i++) {
        perf_file << "Run " << i + 1 << " : " << timestamps[i].count() << " ns\n";
        total_time += timestamps[i].count();
    }
    double average_time = total_time / runs;
    perf_file << "Average time to write " << file_name 
              << " over " << runs << " non-warmup runs: " << average_time << " ns"
              << " \nFile size: " << file_size / 1024 << " KB\n";
    perf_file.close();
}

void read_test(uint32_t file_size, uint32_t runs, uint32_t warmup_runs, const std::filesystem::path& path, bool verify) {
    std::filesystem::path read_path = path / "read_test";
    std::vector<std::chrono::nanoseconds> timestamps;
    for (int i = 0; i < runs; ++i) {
        std::ifstream file(read_path, std::ios::binary);
        if (!file) {
            std::cerr << "File could not be opened for reading at " << read_path << ".\n";
            return;
        }
        char *buffer = static_cast<char*>(malloc(file_size + 1));
        auto start = std::chrono::high_resolution_clock::now();
        if (!file.read(buffer, file_size)) {
            std::cerr << "Error reading file on run " << i + 1 << ".\n";
            file.close();
            return;
        }
        auto end = std::chrono::high_resolution_clock::now();
        file.close();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        if (i >= warmup_runs) {
            timestamps.push_back(duration);
        }
        if (verify && i == runs - 1) {
            std::ofstream verify_file("perf/read_verify.txt", std::ios::binary);
            verify_file.write(reinterpret_cast<const char*>(buffer), file_size);
            verify_file.close();
        }
        usleep(10000);
    }
    std::string file_name = "read_perf_" + std::to_string(file_size / 1024) + "_kb_" + std::to_string(runs) + "_runs.csv"; 
    record_timestamp(timestamps, file_name, file_size, runs - warmup_runs);
}

void write_test(uint32_t file_size, uint32_t runs, uint32_t warmup_runs, const std::filesystem::path& path, bool verify) {
    std::filesystem::path write_path = path / "write_test";
    char *buffer = static_cast<char*>(malloc(file_size + 1));
    for (size_t i = 0; i < file_size; i++) {
        buffer[i] = '1';
    }
    std::vector<std::chrono::nanoseconds> timestamps;
    for (int i = 0; i < runs; ++i) {
        std::ofstream file(write_path, std::ios::binary);
        if (!file) {
            std::cerr << "File could not be opened at " << write_path << ".\n";
        }
        auto start = std::chrono::high_resolution_clock::now();
        if (!file.write(buffer, file_size)) {
            std::cerr << "Error writing file on run " << i + 1 << ".\n";
            file.close();
            return;
        }
        auto end = std::chrono::high_resolution_clock::now();
        file.close();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        if (i >= warmup_runs) {
            timestamps.push_back(duration);
        }
        usleep(10000);
    }
    std::string file_name = "write_perf_" + std::to_string(file_size / 1024) + "_kb_" + std::to_string(runs) + "_runs.csv"; 
    record_timestamp(timestamps, file_name, file_size, runs - warmup_runs);
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <read/write> <-s> <file size in KB> <-n> <num runs> <-w> <warm up runs> <-v> (optional)\n";
        return 1;
    }
    std::string operation = argv[1];  
    if (operation != "read" && operation != "write") {
        std::cerr << "Invalid operation. Please use 'read' or 'write'.\n";
        return 1; 
    }
    uint32_t kb_size, num_runs, warmup_runs;
    bool verify = false;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-s" && i + 1 < argc) {
            kb_size = std::atoi(argv[++i]);
        } else if (arg == "-n" && i + 1 < argc) {
            num_runs = std::atoi(argv[++i]);
        } else if (arg == "-w" && i + 1 < argc) {
            warmup_runs = std::atoi(argv[++i]);
        } else if (arg == "-v") {
            verify = true;
        }
    }
    uint32_t file_size = kb_size * 1024; 
    std::filesystem::path objp_path = "test/latest/pool1";
    std::cout << "Operation: " << operation << std::endl;
    std::cout << "File size: " << kb_size << " KB" << std::endl;
    std::cout << "Number of runs: " << num_runs << std::endl;
    std::cout << "Warmup runs: " << warmup_runs << std::endl;

    std::filesystem::create_directories("perf");
    if (operation == "read") {
        read_test(file_size, num_runs, warmup_runs, objp_path, verify);
    } else if (operation == "write") {
        write_test(file_size, num_runs, warmup_runs, objp_path, verify);
    }
    return 0;
}