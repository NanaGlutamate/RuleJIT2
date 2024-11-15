/**
 * @file ir.hpp
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

#include <vector>

namespace rulejit {

enum struct IROP {
    ALLOCs, ALLOCc,
};

struct Block {};

struct IR {
    std::vector<Block> blocks;
};

}
