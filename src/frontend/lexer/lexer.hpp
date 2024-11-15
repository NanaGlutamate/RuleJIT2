/**
 * @file lexer.hpp
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

#include <type_traits>

namespace rulejit {

class Lexer {
    Lexer();

    template <typename Ty, typename ...Args>
        requires std::is_constructible_v<Ty, Lexer, Args...>
    Ty collect(Args&& ...args);
};

};