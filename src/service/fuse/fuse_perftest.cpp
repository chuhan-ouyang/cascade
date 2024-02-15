#include <fstream>
#include <iostream>
#include <chrono>
#include <vector>
#include <filesystem> 
#include <string>

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

bool verifyContent(const std::vector<char>& buffer, const std::string& expectedWord) {
    std::string bufferContent(buffer.begin(), buffer.end());
    std::cout << bufferContent << std::endl;
    size_t pos = 0;
    while (pos < bufferContent.size()) {
        size_t nextPos = bufferContent.find(expectedWord, pos);
        if (nextPos != pos) {
            return false;
        }
        pos = nextPos + expectedWord.size();
    }
    return true; 
}

void read_test(uint32_t kb_size, uint32_t runs, const std::filesystem::path& path) {
    using namespace std::chrono;
    const std::filesystem::path filePath = path / "read_file.txt";
    std::filesystem::create_directories(path);

    std::vector<double> timings;
    size_t fileSize = kb_size * 1024; 
    createFile(filePath, fileSize);
    for (int i = 0; i < runs; ++i) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file) {
            std::cerr << "File could not be opened for reading at " << filePath << ".\n";
        }
        std::vector<char> buffer(fileSize);
        auto start = high_resolution_clock::now();
        if (!file.read(buffer.data(), fileSize)) {
            std::cerr << "Error reading file on run " << i+1 << ".\n";
            file.close();
            continue;
        }
        auto end = high_resolution_clock::now();
        file.close();
        timings.push_back(duration<double>(end - start).count());
    }
    double averageTime = 0;
    for (auto time : timings) {
        averageTime += time;
    }
    averageTime /= runs;
    std::cout << "Average time to read a newly file " << filePath 
              << " over " << runs << " runs: " << averageTime 
              << " s\nFile size: " << kb_size << " KB";
}

void write_test(uint32_t kb_size, uint32_t runs, const std::filesystem::path& path) {
    using namespace std::chrono;
    const std::string word = "cascade";
    const size_t wordSize = word.size();
    const size_t fileSize = kb_size * 1024;
    const size_t repetitions = fileSize / wordSize;

    std::vector<double> timings;
    std::ofstream file;
    std::filesystem::create_directories(path);
    const std::filesystem::path filePath = path / "benchmark_file.txt";
    for (int i = 0; i < runs; ++i) {
        file.open(filePath, std::ios::binary | std::ios::trunc);
        if (!file) {
            std::cerr << "File could not be opened at " << filePath << ".\n";
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
    std::cout << "Average time to write to " << filePath 
              << " over " << runs << " runs: " << averageTime 
              << " s\nFile size: " << kb_size << " KB\n";
}

int main() {
    std::filesystem::path objp_path = "test/latest/pool1";
    // write_test(1, 1000, objp_path);
    read_test(1, 1000, objp_path);
    return 0;
}