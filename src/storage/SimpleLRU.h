#ifndef AFINA_STORAGE_SIMPLE_LRU_H
#define AFINA_STORAGE_SIMPLE_LRU_H

#include <map>
#include <memory>
#include <mutex>
#include <string>

#include <afina/Storage.h>

namespace Afina {
namespace Backend {

/**
 * # Map based implementation
 * That is NOT thread safe implementaiton!!
 */
class SimpleLRU : public Afina::Storage {
public:
    SimpleLRU(size_t max_size = 1024) : _max_size(max_size), _cur_size(0) {}

    ~SimpleLRU() {
        _lru_index.clear();
        while(_lru_head) {
            if (_lru_head.get()->next != nullptr) {
                _lru_head.get()->next->prev = nullptr;
                _lru_head = std::move(_lru_head.get()->next);
            } else {
                _lru_head.reset(nullptr);
            }
        }
    }

    // Implements Afina::Storage interface
    bool Put(const std::string &key, const std::string &value) override;

    // Implements Afina::Storage interface
    bool PutIfAbsent(const std::string &key, const std::string &value) override;

    // Implements Afina::Storage interface
    bool Set(const std::string &key, const std::string &value) override;

    // Implements Afina::Storage interface
    bool Delete(const std::string &key) override;

    // Implements Afina::Storage interface
    bool Get(const std::string &key, std::string &value) override;

private:
    // LRU cache node
    using lru_node = struct lru_node {
        const std::string key;
        std::string value;
        lru_node* prev;
        std::unique_ptr<lru_node> next;

        lru_node(
            const std::string key, 
            const std::string value,
            lru_node* prev = nullptr,
            std::unique_ptr<lru_node> next = nullptr
        ) : key(key), value(value), prev(prev), next(std::move(next)) {};

    };

    // Maximum number of bytes could be stored in this cache.
    // i.e all (keys+values) must be not greater than the _max_size
    std::size_t _max_size;

    std::size_t _cur_size;

    // Main storage of lru_nodes, elements in this list ordered descending by "freshness": in the head
    // element that wasn't used for longest time.
    //
    // List owns all nodes
    std::unique_ptr<lru_node> _lru_head;

    lru_node* _lru_tail;

    using StringRef = std::reference_wrapper<const std::string>;
    using LruNodeRef = std::reference_wrapper<lru_node>;
    using StringLruNodeMap = std::map<StringRef, LruNodeRef, std::less<std::string>>;

    // Index of nodes from list above, allows fast random access to elements by lru_node#key
    StringLruNodeMap _lru_index;

    bool delTail();

    bool putNewNode(const std::string &key, const std::string &value);

    bool eraseNode(StringLruNodeMap::iterator &it);
};

} // namespace Backend
} // namespace Afina

#endif // AFINA_STORAGE_SIMPLE_LRU_H
