/**
 * @file parser.hpp
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

#include <array>
#include <tuple>
#include <unordered_map>

namespace rulejit {

struct CachedParser {
    std::array<std::unordered_map<std::tuple<>, >, 5> cache;
};

}