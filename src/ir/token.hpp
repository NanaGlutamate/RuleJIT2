#pragma once

#include <unordered_map>
#include <variant>

#include "defs.hpp"

namespace rulejit {

struct TokenManager {
private:
    struct Package {
        std::unordered_map<StringToken, std::variant<TypeTemplateToken, FunctionTemplateToken, GlobalVarToken>> exportToken, innerToken;
    };
    std::unordered_map<StringToken, Package> table;
};

}