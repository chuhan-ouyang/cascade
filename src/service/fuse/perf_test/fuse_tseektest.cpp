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

#define check_put_and_remove_result(result) \
    for (auto& reply_future:result.get()) {\
        auto reply = reply_future.second.get();\
        std::cout << "node(" << reply_future.first << ") replied with version:" << std::get<0>(reply)\
                  << ",ts_us:" << std::get<1>(reply) << std::endl;\
    }


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
    size_t byte_size = kb_size * 1024;

    auto& capi = ServiceClientAPI::get_service_client();
    capi.create_object_pool<PersistentCascadeStoreWithStringKey>("/pool", 0, sharding_policy_type::HASH, {}, "");
    std::vector<uint8_t*> buffers;
    for (uint32_t i = 1; i <= num_runs; i++) {
        uint8_t* buffer = (uint8_t*) malloc(byte_size);
        for (size_t j = 0; j < byte_size; j++) {
            buffer[j] = char(j) - '0';
        }
        ObjectWithStringKey obj;
        obj.key = "/pool/tseek_test";
        obj.blob = Blob(buffer, byte_size);
        auto res = capi.put(obj); // Get Timestamp from Put
        check_put_and_remove_result(res);
        buffers.push_back(buffer);
    }
    // Save Time As Well for Each Run. 
    // use Timestamp to get the version and value i

// Get Time Before Offset


// Get Time At Offset


// Get Time after Offset
    // for (auto& buffer : buffers) {
    //     free(buffer);
    // }
    return 0;
}
