#pragma once

#include <string>
#include <string_view>
#include <unordered_set>
#include <deque>

#include "defs.hpp"

namespace rulejit {

struct StringPool {
    StringToken take(std::string_view s) {
        auto it = table.find(s);
        if (it == table.end()) {
            auto& stored = storage.emplace_back(std::string(s));
            it = table.emplace(stored).first;
        }
        return {&*it};
    }
private:
    std::deque<std::string> storage;
    std::unordered_set<std::string_view> table;
};

}

template <>
struct std::hash<rulejit::StringToken> {
    static size_t operator()(rulejit::StringToken v) noexcept {
        return hash<const std::string_view*>{}(v.data);
    }
};