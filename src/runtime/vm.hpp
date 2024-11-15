/**
 * @file vm.hpp
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
#include <list>

#include "defs.hpp"
#include "gc/mem.hpp"
#include "ir/type.hpp"
#include "ir/func.hpp"
#include "ir/tplate.hpp"

namespace rulejit {

struct CodeManager {
    struct Section {
        std::vector<OPCode> code;
        std::vector<reg> staticVar;
        std::vector<reg> constant;
        struct FunctionInfo {
            usize autoStorageRequirement;
            // never higher than 256 according to restriction of OPCode
            u8 regUsageCnt;
            u8 paramCnt;
            u8 returnCnt;
        } info;
    };
    Section instantiation(const FunctionTemplate& ft) {}
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
    void step() {
        
    }
};

}
