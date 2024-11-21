/**
 * @file cached.hpp
 * @author nanaglutamate
 * @brief tools to create cached function
 * @date 2024-11-17
 * 
 * @details 
 * 
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>nanaglutamate</td><td>2024-11-17</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

#include <type_traits>
#include <memory>
#include <any>
#include <unordered_map>
#include <atomic>
#include <tuple>

namespace tools {

template <typename F, typename T>
struct CachedFunc;

struct Cache {
private:
    inline static std::atomic<size_t> uuidCounter = 0;
    std::unordered_map<size_t, std::any> caches;
public:
    static size_t getCounter() { return uuidCounter++; }

    template<typenamne R, typename ...Arg>
    struct CacheObject {};
};

template <typename R, typename ...Arg, typename T>
    requires (std::is_same_v<R, std::remove_cvref_t<R>> || std::is_same_v<R, const std::remove_cvref_t<R>&>)
            && std::is_base_of_v<CachedFunc<R(Arg...), T>, T>
            && requires { T{}.evaluate(); }
            // && (std::is_same_v<Arg, std::remove_cv_ref_t<Arg>>...)
struct CachedFunc<R(Arg...), T> {
    R operator(Cache c, Arg... arg) {
        static size_t thisCounter = Cache::getCounter();
        auto& cache = c.caches[thisCounter];
        if (auto it = cache.find({arg...}); it != cache.end()) {
            return it->second;
        }
        auto [it, _] = cache.emplace({arg...}, this.evaluate());
        return it->second;
    }
private:

};

}
