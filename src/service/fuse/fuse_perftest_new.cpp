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

/**
 * @param file_size: size of the file to read in bytes
*/
void read_test(uint32_t file_size, uint32_t runs, const std::filesystem::path& path) {
    std::ofstream perf_file("read_perf.csv", std::ios::binary);
    std::ofstream verify_file("read_verify.txt", std::ios::binary);
    double total_time = 0;
    for (int i = 0; i < runs; ++i) {
        std::ifstream file(path, std::ios::binary);
        char* buffer = (char*) malloc(file_size);
        auto start = std::chrono::high_resolution_clock::now();
        file.read(buffer, file_size);
        auto end = std::chrono::high_resolution_clock::now();
        file.close();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        total_time += duration.count();
        perf_file << "Run " << i << " : " << duration.count() << " ns\n";
        
        // correctness check for read
        std::cout << "got: " << buffer << std::endl;
        verify_file.write(reinterpret_cast<const char*>(buffer), file_size);
        verify_file.close();
        usleep(10000);
    }
    double averageTime = total_time / runs;
    perf_file << "Average time to read " << path 
              << " over " << runs << " runs: " << averageTime 
              << " \nFile size: " << file_size << " bytes\n";
}

// void write_test(uint32_t file_size, uint32_t runs, const std::filesystem::path& path) {
//     const char* word = "cascade";
//     size_t wordLength = std::strlen(word); // Length of the word "cascade"
//     int repeated_count = 3; // Example: repeat "cascade" 3 times

//     // Allocate memory: 7 * repeated_count
//     uint8_t* data = new uint8_t[wordLength * repeated_count];

//     // Copy the word into data for repeated_count times
//     for (int i = 0; i < repeated_count; ++i) {
//         std::memcpy(data + (i * wordLength), word, wordLength);
//     }

//     std::ofstream result_file("write_result.txt");
//     double total_time = 0;
//     for (int i = 0; i < runs; ++i) {
//         std::ofstream file(path, std::ios::binary);
//         auto start = std::chrono::high_resolution_clock::now();
//         if (!file.write(data.data(), data.size())) {
//             std::cerr << "Failed to write to file." << std::endl;
//         } 
//         auto end = std::chrono::high_resolution_clock::now();
//         file.close();
//         auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
//         total_time += duration.count();
//         result_file << "Run " << i << " : " << duration.count() << " ns\n";
//         // sleep here
//         usleep(10000);
//     }
//     double averageTime = total_time / runs;
//     result_file << "Average time to write " << path 
//               << " over " << runs << " runs: " << averageTime 
//               << " \nFile size: " << file_size << " bytes\n";
// }

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
        // write_test(file_size, 1, write_path);
    }
    return 0;
}