#pragma once

#include <optional>
#include <string_view>

namespace rulejit {

struct Token {
    std::string_view token;
};

struct TokenStream {
    std::string_view src;
    Token next(Token token) {
        return {};
    }
    Token iter() {
        return {};
    }
};

struct Context {};

}