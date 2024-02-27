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
 * Put to the key /pool1/read_test for a variable size bytes object filled with "1"
*/
int main (int argc, char* argv[]) {
    int kb_size = atoi(argv[1]);
    size_t byte_size = kb_size * 1024;
    auto& capi = ServiceClientAPI::get_service_client();
    uint8_t* buffer = (uint8_t*) malloc(byte_size);
    for (size_t i = 0; i < byte_size; i++) {
        buffer[i] = '1';
    }
    capi.create_object_pool<PersistentCascadeStoreWithStringKey>("/pool1", 0, sharding_policy_type::HASH, {}, "");
    ObjectWithStringKey obj;
    obj.key = "/pool1/read_test";
    // TODO (chuhan): is byte_size he correct argument to pass into blob
    obj.blob = Blob(buffer, byte_size);
    auto res = capi.put(obj);
    check_put_and_remove_result(res);
    free(buffer);
    return 0;
}