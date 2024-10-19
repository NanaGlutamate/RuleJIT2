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