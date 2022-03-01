#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

bool SimpleLRU::delTail() {
    _cur_size -= (_lru_tail->key.size() + _lru_tail->value.size());

    _lru_index.erase(_lru_tail->key);

    _lru_tail = _lru_tail->prev;
    _lru_tail->next = nullptr;
}

bool SimpleLRU::putNewNode(const std::string &key, const std::string &value) {
    _cur_size += key.size() + value.size();

    while (_cur_size > _max_size) {
        delTail();
    }

    auto tmp = std::move(_lru_head);
    _lru_head = std::make_unique<lru_node>(key, value);
    if (tmp != nullptr) {
        tmp->prev = _lru_head.get();
        _lru_head->next = std::move(tmp);
    } else {
        _lru_tail = _lru_head.get();
    }

    _lru_index.emplace(std::ref(_lru_head->key), std::ref(*_lru_head));

    return true;
}

bool SimpleLRU::eraseNode(StringLruNodeMap::iterator &it) {
    if (it->second.get().next.get() != nullptr) {
        it->second.get().next.get()->prev = it->second.get().prev;
    }

    if (it->second.get().prev != nullptr) {
        it->second.get().prev->next = std::move(it->second.get().next);
    }

    _lru_index.erase(it);

    _cur_size -= (it->second.get().key.size() + it->second.get().value.size());

    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) { 
    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) {
        return putNewNode(key, value);
    }

    eraseNode(it);

    return putNewNode(key, value); 
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) { 
    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) {
        return putNewNode(key, value);
    }

    return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) { 
    auto it = _lru_index.find(std::ref(key));
    if (it != _lru_index.end()) {
        eraseNode(it);

        return putNewNode(key, value);
    }

    return false; 
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) { 
    auto it = _lru_index.find(key);
    if (it != _lru_index.end()) {
        return eraseNode(it);
    }

    return false; 
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
    auto it = _lru_index.find(key);
    if (it != _lru_index.end()) {
        value = it->second.get().value;

        return true;
    }

    return false;
}

} // namespace Backend
} // namespace Afina
