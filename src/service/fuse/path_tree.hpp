#include <filesystem>
#include <vector>
#include <atomic>
#include <derecho/utils/logger.hpp>

namespace fs = std::filesystem;

enum NodeFlag : uint32_t {
    ROOT_DIR = 1 << 0,

    OP_PREFIX_DIR = 1 << 1,
    OP_PATH_DIR = 1 << 2,

    KEY_DIR = 1 << 3,
    KEY_FILE = 1 << 4,

    LATEST_DIR = 1 << 5,

    METADATA_PREFIX_DIR = 1 << 6,
    METADATA_INFO_FILE = 1 << 7,

    SNAPSHOT_ROOT_DIR = 1 << 8,
    SNAPSHOT_TIME_DIR = 1 << 9,
};

// TODO: make PathTreeNode cache aligned
// invariant: object T is a fixed-size length variable
template <typename T>
struct PathTree {
    // last path of the full path name
    // Ex. If the full path name is /pool1/k1, its label is k1
    std::string label;
    char label_padding[64 - sizeof(std::string)]; 

    T data;
    char data_padding[64 - sizeof(T) % 64]; 

    std::shared_mutex mutex;
    char mutex_padding[64 - sizeof(std::shared_mutex) % 64]; 
    
    bool file_valid = false;
    char bool_padding[64 - sizeof(bool)];

    PathTree<T>* parent;
    char parent_padding[64 - sizeof(PathTree<T>*) % 64];

    std::unordered_map<std::string, PathTree<T>*> children; 
    // std::unordered_map<std::string, std::shared_ptr<PathTree<T>>> children;

    char children_padding[64 - sizeof(std::unordered_map<std::string, PathTree<T>*>) % 64];

    // std::string objp_subdir;
    PathTree() {}

    PathTree(std::string label, T data, PathTree<T>* parent)
            : label(label), data(data), parent(parent) {}

    // PathTree(std::string label, PathTree<T>* parent, T data)
    //         : label(label), parent(parent), data(data) {}

    PathTree(std::string label, T data) : PathTree(label, data, nullptr) {}

    ~PathTree() {
        for(auto& [k, v] : children) {
            delete v;
        }
    }

    std::vector<std::string> entries() const {
        std::vector<std::string> res;
        res.reserve(children.size());
        for(const auto& [k, _] : children) {
            res.push_back(k);
        }
        return res;
    }

    // TODO: try deleting
    std::string absolute_path() const {
        std::vector<std::string> parts;
        for(const PathTree<T>* node = this; node != nullptr; node = node->parent) {
            parts.push_back(node->label);
        }
        std::string res;
        for(auto it = parts.rbegin(); it != parts.rend(); ++it) {
            if(res.compare("") != 0 && res.compare("/") != 0) {
                res += "/";
            }
            res += *it;
        }

        return res;
    }

    void print(int depth = 0, std::ostream& stream = std::cout,
               int pad = 0) const {
        for(int i = 0; i < pad - 1; ++i) {
            stream << "  ";
        }
        if(pad) {
            stream << "|-";
        }
        stream << label << "\n";
        if(children.empty() || depth == 0) {
            return;
        }
        for(const auto& [_, v] : children) {
            v->print(depth - 1, stream, pad + 1);
        }
        if(pad == 0) {
            stream << "\n\n";
        }
    }

    /** 
     *  @fn create a new node based on the path if the node doesn't exist or update the existing node's data
     *  @tparam path is the full path name of the final node to set
     *  @tparam intermediate_data is the parents' data of final node to set if parent nodes don't exist
     *  @tparam data is the final node's data to set in the tree 
     *  @return Will return a node ptr no matter if it is newly created or already exists
    */
    PathTree<T>* set(const fs::path& path, T intermediate_data, T data) {
        if(path.empty()) {
            return nullptr;
        }
        auto it = path.begin();
        if(*it != label) {
            dbg_default_trace("In {}, *it != label", __PRETTY_FUNCTION__);
            return nullptr;
        }
        bool created_new = false;
        PathTree<T>* cur = this;
        // Iterate from root to child directory
        for(++it; it != path.end(); ++it) {
            // Create directory if it is in the path but currently does not exist
            if(!cur->children.count(*it)) {
                created_new = true;
                PathTree<T>* next = new PathTree<T>(*it, intermediate_data, cur);
                cur->children.insert({*it, next});
                cur = next;
            } else {
                cur = cur->children.at(*it);
            }
        }
        if(!created_new) {
            dbg_default_trace("In {}, !created_new at path: {}", __PRETTY_FUNCTION__, path);
            // return nullptr;
        }
        cur->data = data;
        return cur;
    }

    // returns nullptr on fail or location does not exist
    PathTree<T>* get(const fs::path& path) {
        if(path.empty()) {
            return nullptr;
        }
        auto it = path.begin();
        if(*it != label) {
            return nullptr;
        }
        PathTree<T>* cur = this;
        for(++it; it != path.end(); ++it) {
            if(!cur->children.count(*it)) {
                return nullptr;
            }
            cur = cur->children.at(*it);
        }
        return cur;
    }

    PathTree<T>* get_while_valid(const fs::path& path) {
        if(path.empty()) {
            return nullptr;
        }
        auto it = path.begin();
        if(*it != label) {
            return nullptr;
        }
        PathTree<T>* cur = this;
        for(++it; it != path.end(); ++it) {
            if(!cur->children.count(*it)) {
                return cur;
            }
            cur = cur->children.at(*it);
        }
        return cur;
    }

    PathTree<T>* extract(const fs::path& path) {
        // TODO use??
        PathTree<T>* cur = get(path);
        if(cur == nullptr || cur->parent == nullptr) {
            return nullptr;
        }
        PathTree<T>* par = cur->parent;
        par->children.erase(cur->label);

        cur->parent = nullptr;
        return cur;
    }

    // deletes node and replaces with replacement. updating parent
    // requires same label
    static bool replace(PathTree<T>* node, PathTree<T>* replacement) {
        if(node == nullptr || replacement == nullptr || node->label != replacement->label || node->parent == nullptr) {
            return false;
        }
        PathTree<T>* par = node->parent;
        replacement->parent = par;
        par->children[node->label] = replacement;
        delete node;
        return true;
    }
};
