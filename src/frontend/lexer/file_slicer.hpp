/**
 * @file file_slicer.hpp
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

#include <string_view>

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