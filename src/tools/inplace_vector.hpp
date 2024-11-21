/**
 * @file inplace_vector.hpp
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

#include <cassert>
#include <type_traits>

namespace tools {

template <typename T, size_t Capacity>
struct inplace_vector {
    size_t size = 0;
    inline static constexpr size_t capacity = Capacity;

    struct alignas(T) Data{
        char bytes[sizeof(T)];
    } data[Capacity];

    T* begin() { return reinterpret_cast<T*>(data[0].bytes); }
    T* end() { return reinterpret_cast<T*>(data[size].bytes); }
    template <typename ...Args>
    T& emplace(Args&&... args) {
        assert(size != capacity);
        new(data[size].bytes) T{std::forward<Args>(args)...};
        size++;
    }
    void pop_back() {
        size--;
        reinterpret_cast<T*>(data[size].byte)->~T();
    }
};

}