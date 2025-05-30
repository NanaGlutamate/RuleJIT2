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

#include <cctype>
#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace rulejit {

using CharType = char;
using StringViewType = std::string_view;

struct LexerIterator {
    StringViewType token;
};

class LexerContext {
  public:
    LexerContext(StringViewType src) : src(src), cache() {}
    LexerContext(LexerContext&& other) noexcept : src(other.src), cache(std::move(other.cache)) {}
    LexerContext& operator=(LexerContext&& other) noexcept {
        src = other.src;
        cache = std::move(other.cache);
        return *this;
    }

    enum struct TokenType {
        // error
        kUnknown = 0,
        // ;
        kEndLine,
        // +, >=<, ==> (caution: prefix operator- may regarded as part of symbol combined with previous one if no space
        // to split them)
        kSymbol,
        // a, keywords also included
        kIdentifier,
        // 1, 0xFF, 0b100
        kInt,
        // 3e8, 0.0, 0f, 0.,
        kReal,
        // end of file
        kEOF,
        // char
        kChar,
        // normal string like "123" or f"123", normal string can contains escape char like \n
        kStringFull,
        // for template string like f"123{a}123{b}123" -> ("f\"123{", begin), ..., ("}123{", middle), ..., ("}123\"",
        // end)
        kStringBegin,
        kStringMiddle,
        kStringEnd,
        __size,
    };

    enum struct Guidance {
        kNone = 0,
        kTemplateString,
        __size,
    };

    struct LexerIteratorWithType {
        LexerIterator iter;
        TokenType type;
    };

    LexerIteratorWithType first() { return next(0, Guidance::kNone); }

    LexerIteratorWithType next(LexerIterator pre, Guidance guide = Guidance::kNone) {
        return next(pre.token.data() + pre.token.size() - src.data(), guide);
    }

  private:
    LexerIteratorWithType next(size_t startIndex, Guidance guide) {
        size_t cacheKey = getCacheKey(startIndex, guide);
        if (auto it = cache.find(cacheKey); it == cache.end()) {
            return it->second;
        }
        size_t realStartIndex = skipSpaceAndComment(startIndex);
        auto [endIndex, type] = expandToken(realStartIndex, guide);
        return cache[cacheKey] = {src.substr(realStartIndex, endIndex - startIndex), type};
    }

    size_t skipSpaceAndComment(size_t index) {
        using namespace std::literals;
        while (index < src.length() && isspace(src[index]) || src.substr(index, 2) == "//"sv) {
            while (index < src.length() && isspace(src[index])) {
                index++;
            }
            if (src.substr(index, 2) == "//"sv) {
                index += 2;
                while (index < src.length() && src[index] != '\n') {
                    index++;
                }
            }
        }
        return index;
    }

    std::pair<size_t, TokenType> expandToken(size_t index, Guidance guide) {
        using namespace std::literals;
        if (index >= src.length()) {
            return {index, TokenType::kEOF};
        }
        CharType firstChar = getChar(index);
        if (guide == Guidance::kTemplateString) {
            return expandTemplateString(index);
        } else if (src.substr(index, 2) == "f\""sv) {
            index += 2;
            while (index < src.length() && src[index] != '{' && src[index] != '\"') {
                index = skipQuotedChar(index);
            }
            if (getChar(index) == '{') {
                return {index + 1, TokenType::kStringBegin};
            } else if (getChar(index) == '\"') {
                return {index + 1, TokenType::kStringFull};
            }
            return {index, TokenType::kUnknown};
        } else if (firstChar == '"') {
            index++;
            while (index < src.length() && src[index] != '\"') {
                index = skipQuotedChar(index);
            }
            if (getChar(index) == '\"') {
                return {index + 1, TokenType::kStringFull};
            }
            return {index, TokenType::kUnknown};
        } else if (firstChar == '\'') {
            index++;
            if (index < src.length() && src[index] != '\'') {
                index = skipQuotedChar(index);
                if (getChar(index) == '\'') {
                    return {index + 1, TokenType::kChar};
                }
            }
            return {index, TokenType::kUnknown};
        } else if (isdigit(firstChar)) {
            // numerical
            skipNumSeq(index);
            if (getChar(index) == '.') {
                index++;
                if (isdigit(getChar(index))) {
                    skipNumSeq(index);
                } else {
                    return {index - 1, TokenType::kInt};
                }
            }
            if (getChar(index) == 'e') {
                index++;
                if (getChar(index) == '+' || getChar(index) == '-') {
                    index++;
                } else if (!isdigit(getChar(index))) {
                    return {index, TokenType::kUnknown};
                }
                if (isdigit(getChar(index))) {
                    skipNumSeq(index);
                } else {
                    return {index, TokenType::kUnknown};
                }
            } else if (getChar(index) == 'f' || getChar(index) == 'd') {
                return {index + 1, TokenType::kReal};
            }
            return {index, TokenType::kInt};
        } else if (isSingleOnlySymbol(firstChar)) {
            return {index + 1, TokenType::kSymbol};
        } else if (isConcatSymbol(firstChar)) {
            while (isConcatSymbol(getChar(index))) {
                index++;
            }
            return {index, TokenType::kSymbol};
        } else if (isLegalIdent(firstChar)) {
            while (isLegalIdent(getChar(index))) {
                index++;
            }
            return {index, TokenType::kIdentifier};
        }
        return {index, TokenType::kUnknown};
    }

    std::pair<size_t, TokenType> expandTemplateString(size_t index) {
        if (getChar(index) == '}') {
            index++;
            while (index < src.length() && src[index] != '{' && src[index] != '\"') {
                index = skipQuotedChar(index);
            }
            if (getChar(index) == '{') {
                return {index + 1, TokenType::kStringMiddle};
            } else if (getChar(index) == '\"') {
                return {index + 1, TokenType::kStringEnd};
            }
        }
        return {index, TokenType::kUnknown};
    }

    void skipNumSeq(size_t& index) {
        do {
            index++;
        } while (isdigit(getChar(index)));
    }

    size_t skipQuotedChar(size_t index) {
        if (getChar(index) == '\\') {
            index += 1;
            CharType c = getChar(index);
            // escape char like '\n', '\0xaa', '\0uaaaa'
            if (c == 'n' || c == 'r' || c == 't' || c == '\\') {
                // single char
                return index + 1;
            } else if (c == 'u' || c == 'x') {
                // unicode
                index += 1;
                int len = c == 'u' ? 8 : 2;
                for (int i = 0; i < len; i++) {
                    c = getChar(index);
                    if (isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
                        index++;
                    }
                }
                return index;
            }
        } else {
            return index + 1;
        }
        return index;
    }

    bool isSingleOnlySymbol(CharType c) {
        return c == ';' || c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}' || c == ',' || c == ':';
    }

    bool isConcatSymbol(CharType c) {
        return c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '=' || c == '!' || c == '<' ||
               c == '>' || c == '&' || c == '|' || c == '^' || c == '~' || c == '.' || c == '?';
    }

    bool isLegalIdent(CharType c) {
        // utf-8
        return isalpha(c) || isdigit(c) || c == '_' || c & 0x80;
    }

    CharType getChar(size_t index) {
        if (index >= src.length()) {
            return '\0';
        }
        return src[index];
    }

    size_t getCacheKey(size_t index, Guidance guide) {
        if (SIZE_MAX / static_cast<size_t>(Guidance::__size) <= index) {
            std::abort();
        }
        return index * static_cast<size_t>(Guidance::__size) + static_cast<size_t>(guide);
    }

    std::unordered_map<size_t, LexerIteratorWithType> cache;
    StringViewType src;
};

}; // namespace rulejit