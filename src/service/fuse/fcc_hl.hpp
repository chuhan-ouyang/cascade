#pragma once

#define FUSE_USE_VERSION 34

#include <cascade/object_pool_metadata.hpp>
#include <cascade/service_client_api.hpp>
#include <fuse3/fuse.h>
#include <memory>
#include <mutils-containers/KindMap.hpp>
#include <nlohmann/json.hpp>
#include <shared_mutex>

#include "path_tree.hpp"

namespace fs = std::filesystem;

#define BEFORE_CAPI_GET 1001
#define AFTER_CAPI_GET 1002

#define BLOB_SIZE_OFFSET 58
#define LATEST_PREFIX_SIZE 7


std::shared_ptr<spdlog::logger> DL;

int extract_number(const std::string& input) {
    size_t numPos = input.find_first_of("0123456789");
    if (numPos == std::string::npos) {
        return 0;
    }
    std::string numStr = input.substr(numPos);
    int res = std::stoi(numStr);
    return res;
}

enum NodeFlag;

/*
op_get_by_time
op_get_size <-- create new function for content size ???
VT template type ??
op_list_keys_by_time
*/

const uint32_t FILE_FLAG = KEY_FILE | METADATA_INFO_FILE;
const uint32_t DIR_FLAG = ~FILE_FLAG;
const uint32_t OP_FLAG = OP_PREFIX_DIR | OP_PATH_DIR | KEY_DIR | KEY_FILE;

struct NodeData {
    NodeFlag flag;
    // timestamp in microsec
    uint64_t timestamp;
    std::shared_ptr<uint8_t[]> bytes;

    // not empty if this node contains a full object pool path (then use list keys)
    std::string objp_name;

    size_t size;

    bool writeable;

    NodeData(const NodeFlag _flag) : flag(_flag), timestamp(0),
                                    size(0), writeable(false) { }

    NodeData(const NodeFlag _flag, const std::string& _objp_name) :
                                    flag(_flag), timestamp(0),
                                    objp_name(_objp_name), size(0), writeable(false) { }
    
    ~NodeData() {};
};

namespace derecho {
namespace cascade {

struct FuseClientContext {
    using Node = PathTree<NodeData>;

    std::shared_ptr<Node> root;

    // object pools are not versioned? :(
    const fs::path METADATA_PATH = "/.cascade";
    const fs::path SNAPSHOT_PATH = "/snapshot";
    const fs::path LATEST_PATH = "/latest";
    const fs::path ROOT = "/";
    ServiceClientAPI& capi;
    node_id_t node_id;

    bool version_snapshot;
    persistent::version_t max_ver;
    uint64_t max_timestamp;  // timestamp in microsecond

    std::set<fs::path> local_latest_dirs;
    std::set<persistent::version_t> snapshots;
    std::set<uint64_t> snapshots_by_time;

    std::vector<std::shared_ptr<uint8_t[]>> fileptrs_in_use;

    time_t update_interval;
    time_t last_update_sec;

    FuseClientContext(int update_int, bool ver_snap) : capi(ServiceClientAPI::get_service_client()) {
        // DL = LoggerFactory::createLogger("fuse_client", spdlog::level::from_str(derecho::getConfString(CONF_LOGGER_DEFAULT_LOG_LEVEL)));
        // DL->set_pattern("[%T][%n][%^%l%$] %v");

        node_id = capi.get_my_id();
        max_ver = 0;
        max_timestamp = 0;

        version_snapshot = ver_snap;
        //dbg_info(DL, "snapshot type: {}", version_snapshot ? "version" : "timestamp");
        update_interval = update_int;

        root = std::make_unique<Node>(ROOT, NodeData(ROOT_DIR));
        // root->set(SNAPSHOT_PATH, NodeData(SNAPSHOT_ROOT_DIR), NodeData(SNAPSHOT_ROOT_DIR));
        reset_latest();
        fill_at(LATEST_PATH, CURRENT_VERSION);
        last_update_sec = time(0);
    }

    /*  --- pathtree related logic ---  */

    void reset_latest() {
        auto latest = root->get(LATEST_PATH);
        if(latest != nullptr) {
            latest->data = NodeData(LATEST_DIR);
            // TODO: no need to clear
            latest->children.clear();
        } else {
            root->set(LATEST_PATH, NodeData(LATEST_DIR), NodeData(LATEST_DIR));
        }
    }

    /**
     * @fn Return true if the node is not a nullptr and false otherwise
    */
    // Node* add_op_info(const fs::path& path, const std::string& contents) {
    bool add_op_info(const fs::path& path, const std::string& contents) {
        auto node = root->set(path, NodeData(METADATA_PREFIX_DIR),
                              NodeData(METADATA_INFO_FILE));
        if(node == nullptr) {
            return false;
        }
        node->data.bytes = std::shared_ptr<uint8_t[]>(new uint8_t[contents.size()]);
        std::copy(contents.begin(), contents.end(), node->data.bytes.get());
        node->data.size = contents.size();
        // auto str = reinterpret_cast<char*>(node->data.bytes.get());
        return true;
    }

    /**
     * @fn Return 
    */
    // Node* add_snapshot_time(const fs::path& path) {
    std::shared_ptr<Node> add_snapshot_time(const fs::path& path) {
        return root->set(path, NodeData(SNAPSHOT_ROOT_DIR), NodeData(SNAPSHOT_TIME_DIR));
    }

    // TODO op: old app_op_root -> now add_full_op_dir
    // Node* add_op_root(const fs::path& path) {
    //     return root->set(path, NodeData(OP_PREFIX_DIR), NodeData(OP_PATH_DIR));
    // }

    // TODO op: add documentation
    // Node* add_full_op_dir(const fs::path& path, const std::string& objp_name) {
    std::shared_ptr<Node> add_full_op_dir(const fs::path& path, const std::string& objp_name) {
        dbg_default_trace("Entered: {}", __PRETTY_FUNCTION__);
        // int, data
        // Node* path_tree_node = root->set(path, NodeData(OP_PREFIX_DIR), NodeData(OP_PATH_DIR, objp_name));
        std::shared_ptr<Node> path_tree_node = root->set(path, NodeData(OP_PREFIX_DIR), NodeData(OP_PATH_DIR, objp_name));
        // TODO op: path should not contain /latest
        if (path_tree_node == nullptr) {
            dbg_default_trace("In {}, path_tree_node is nullptr", __PRETTY_FUNCTION__);
        }
        if (path_tree_node->data.objp_name.empty()) {
            dbg_default_trace("In {}, node's objp_name is empty", __PRETTY_FUNCTION__);
        }
        dbg_default_trace("In {}, Set path_tree_node's objp_subdir to: {}", __PRETTY_FUNCTION__, path);
        return path_tree_node;
    }

    /**
     * Add node for the path, without getting contents for the file
    */
    // Node* add_op_key(const fs::path& path, const std::string& op_path) {
    std::shared_ptr<Node> add_op_key(const fs::path& path, const std::string& op_path) {
        // invariant: assumes op_root already exists
        // Node* node_ptr = root->set(path, NodeData(KEY_DIR, op_path), NodeData(KEY_FILE, op_path));
        std::shared_ptr<Node> node_ptr = root->set(path, NodeData(KEY_DIR, op_path), NodeData(KEY_FILE, op_path));
        // TODO op: check that op_path is correct
        if (node_ptr->data.objp_name.empty()) {
            dbg_default_trace("In {}, path: {}, op_path is empty: {}, node's objp_name is empty", __PRETTY_FUNCTION__, path, op_path.empty());
        }
        node_ptr->data.writeable = true;
        return node_ptr;
    }

    // TODO op: change
    // Node* add_op_key_dir(const fs::path& path) {
    std::shared_ptr<Node> add_op_key_dir(const fs::path& path) {
        // invariant: assumes op_root already exists
        return root->set(path, NodeData(KEY_DIR), NodeData(KEY_DIR));
    }

    // Node* object_pool_root(Node* node) {
    std::shared_ptr<Node> object_pool_root(std::shared_ptr<Node> node) {
        if(node == nullptr) {
            return nullptr;
        }
        if(node->parent == nullptr || node->data.flag & OP_PATH_DIR) {
            return node;
        }
        if(node->data.flag & (KEY_DIR | KEY_FILE)) {
            return object_pool_root(node->parent);
        }
        return nullptr;
    }

    // Node* nearest_object_pool_root(const fs::path& path) {
    std::shared_ptr<Node> nearest_object_pool_root(const fs::path& path) {
        auto node = root->get_while_valid(path);
        return object_pool_root(node);
    }

    bool add_snapshot(const fs::path& path) {
        if(path.parent_path() != SNAPSHOT_PATH) {
            return false;
        }
        try {
            unsigned long long ts_us = std::stoull(path.filename());
            persistent::version_t ver = ts_us;
            if(version_snapshot && ver <= max_ver) {
                add_snapshot_folder(ver);
                return true;
            }
            if(!version_snapshot && ts_us <= max_timestamp) {
                add_snapshot_folder_by_time(ts_us);
                return true;
            }
        } catch(std::invalid_argument const& ex) {
        } catch(std::out_of_range const& ex) {
        }
        return false;
    }

    void add_snapshot_folder(persistent::version_t ver) {
        auto snapshot = SNAPSHOT_PATH;
        snapshot += "/" + std::to_string(ver);
        auto res = add_snapshot_time(snapshot);
        //dbg_info(DL, "adding {}", snapshot);
        if(res != nullptr) {
            fill_at(snapshot, ver);
        }
    }

    void add_snapshot_folder_by_time(uint64_t ts_us) {
        auto snapshot = SNAPSHOT_PATH;
        snapshot += "/" + std::to_string(ts_us);
        auto res = add_snapshot_time(snapshot);
        //dbg_info(DL, "adding {}", snapshot);
        if(res != nullptr) {
            fill_at_by_time(snapshot, ts_us);
        }
    }

    // std::string path_while_op(const Node* node) const {
    std::string path_while_op(std::shared_ptr<const Node> node) const {
        std::vector<std::string> parts;
        for(; node != nullptr && (node->data.flag & OP_FLAG); node = node->parent) {
            parts.push_back(node->label);
        }
        std::string res;
        for(auto it = parts.rbegin(); it != parts.rend(); ++it) {
            res += "/" + *it;
        }

        return res;
    }

    // int put_to_capi(const Node* node) {
    int put_to_capi(const std::shared_ptr<Node> node) {
        // invariant: node is file
        ObjectWithStringKey obj;
        obj.key = path_while_op(node);
        obj.previous_version = INVALID_VERSION;
        obj.previous_version_by_key = INVALID_VERSION;
        obj.blob = Blob(node->data.bytes.get(), node->data.size, true);
        // TODO verify emplaced avoids blob deleting data :(

        auto result = capi.put(obj);
        for(auto& reply_future : result.get()) {
            auto reply = reply_future.second.get();
            dbg_default_trace("node({}) replied with version:{},ts_us:{}",
                   reply_future.first, std::get<0>(reply), std::get<1>(reply));
        }
        // TODO check for error

        return 0;
    }

    void fill_op_meta(const fs::path& prefix, const std::string& op_root) {
        auto opm = capi.find_object_pool(op_root);
        json j{{"valid", opm.is_valid()},
               {"null", opm.is_null()}};
        if(opm.is_valid() && !opm.is_null()) {
            j.emplace("pathname", opm.pathname);
            j.emplace("version", opm.version);
            j.emplace("timestamp_us", opm.timestamp_us);
            j.emplace("previous_version", opm.previous_version);
            j.emplace("previous_version_by_key", opm.previous_version_by_key);
            j.emplace("subgroup_type", std::to_string(opm.subgroup_type_index) + "-->" + DefaultObjectPoolMetadataType::subgroup_type_order[opm.subgroup_type_index].name());
            j.emplace("subgroup_index", opm.subgroup_index);
            j.emplace("sharding_policy", std::to_string(opm.sharding_policy));
            j.emplace("deleted", opm.deleted);
        }
        auto op_root_meta_path = prefix;
        op_root_meta_path += METADATA_PATH;
        op_root_meta_path += op_root;
        add_op_info(op_root_meta_path, j.dump(2));
    }

    void fill_at(const fs::path& prefix, persistent::version_t ver) {
        // TODO no version for list object pools
        auto str_paths = capi.list_object_pools(false, true);

        if(ver == CURRENT_VERSION) {
            auto meta_path = prefix;
            meta_path += METADATA_PATH;
            root->set(meta_path,
                      NodeData(METADATA_PREFIX_DIR), NodeData(METADATA_PREFIX_DIR));
        }
        for(const std::string& op_root : str_paths) {
            auto op_root_path = prefix;
            op_root_path += op_root;
            // op_root_path = /latest/..
            auto op_root_node = add_full_op_dir(op_root_path, op_root);
            if(op_root_node == nullptr) {
                dbg_default_error("In {}, op root node ({}) is not successfully created", __PRETTY_FUNCTION__, op_root_path);
                continue;
            }
            if(ver == CURRENT_VERSION) {
                fill_op_meta(prefix, op_root);
            }
            auto keys = get_keys(op_root, ver);
            std::sort(keys.begin(), keys.end(), std::greater<>());
            // sort removes files colliding with directory
            for(const auto& k : keys) {
                auto key_path = prefix;
                key_path += k;
                auto node = add_op_key(key_path, op_root);
                // colliding keys do not get added
                if(node == nullptr) {
                    dbg_default_trace("In {}, node is nullptr for key: {}", __PRETTY_FUNCTION__, k);
                    // update_contents(node, k, ver);
                    // //dbg_info(DL, "file: {}", std::quoted(reinterpret_cast<const char*>(node->data.bytes.data())));
                }
            }
        }
    }

    void fill_at_by_time(const fs::path& prefix, uint64_t ts_us) {
        // TODO no version for list object pools
        auto str_paths = capi.list_object_pools(false, true);

        for(const std::string& op_root : str_paths) {
            auto op_root_path = prefix;
            op_root_path += op_root;
            auto op_root_node = add_full_op_dir(op_root_path, op_root);
            if(op_root_node == nullptr) {
                continue;
            }

            auto keys = get_keys_by_time(op_root, ts_us);
            std::sort(keys.begin(), keys.end(), std::greater<>());
            // sort removes files colliding with directory
            for(const auto& k : keys) {
                auto key_path = prefix;
                key_path += k;
                // colliding keys do not get added
                auto node = add_op_key(key_path, op_root);
                if(node != nullptr) {
                    get_contents_by_time(node, k, ts_us);
                    // //dbg_info(DL, "file: {}", std::quoted(reinterpret_cast<const char*>(node->data.bytes.data())));
                }
            }
        }
    }


    int get_attr(const std::string& path, struct stat* stbuf) {
        auto node = get(path, false);
        if(node == nullptr) {
            return -ENOENT;
        }
        // not needed: st_dev, st_blksize, st_ino (unless use_ino mount option)
        // TODO maintain stbuf in data?
        stbuf->st_nlink = 1;
        stbuf->st_uid = fuse_get_context()->uid;
        stbuf->st_gid = fuse_get_context()->gid;
        // TODO merge timestamp for dirs, update during set
        int64_t sec = 0; // node->data.timestamp / 1'000'000;
        int64_t nano = 0; // (node->data.timestamp % 1'000'000) * 1000;
        stbuf->st_mtim = timespec{sec, nano};
        stbuf->st_ctim = stbuf->st_mtim;
        // TODO: need work for how to set last access time
        stbuf->st_atim = timespec{last_update_sec, 0};
        // if(uint64_t(last_update_sec) * 1'000'000 < node->data.timestamp) {
        //     stbuf->st_atim = stbuf->st_mtim;
        // }
        // - at prefix dir location add .info file ???

        // TODO timestamps messing with vim??

        if(node->data.flag & DIR_FLAG) {
            if(node->data.flag & OP_PREFIX_DIR) {
                stbuf->st_mode = S_IFDIR | 0555;
            } else {
                stbuf->st_mode = S_IFDIR | 0755;
            }
            stbuf->st_nlink = 2;  // TODO calculate properly
            for(const auto& [_, v] : node->children) {
                stbuf->st_nlink += v->data.flag & DIR_FLAG;
            }
        } else {
            // TODO somehow even when 0444, can still write ???
            stbuf->st_mode = S_IFREG | (node->data.flag & KEY_FILE ? 0744 : 0444);
            auto result = capi.get_size(path.substr(LATEST_PREFIX_SIZE), -1, true);
            for (auto& reply_future : result.get()) {
                size_t size = reply_future.second.get();
                stbuf->st_size = std::max(size, (size_t)BLOB_SIZE_OFFSET) - BLOB_SIZE_OFFSET;
                dbg_default_debug("Path: {}, size is : {}", path, size);
                break; 
            }
        }

        // dev_t st_dev;         /* ID of device containing file */
        // ino_t st_ino;         /* Inode number */
        // mode_t st_mode;       /* File type and mode */
        // nlink_t st_nlink;     /* Number of hard links */
        // uid_t st_uid;         /* User ID of owner */
        // gid_t st_gid;         /* Group ID of owner */
        // dev_t st_rdev;        /* Device ID (if special file) */
        // off_t st_size;        /* Total size, in bytes */
        // blksize_t st_blksize; /* Block size for filesystem I/O */
        // blkcnt_t st_blocks;   /* Number of 512B blocks allocated */

        // struct timespec st_atim; /* Time of last access */
        // struct timespec st_mtim; /* Time of last modification */
        // struct timespec st_ctim; /* Time of last status change */
        return 0;
    }

    std::string string() {
        std::stringstream ss;
        root->print(100, ss);
        return ss.str();
    }

    // void update_contents(Node* node, const std::string& path, persistent::version_t ver) {
    void update_contents(std::shared_ptr<Node> node, const std::string& path, persistent::version_t ver) {
        int record_id = extract_number(path);
        dbg_default_error("In {}, path: {}", __PRETTY_FUNCTION__, path);
        TimestampLogger::log(BEFORE_CAPI_GET,node_id,record_id,get_walltime());
        auto result = capi.get(path, ver, true);
        TimestampLogger::log(AFTER_CAPI_GET,node_id,record_id,get_walltime());
        for(auto& reply_future : result.get()) {
            auto reply = reply_future.second.get();
            if(ver == CURRENT_VERSION) {
                max_ver = std::max(max_ver, reply.version);
                max_timestamp = std::max(max_timestamp, reply.timestamp_us);
                node->data.writeable = true;
            }
            // TODO std::move ??
            Blob blob = reply.blob;
            blob.memory_mode = derecho::cascade::object_memory_mode_t::EMPLACED;
            node->data.bytes = std::shared_ptr<uint8_t[]>(new uint8_t[blob.size]);
            // memcpy(node->data.bytes.get(), blob.bytes, blob.size);

            node->data.bytes.reset((uint8_t*)blob.bytes);
            node->data.size = blob.size;
            // dbg_default_debug("In {}, path: {}, actual size: {}", __PRETTY_FUNCTION__, path, node->data.size);
            node->data.timestamp = reply.timestamp_us;
            return;
        }
    }

    // not to be called by latest
    // void get_contents_by_time(Node* node, const std::string& path, uint64_t ts_us) {
    void get_contents_by_time(std::shared_ptr<Node> node, const std::string& path, uint64_t ts_us) {
        auto result = capi.get_by_time(path, ts_us, true);
        // TODO only get file contents on open

        for(auto& reply_future : result.get()) {
            auto reply = reply_future.second.get();
            Blob blob = reply.blob;
            blob.memory_mode = derecho::cascade::object_memory_mode_t::EMPLACED;
            // TODO std::move ??
            node->data.bytes = std::shared_ptr<uint8_t[]>(new uint8_t[blob.size]);

            // memcpy(node->data.bytes.get(), blob.bytes, blob.size);

            node->data.bytes.reset((uint8_t*)blob.bytes);
            node->data.size = blob.size;
            node->data.timestamp = reply.timestamp_us;
            return;
        }
    }

    std::vector<std::string> get_keys(const std::string& path, persistent::version_t ver) {
        auto future_result = capi.list_keys(ver, true, path);
        return capi.wait_list_keys(future_result);
    }

    std::vector<std::string> get_keys_by_time(const std::string& path, uint64_t ts_us) {
        auto future_result = capi.list_keys_by_time(ts_us, true, path);
        return capi.wait_list_keys(future_result);
    }

    // TODO use object pool root meta file to edit version # and such?
    /**
     * Get the pointer for the node if it exists and fill in the KEY_FILE node's data content via update_contents
     * @param path: full path name for the node
    */
    // Node* get_file(const std::string& path) {
    std::shared_ptr<Node> get_file(const std::string& path) {
        // Node* node = root->get(path);
        std::shared_ptr<Node> node = root->get(path);
        if (!node->file_valid) {
            dbg_default_error("In {}, !node->data.file_valid", __PRETTY_FUNCTION__);
            dbg_default_error("In {}, getting file contents: {}", __PRETTY_FUNCTION__, path);
            // TODO op: path: should not include "/latest", see /pool1/k1, or /version
            auto new_path = path.substr(LATEST_PREFIX_SIZE);
            update_contents(node, new_path, CURRENT_VERSION);
        }
        return node;
    }

    // make a trash folder? (move on delete)
    // TODO op: new change
    /**
     * Get the pointer for the node if it exists and could fill in the KEY_FILE node's data content via get_file
     * @param path: full path name for the node
     * @param fill_contents: whether to fill the node's data content from capi.get from remote server
    */
    // Node* get(const std::string& path, bool fill_contents) {
    std::shared_ptr<Node> get(const std::string& path, bool fill_contents) {
        // std::cout << "\nEntered fcc_hl:get: " << path << std::endl;
        // Node* node = root->get(path);
        std::shared_ptr<Node> node = root->get(path);
        if (node == nullptr) {
            dbg_default_debug("In {}, cc_hl:Node is nullptr, path: {}", __PRETTY_FUNCTION__, path);
            return node;
        }
        if (node->data.flag == KEY_FILE && fill_contents) {
            dbg_default_trace("In {}, call get_file with path: {}", __PRETTY_FUNCTION__, path);
            get_file(path);
        }
        // std::cout << "\nExited fcc_hl:get: " << path << std::endl;
        return node;
    }


    // TODO: add documentation for this function
    // void update_dir(Node* node, const fs::path& prefix) {
    void update_dir(std::shared_ptr<Node> node, const fs::path& prefix) {
        dbg_default_trace("Entered {}", __PRETTY_FUNCTION__);
        if (node == nullptr) {
            dbg_default_trace("In {} node is nullptr", __PRETTY_FUNCTION__);
        }
        if (node->data.objp_name.empty()) {
            dbg_default_trace("In {} Entered node->objp_subdir.empty()", __PRETTY_FUNCTION__);
            auto str_paths = capi.list_object_pools(false, true);
            for(const std::string& op_root : str_paths) {
                auto op_root_path = prefix;
                op_root_path += op_root;
                dbg_default_trace("In {} op_root_path: {}", __PRETTY_FUNCTION__, op_root_path);
                auto op_root_node = add_full_op_dir(op_root_path, op_root);
                if(op_root_node == nullptr) {
                    continue;
                }
            }
        } else {
            dbg_default_trace("In {} Entered !node->objp_subdir.empty()", __PRETTY_FUNCTION__);
            // check if readdir will call read_buf by adding prints
            // compile and run first
            // distinguish for which function should call get_dir vs. get_file
            // 1) get capi.list_keys(with path),
            // 2) cha lou using add_op_key and set objp_path
            // 3) try not calling get_contents (empty files)
            std::string op_root = node->data.objp_name;
            dbg_default_trace("In {} op_root: {}", __PRETTY_FUNCTION__, op_root);
            auto keys = get_keys(op_root, CURRENT_VERSION);
            std::sort(keys.begin(), keys.end(), std::greater<>());
            // sort removes files colliding with directory
            for(const auto& k : keys) {
                auto key_path = prefix;
                key_path += k;
                auto node = add_op_key(key_path, op_root);
                // colliding keys do not get added
                if(node != nullptr) {
                    // update_contents(node, k, CURRENT_VERSION);
                    // //dbg_info(DL, "file: {}", std::quoted(reinterpret_cast<const char*>(node->data.bytes.data())));
                }
            }
        }

        // Update local directories that user created via mkdir.
        // These local directories do not persistent in cascade or are visible to other fuse client processes,
        // until a file (kv object) is created under the directory, and put to cascade
        for(auto it = local_latest_dirs.begin(); it != local_latest_dirs.end();) {
            if(nearest_object_pool_root(*it) == nullptr) {
                // TODO verify for op_root deleted
                it = local_latest_dirs.erase(it);
            } else {
                add_op_key_dir(*it);
                ++it;
            }
        }
    }

    // Node* get_dir(const std::string& path) {
    std::shared_ptr<Node> get_dir(const std::string& path) {
        // Node* node = root->get(path);
        std::shared_ptr<Node> node = root->get(path);
        update_dir(node, LATEST_PATH);
        return node;
    }
};

}  // namespace cascade
}  // namespace derecho
