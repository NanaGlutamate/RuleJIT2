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
#include <string_view>
#include <string>
#include <tuple>
#include <unordered_map>
#include <deque>
#include <utility>

namespace rulejit {

class LexerContext {
  public:
    LexerContext(std::string_view src): src(src), cache() {}
    LexerContext(LexerContext&& other) noexcept: src(other.src), cache(std::move(other.cache)) {}
    LexerContext& operator=(LexerContext&& other) noexcept {
        src = other.src;
        cache = std::move(other.cache);
    }

    enum struct TokenType {
        // error
        kUnknown = 0,
        // ;
        kEndLine,
        // +
        kSymbol,
        // a
        kIdentifier,
        // 1, 0xFF, 0b100
        kInt,
        // 3e8, 0.0, 0f, 0.
        kReal,
        // end of file
        kEOF,
        // normal string like "123", normal string can contains \n
        kStringFull,
        // for template string like "123{a}123{b}123"
        kStringBegin,
        kStringMiddle,
        kStringEnd,
    };

    enum struct Guidence {
        kNone = 0,
        kTemplateString,
    };

    struct LexerIterator {
        std::string_view token;
        TokenType type;
    };

    LexerIterator first() {
        return next(0, Guidence::kNone);
    }

    LexerIterator next(LexerIterator pre, Guidence guide = Guidence::kNone) {
        return next(pre.token.data() - src.data(), guide);
    }

  private:
    LexerIterator next(size_t startIndex, Guidence guide) {
        if (auto it = cache.find({startIndex, guide}); it == cache.end()) {
            return it->second;
        }
        size_t realStartIndex = skipSpaceAndComment(startIndex);
        if (realStartIndex == src.length()) {
            return cache[{startIndex, guide}] = {src.substr(realStartIndex, 0), TokenType::kEOF};
        }
        auto [endIndex, type] = expandToken(realStartIndex, guide);
        return cache[{startIndex, guide}] = {src.substr(realStartIndex, endIndex - startIndex), type};
    }

    size_t skipSpaceAndComment(size_t index) {
        while (index < src.length() && isspace(src[index])) {
            index++;
        }
        if (index + 1 < src.length() && src[index] == '/' && src[index + 1] == '/') {

        }
        return index;
    }

    std::tuple<size_t, TokenType> expandToken(size_t startIndex, Guidence guide) {
        char firstChar = src[startIndex];
        if (guide == Guidence::kTemplateString) {
            if (firstChar != '}') {
                return {startIndex, TokenType::kUnknown};
            }
        } if (isdigit(firstChar)) {
            // numerical
        } else if (isSymbol(firstChar)) {

        } else if (isLegalIdent(firstChar)) {
            
        } else if (firstChar == '"') {

        }
    }

    bool isSymbol(char c) {
        return isInfix(c) || c == '[' || c == ']'
                || c == '(' || c == ')'
                || c == '{' || c == '}'
                || c == '.' || c == ':'
                || c == '!';
    }

    bool isInfix(char c) {
        return c == '+' || c == '-' || c == '*' || c == '/'
                || c == '&' || c == '|' || c == '^';
    }

    bool isTwoCharSymbol(char a, char b) {
        return (isInfix(a) && b == '=')
                || (a == ':' && b == ':');
    }

    bool isLegalIdent(char c) {
        // utf-8
        return isalpha(c) || isdigit(c) || c == '_' || c < 0;
    }

    std::unordered_map<std::tuple<size_t, Guidence>, LexerIterator> cache;
    std::string_view src;
};

};