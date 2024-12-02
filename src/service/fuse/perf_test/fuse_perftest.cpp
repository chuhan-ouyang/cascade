#include <cascade/service_types.hpp>
#include <derecho/utils/logger.hpp>

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
#include <chrono>

using namespace derecho::cascade;

#define READ_START_TIME 1000
#define READ_END_TIME 1004
#define WRITE_START_TIME 2000
#define WRITE_END_TIME 2004

void record_timestamp(std::vector<std::chrono::nanoseconds> timestamps, std::string& file_name, uint32_t file_size, uint32_t runs) {
    std::ofstream perf_file("perf/" + file_name, std::ios::binary);
    int64_t total_time = 0;
    for (size_t i = 0; i < timestamps.size(); i++) {
        perf_file << "Run " << i + 1 << " : " << timestamps[i].count() << " ns\n";
        total_time += timestamps[i].count();
    }
    double average_time = total_time / runs;
    perf_file << "Average time to write " << file_name
              << " over " << runs << " runs: " << average_time << " ns"
              << " \nFile size: " << file_size / 1024 << " KB\n";
    perf_file.close();
}

void read_test(uint32_t file_size, uint32_t runs, const std::filesystem::path& path, bool verify) {
    for (uint32_t i = 0; i < runs; ++i) {
        std::string file_name = "read_test" + std::to_string(i);
        std::filesystem::path read_path = path / file_name;
        std::ifstream file(read_path, std::ios::binary);
        if (!file) {
            std::cerr << "File could not be opened for reading at " << read_path << ".\n";
            return;
        }
        char *buffer = static_cast<char*>(malloc(file_size + 1));
        TimestampLogger::log(READ_START_TIME,4,i,get_walltime());
        if (!file.read(buffer, file_size)) {
            std::cerr << "Error reading file on run " << i + 1 << ".\n";
            file.close();
            return;
        }
        file.close();
        TimestampLogger::log(READ_END_TIME,4,i,get_walltime());
        if (verify && i == runs - 1) {
            std::ofstream verify_file("perf/read_verify.txt", std::ios::binary);
            verify_file.write(reinterpret_cast<const char*>(buffer), file_size);
            verify_file.close();
        }
        usleep(10000);
    }
    std::string logger_path = "fuse_perftest_logger.csv";
    TimestampLogger::flush(logger_path, false);
}

void write_test(uint32_t file_size, uint32_t runs, const std::filesystem::path& path, bool verify) {
    std::filesystem::path write_path = path / "write_test";
    char *buffer = static_cast<char*>(malloc(file_size + 1));
    for (size_t i = 0; i < file_size; i++) {
        buffer[i] = '1';
    }
    std::vector<long long> write_latencies_ns;
    for (uint32_t i = 1; i <= runs; ++i) {
        std::ofstream file(write_path, std::ios::binary);
        if (!file) {
            std::cerr << "File could not be opened at " << write_path << ".\n";
        }
        // TimestampLogger::log(WRITE_START_TIME,4,i,get_walltime());
        auto start_time = std::chrono::high_resolution_clock::now();
        if (!file.write(buffer, file_size)) {
            std::cerr << "Error writing file on run " << i + 1 << ".\n";
            file.close();
            return;
        }
        file.close();
        // TimestampLogger::log(WRITE_END_TIME,4,i,get_walltime());
        auto end_time = std::chrono::high_resolution_clock::now();
        long long latency_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
        write_latencies_ns.push_back(latency_ns);
        usleep(10000);
    }

    std::string file_name = "write_perf_" + std::to_string(file_size / 1024) + "_kb_" + std::to_string(runs) + "_runs_perftest.csv";
    std::ofstream csv_file(file_name);
    if (!csv_file) {
        std::cerr << "Could not create CSV file: " << file_name << ".\n";
        return;
    }
    csv_file << "Run,Latency (ns)\n";
    for (size_t i = 0; i < write_latencies_ns.size(); ++i) {
        csv_file << (i + 1) << "," << write_latencies_ns[i] << "\n";
    }
    csv_file.close();

    // TimestampLogger::flush(file_name, false);
    free(buffer);
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <read/write> <-s> <file size in KB> <-n> <num runs> <-v> (optional)\n";
        return 1;
    }
    std::string operation = argv[1];
    if (operation != "read" && operation != "write") {
        std::cerr << "Invalid operation. Please use 'read' or 'write'.\n";
        return 1;
    }
    uint32_t kb_size, num_runs;
    bool verify = false;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-s" && i + 1 < argc) {
            kb_size = std::atoi(argv[++i]);
        } else if (arg == "-n" && i + 1 < argc) {
            num_runs = std::atoi(argv[++i]);
        } else if (arg == "-v") {
            verify = true;
        }
    }
    uint32_t file_size = kb_size * 1024;
    std::filesystem::path objp_path = "test/latest/pool";
    std::cout << "Operation: " << operation << std::endl;
    std::cout << "File size: " << kb_size << " KB" << std::endl;
    std::cout << "Number of runs: " << num_runs << std::endl;

    std::filesystem::create_directories("perf");
    if (operation == "read") {
        read_test(file_size, num_runs, objp_path, verify);
    } else if (operation == "write") {
        write_test(file_size, num_runs, objp_path, verify);
    }
    return 0;
}