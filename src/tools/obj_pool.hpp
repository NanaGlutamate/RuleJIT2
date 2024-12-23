/**
 * @file obj_pool.hpp
 * @author nanaglutamate
 * @brief 
 * @date 2024-11-15
 * 
 * @details 
 * 
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>nanaglutamate</td><td>2024-11-15</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

#include <unordered_map>
#include <vector>
#include <span>
#include <memory>
#include <functional>

#include "defs.hpp"

namespace tools {

template<typename Obj>
struct Pool;

template<typename Obj>
    requires std::is_base_of_v<Poolable<Obj>, Obj>
struct Poolable {
    Pool<Obj>* p;
};

template<typename Obj>
    requires std::is_base_of_v<Poolable<Obj>, Obj>
struct Pool {
    void release(Obj* obj) {
        obj->~Obj();
        obj->p = std::bit_cast<Pool<Obj>*>(recycled);
        recycled = obj;
    }
    using Deleter = decltype([](Obj* obj){ obj->p->release(obj); });
    template<typename ...Args>
        requires std::is_constructible_v<Obj, Pool<Obj>*, Args...>
    std::unique_ptr<Obj, Deleter> make_unique(Args&& ...args) {
        if (recycled) {
            Obj* target = recycled;
            recycled = std::bit_cast<Obj*>(recycled->p);
            return new(target) Obj{this, std::forward<Args>(args)...};
        }
        return &*data.emplace_back(this, std::forward<Args>(args)...);
    }
private:
    std::deque<Obj> data;
    Obj* recycled = nullptr;
};

}