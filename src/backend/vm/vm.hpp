#pragma once

#include <vector>
#include <list>
#include <deque>

#include "defs.hpp"
#include "gc/mem.hpp"
#include "ir/type.hpp"
#include "ir/func.hpp"
#include "ir/tplate.hpp"

namespace rulejit {

struct CodeManager {
    struct Section {
        std::vector<reg> staticVar;

    };
};

struct VMContext {
    TemplateManager* tpm;

    FunctionManager* fm;
    TypeManager* tm;

    CodeManager* cm;
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
