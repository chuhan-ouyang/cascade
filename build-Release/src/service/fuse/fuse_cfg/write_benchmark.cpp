#include <fstream>
#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <filesystem> // For directory and file operations

int main() {
    using namespace std::chrono;
    const std::string word = "cascade";
    const size_t wordSize = word.size();
    const size_t fileSize = 5 * 1024 * 1024; // 5MB intended size, actual may vary slightly
    const size_t repetitions = fileSize / wordSize;

    std::vector<double> timings;
    const int runs = 100;
    std::ofstream file;

    // Directory path where the file will be written
    const std::filesystem::path dirPath = "n4/test/latest/pool1";

    // Ensure the directory exists
    std::filesystem::create_directories(dirPath);

    // Path to the file within the specified directory
    const std::filesystem::path filePath = dirPath / "benchmark_file.txt";

    for (int i = 0; i < runs; ++i) {
        file.open(filePath, std::ios::binary | std::ios::trunc);
        if (!file) {
            std::cerr << "File could not be opened at " << filePath << ".\n";
            return 1;
        }
        
        auto start = high_resolution_clock::now();
        
        for (size_t j = 0; j < repetitions; ++j) {
            file << word;
        }
        
        file.close();
        
        auto end = high_resolution_clock::now();
        timings.push_back(duration<double>(end - start).count());
    }
    
    double averageTime = 0;
    for (auto time : timings) {
        averageTime += time;
    }
    averageTime /= runs;
    
    // Calculate the size of the file in KB
    auto fileSizeKB = std::filesystem::file_size(filePath) / 1024.0;

    std::cout << "Average time to write to " << filePath 
              << " over " << runs << " runs: " << averageTime 
              << " s\nFile size: " << fileSizeKB << " KB\n";
    
    return 0;
}
