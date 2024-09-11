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