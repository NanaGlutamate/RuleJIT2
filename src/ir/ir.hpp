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
