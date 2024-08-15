#pragma once

#include <vector>
#include <list>
#include <deque>

#include "defs.hpp"
#include "mem.hpp"
#include "ir/type.hpp"
#include "ir/func.hpp"

namespace rulejit {

struct VMContext {
    FunctionManager fm;
    TypeManager tm;
};

struct ThreadVM {
    std::vector<reg> r;
    std::vector<reg*> a;
    struct FunctionExecutionContext {
    };
};

struct VM {
    struct ObjPools {

    } pool;
    VMContext ctx;
    Memory globalMemory;
    // TODO: padding
    std::list<ThreadVM> threads;
};

}
