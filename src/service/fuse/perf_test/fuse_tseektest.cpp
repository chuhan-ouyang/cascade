#include <cascade/service_client_api.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <typeindex>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <cascade/utils.hpp>
#include <sys/prctl.h>

using namespace derecho::cascade;
std::vector<uint64_t> readCSV() {
    std::vector<uint64_t> data;
    std::ifstream file("tseekTest.csv");
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            uint64_t value;
            ss >> value;
            data.push_back(value);
        }
        file.close();
        std::cout << "CSV file has been read successfully." << std::endl;
    } else {
        std::cerr << "Unable to open CSV file." << std::endl;
    }
    return data;
}

void read_test(uint32_t file_size, int index, const std::filesystem::path& path, bool verify) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "File could not be opened for reading at " << path << ".\n";
        return;
    }
    char *buffer = static_cast<char*>(malloc(file_size + 1));
    if (!file.read(buffer, file_size)) {
        file.close();
        return;
    }
    file.close();
    if (verify) {
        std::cout << "Answer as Expected? " << buffer[0] << " INDEX: " << index <<'\n';
        if (index == (buffer[0] - '0')){
            std::cout << "TEST PASSED!" <<'\n';
        }else{
            std::cout << "TEST FAILED!" << '\n';
        }
    }
}
/**
 * Put to the key /pool/read_test for a variable size bytes object filled with "1"
 * Usage: ./fuse_perftest_put -i <indexLookup> -o <timeOffsetFromNode>
*/
int main (int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <-s> <file size in KB> <-n> <num runs>\n";
        return 1;
    }
    uint32_t offset = 0;
    int index = 0;
    bool verify = false;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-i" && i + 1 < argc) {
            index = std::atoi(argv[++i]);
        } else if (arg == "-o" && i + 1 < argc) {
            offset = std::atoi(argv[++i]);
        }else if (arg == "-v") {
            verify = true;
        }
    }
    size_t byte_size = 1024;
    std::vector<uint64_t> timeStampRuns = readCSV();

    // for (uint64_t timeStamp : timeStampRuns){
    //     std::cout << "Time: " << timeStamp <<'\n';
    // }
    const char* filePath = "test/latest/pool/read_test";
    int file = open(filePath, O_RDWR);
    if (file < 0) {
        perror("Error opening file");
        return 1;
    }

    uint64_t timeOffset = timeStampRuns.at(index) + offset;
    // Move the file pointer to the desired offset
    off_t newPosition = lseek(file, timeOffset, SEEK_DATA);
    if (newPosition < 0) {
        perror("Error seeking file");
        close(file);
        return 1;
    }
    close(file);
    read_test(byte_size, index, filePath, verify);
    return 0;
}
