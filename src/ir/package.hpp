#pragma once

#include <unordered_map>
#include <variant>
#include <vector>

#include "defs.hpp"
#include "tplate.hpp"

namespace rulejit {

struct Package {
    StringToken packageName;
    std::unordered_map<StringToken, std::variant<TypeTemplateToken, FunctionTemplateToken, GlobalVarToken, PackagedToken>> exportToken, innerToken;
};

struct PackageManager {
    struct Context {

    };
    std::vector<u8> serialize(const Package& pkg) {
        return {};
    }
    Package deserialize(const std::vector<u8>& stream, Context* ctx) {
        return {};
    }
};

}