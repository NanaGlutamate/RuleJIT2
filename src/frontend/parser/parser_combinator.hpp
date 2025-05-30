/**
 * @file parser_combinator.hpp
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
#include <vector>

#include "frontend/lexer/lexer.hpp"

namespace rulejit {

template <typename T>
concept ParserNode = requires { T::operator(); };

auto def = []() static {};

} // namespace rulejit