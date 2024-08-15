#pragma once

#include "defs.hpp"
#include "opcode.hpp"
#include "ir.hpp"
#include "tools/string_pool.hpp"

namespace rulejit {

struct FunctionManager {
    struct Function {
        std::vector<OPCode> code;
        std::vector<reg> constant;
        struct FunctionInfo {
            enum struct RegType {
                POINTER,
                DATA,
            };
            std::vector<RegType> regInfo;
            usize autoRequirement;
            usize paramCnt;
        } info;
    };
    struct FunctionTemplate {
        // TODO: ir
        IR ir;
        Function* onInstantiation;
        Function instantiation() {
            return Function{code, std::move(c)};
        }
    };
};

}