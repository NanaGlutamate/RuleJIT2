#pragma once

#include <unordered_map>
#include <vector>
#include <span>
#include <memory>
#include <functional>

#include "defs.hpp"

namespace rulejit {

/**
 * @brief merge multi alloc, release at once
 * 
 * TODO: align?
 * 
 * @tparam OBJ 
 */
template<typename OBJ, size_t CHUNK_CNT = PageSize / sizeof(OBJ)>
class ObjectPool {
public:
    explicit ObjectPool() noexcept : next(nullptr), end(nullptr) { 
        pool.reserve(32);
    };
    ~ObjectPool() = default;

    ObjectPool(ObjectPool&& obj) = delete;
    ObjectPool& operator=(ObjectPool&& obj) = delete;
    ObjectPool(const ObjectPool& obj) noexcept = delete;
    auto operator=(const ObjectPool& obj) noexcept = delete;

    // TODO: unconstructed object
    OBJ* get() {
        if (next == end) [[unlikely]] {
            auto new_chunk = std::make_unique<OBJ[]>(CHUNK_CNT);
            next = new_chunk.get();
            end = next + CHUNK_CNT;
            pool.emplace_back(std::move(new_chunk));
        }
        return next++;
    }
    // std::span<OBJ> get_list(size_t len) {
    //     // TODO: if end - next is too small, will produce memory fragment
    //     if (next + len <= end) {
    //         auto tmp = next;
    //         next += len;
    //         return std::span{tmp, len};
    //     }
    //     return std::span<OBJ>{pool.emplace(std::make_unique<OBJ[]>(CHUNK_CNT))->get(), len};
    // }
    // template<size_t L>
    //     requires (L != std::dynamic_extent) 
    // std::span<OBJ, L> get_array() {
    //     return std::span<OBJ, L>{get_list(L).data(), L};
    // }
private:
    std::vector<std::unique_ptr<OBJ[]>> pool;
    OBJ* next;
    OBJ* end;
};

template <typename T, size_t CHUNK_CNT = size_t(-1)>
struct PooledList {
    struct Node {
        T value;
        Node* next;
    };
    using Pool = ObjectPool<Node, CHUNK_CNT == size_t(-1) ? (4 * 1024 / sizeof(Node)) : CHUNK_CNT>;
    Pool* pool;
    Node* head;
    explicit PooledList(Pool& p): pool(&p), head(nullptr) {}
    PooledList(const PooledList& other) = default;
    PooledList(PooledList&& other) = default;
    ~PooledList() = default;
    template <typename ...Ty>
    T& emplace_front(Ty&&... v) {
        auto p = pool->get();
        head = new(p) Node{{std::forward<>(v)...}, head};
    }
};

/**
 * @brief 
 * 
 * @tparam 1024 
 */
template<size_t PAGE_SIZE = 4 * 1024>
class MultiObjPool {
public:
    MultiObjPool() = default;
    MultiObjPool(const MultiObjPool&) = delete;
    MultiObjPool(MultiObjPool&&) = delete;
    auto operator=(const MultiObjPool&) = delete;
    auto operator=(MultiObjPool&&) = delete;
    ~MultiObjPool() {
        for (auto&& f : defered) {
            f();
        }
    }

    template<typename OBJ>
    ObjectPool<OBJ, PAGE_SIZE / sizeof(OBJ)>& getPool() {
        static std::unordered_map<MultiObjPool*, ObjectPool<OBJ, PAGE_SIZE / sizeof(OBJ)>> pool;
        // TODO: lock
        auto it = pool.find(this);
        if (it == pool.end()) {
            it = pool.emplace(this, ObjectPool<OBJ, PAGE_SIZE / sizeof(OBJ)>{});
            defered.emplace([it](){ pool.erase(it); });
        }
        return it->second();
    }
private:
    std::vector<std::function<void()>> defered;
};

}