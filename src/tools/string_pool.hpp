/**
 * @file string_pool.hpp
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

#include <deque>
#include <span>
#include <string>
#include <string_view>
#include <unordered_set>

#include "defs.hpp"

namespace tools {

using rulejit::StringToken;

struct StringPool {
    StringToken take(std::string&& s) {
        auto it = table.find(s);
        if (it == table.end()) {
            auto& stored = storage.emplace_back(std::move(s));
            it = table.emplace(stored).first;
        }
        return {&*it};
    }

    StringToken take(std::string_view s) {
        auto it = table.find(s);
        if (it == table.end()) {
            auto& stored = storage.emplace_back(std::string(s));
            it = table.emplace(stored).first;
        }
        return {&*it};
    }

    template <typename Callback>
        requires requires(Callback cb, StringToken token) { cb(token); }
    void takeCompressedPool(std::string&& pool, std::span<std::string_view> s, Callback& cb) {
        storage.push_back(std::move(pool));
        for (auto sv : s) {
            auto it = table.emplace(sv).first;
            cb(*it);
        }
    }

  private:
    std::deque<std::string> storage;
    std::unordered_set<std::string_view> table;
};

} // namespace tools

template <>
struct std::hash<tools::StringToken> {
    static size_t operator()(tools::StringToken v) noexcept { return hash<const std::string_view*>{}(v.data); }
};