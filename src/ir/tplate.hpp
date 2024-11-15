/**
 * @file tplate.hpp
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

#include "defs.hpp"
#include "ir.hpp"

namespace rulejit {

struct TemplateParam {
    StringToken paramName;
    std::vector<TraitToken> paramConstraints;
};

struct FunctionTemplate {
    IR ir;
    std::vector<TemplateParam> templateParams;
};

struct TypeTemplate {
    std::vector<TemplateParam> templateParams;
};

struct TraitTemplate {
    std::vector<TemplateParam> templateParams;
};

struct ImplTemplate {
    std::vector<TemplateParam> templateParams;
};

struct TemplateManager {



};

}