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

/**
 * Put to the key /pool/read_test for a variable size bytes object filled with "1"
 * Usage: ./fuse_perftest_put -s <kb_size> -r <runs>
*/
int main (int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <-s> <file size in KB> <-n> <num runs>\n";
        return 1;
    }
    uint32_t kb_size = 0, num_runs = 0;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-s" && i + 1 < argc) {
            kb_size = std::atoi(argv[++i]);
        } else if (arg == "-n" && i + 1 < argc) {
            num_runs = std::atoi(argv[++i]);
        }
    }
    if (num_runs < 4){
        std::cerr << "Must have at least 4 Runs\n";
        return -1;
    }
    size_t byte_size = kb_size * 1024;

    auto& capi = ServiceClientAPI::get_service_client();
    capi.create_object_pool<PersistentCascadeStoreWithStringKey>("/pool", 0, sharding_policy_type::HASH, {}, "");
    std::vector<uint8_t*> buffers;
	std::vector<uint64_t> timeStampRuns;
    for (uint32_t i = 1; i <= num_runs; i++) {
        uint8_t* buffer = (uint8_t*) malloc(byte_size);
        for (size_t j = 0; j < byte_size; j++) {
            buffer[j] = char(j) - '0';
        }
        ObjectWithStringKey obj;
        obj.key = "/pool/tseek_test";
        obj.blob = Blob(buffer, byte_size);
        auto res = capi.put(obj); // Get Timestamp from Put
        // Save Time From Put
	    for (auto& reply_future: res.get()){
	    	auto reply = reply_future.second.get();
            uint64_t ts = std::get<1>(reply);
            timeStampRuns.push_back(ts);
            break;
        }
        buffers.push_back(buffer);
    }
    // Save Time As Well for Each Run. 
    // use Timestamp to get the version and value i
    uint64_t firstTime = timeStampRuns.at(0);
    uint64_t halfway  = timeStampRuns.at(num_runs/2);
    uint64_t lastTime = timeStampRuns.at(num_runs-1);
    const char* filePath = "/pool/tseek_test";
    int file = open(filePath, O_RDWR);
    if (file == -1) {
        printf("Error opening the file.\n");
        return 1;
    }
    int SEEK_TIME = 10;
    off_t offset0 = lseek(file, 0, SEEK_TIME);
    if (offset0 == (off_t)-1) {
        printf("Error seeking the file.\n");
        close(file);
        return 1;
    }
    std::cout << "Offset Should be 0: " << offset0 <<'\n';

    off_t offset2 = lseek(file, firstTime, SEEK_TIME);
    if (offset2 == (off_t)-1) {
        printf("Error seeking the file.\n");
        close(file);
        return 1;
    }
    std::cout << "Offset Should be 0: " << offset2 <<'\n';
	
// Get Time Before Offset
// Find Time

// Get Time At Offset
// Find Final Time

// Get Time after Offset
    // for (auto& buffer : buffers) {
    //     free(buffer);
    // }
    return 0;
}
