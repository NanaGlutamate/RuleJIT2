#pragma once

#include <tuple>

namespace rulejit {

/**
 * @brief 
 * func: T -> (T, V)
 * collector: T -> (func1, func2, ...) -> (T, V1, V2, ...)
 * 
 * @tparam T 
 * @tparam Ret 
 */
template <typename T, typename ...Ret>
struct ChainedObj {
    ChainedObj(T& tar) requires (sizeof...(Ret) == 0): target(tar) {}
    template <typename Func, typename ...Args>
        requires std::is_invocable_v<Func, T, Args...>
    ChainedObj<T, Ret..., U> collect(Func&& collector, Args&& ...args) {
        return 
    }
  private:
    template <typename ...Args>
    ChainedObj(T& tar, Args&& ...args): target(tar), ret(std::forward<Args>(args)...) {}
    T& target;
    std::tuple<Ret...> ret;
};

}