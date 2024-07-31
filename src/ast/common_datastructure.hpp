#pragma once

#include <string_view>

namespace rulejit {

struct Token {
    std::string_view token;
};
struct Type;
struct TypeHandle { Type* type; };

}