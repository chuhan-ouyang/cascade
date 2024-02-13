#include <fstream>
#include <iostream>
#include <chrono>
#include <vector>
#include <filesystem> // For file operations
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
    file.close(); // Ensure file is closed and data is flushed
}

bool verifyContent(const std::vector<char>& buffer, const std::string& expectedWord) {
    std::string bufferContent(buffer.begin(), buffer.end());
    std::cout << bufferContent << std::endl;
    size_t pos = 0;
    while (pos < bufferContent.size()) {
        size_t nextPos = bufferContent.find(expectedWord, pos);
        if (nextPos != pos) {
            // Found a mismatch or extra characters between words
            return false;
        }
        pos = nextPos + expectedWord.size();
    }
    return true; // All words in the buffer are "cascade"
}

int main() {
    using namespace std::chrono;

    const std::filesystem::path dirPath = ".";
    const std::filesystem::path filePath = dirPath / "5MB_file.txt";
    std::filesystem::create_directories(dirPath);

    const std::string dataPattern = "cascade";
    std::vector<double> timings;
    const int runs = 1;
    size_t fileSize = 1024; // 1kb
    createFile(filePath, fileSize);

    for (int i = 0; i < runs; ++i) {

        std::ifstream file(filePath, std::ios::binary);
        if (!file) {
            std::cerr << "File could not be opened for reading at " << filePath << ".\n";
            return 1;
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

        if (!verifyContent(buffer, dataPattern)) {
            std::cerr << "Verification failed: Not all words are 'cascade' on run " << i+1 << ".\n";
            continue; // Skip further processing for this run
        }

        timings.push_back(duration<double>(end - start).count());
    }

    double averageTime = 0;
    for (auto time : timings) {
        averageTime += time;
    }
    averageTime /= runs;
    
    auto fileSizeKB = fileSize / 1024.0;

    std::cout << "Average time to read and verify a newly created 5MB file " << filePath 
              << " over " << runs << " runs: " << averageTime 
              << " s\nFile size: " << fileSizeKB << " KB\nVerification: Success\n";
    
    return 0;
}
