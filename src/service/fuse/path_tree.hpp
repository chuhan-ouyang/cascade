#include <filesystem>
#include <vector>
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

template <typename T>
struct PathTree {
    std::string label;
    T data;

    PathTree<T>* parent;
    std::unordered_map<std::string, PathTree<T>*> children;
    // std::string objp_subdir;

    PathTree(std::string label, T data, PathTree<T>* parent)
            : label(label), data(data), parent(parent) {}
    PathTree(std::string label, PathTree<T>* parent, T data)
            : label(label), parent(parent), data(data) {}
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

    // TODO op: set if not exist: bu que (doesn't duo shan); otherwise get existing node
    // Will return a node ptr no matter if it is newly created or already exists
    PathTree<T>* set(const fs::path& path, T intermediate, T data) {
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
        for(++it; it != path.end(); ++it) {
            if(!cur->children.count(*it)) {
                created_new = true;
                // parent, data
                // TODO op: if field of intermediate's NodeFlag type is KEY_DIR or KEY_File
                // Need to set "next"'s objp_dir to the path
                PathTree<T>* next = new PathTree<T>(*it, cur, intermediate);
                // if (intermediate.flag == (KEY_DIR | KEY_FILE)) {
                   //  next->objp_subdir = path;
                // }
                cur->children.insert({*it, next});
                cur = next;
            } else {
                cur = cur->children.at(*it);
            }
        }
        if(!created_new) {
            dbg_default_trace("In {}, !created_new", __PRETTY_FUNCTION__);
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
