#pragma once

#include <vector>

#include "defs.hpp"

namespace rulejit {

struct TemplateParam {
    StringToken paramName;
    std::vector<TraitToken> paramConstraints;
};

struct TemplateManager {



};

}