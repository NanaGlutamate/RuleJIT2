/**
 * @file package.hpp
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

#include <unordered_map>
#include <variant>
#include <vector>

#include "defs.hpp"
#include "tplate.hpp"

namespace rulejit {

struct Package {
    FunctionTemplateToken onload;
    StringToken packageName;
    std::unordered_map<StringToken, std::variant<TypeTemplateToken, FunctionTemplateToken, GlobalVarToken, TraitToken, ImplToken>> exportToken, innerToken;
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
private:
    
};

}