#pragma once

#include <optional>
#include <variant>
#include <type_traits>

template<typename H, typename T0, typename ...T>
struct Lazy{
    Lazy(H&& f): holder(std::forward<H>(f)) {}
    T0 value() requires(sizeof...(T) == 0) {
        return holder();
    }
    std::variant<std::nullopt_t, T0, T...> value() requires(sizeof...(T) != 0) {
        return holder();
    }
    // only enable operator|| of 2 r-value
    template<typename H_, typename T0_, typename ...T_>
    auto operator||(Lazy<H_, T0_, T_...>&& other) && {
        auto f = [lhs{std::move(holder)}, rhs{std::move(other.holder)}](){
            
        };
        // TODO: RVO?
        return Lazy<decltype(f), T0, T..., T0_, T_...>{std::move(f)};
    }
  private:
    H holder;
};

template<typename F>
Lazy(F) -> Lazy<F, std::invoke_result_t<F>>;

template<typename F, typename ...A>
auto lazy_invoke(F&& f, A&&...args){
    return Lazy{[f{std::forward<F>(f)}, ...args{std::forward<A>(args)}](){return std::invoke(f, args...);}};
}