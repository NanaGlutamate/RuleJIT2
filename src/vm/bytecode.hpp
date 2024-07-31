#pragma once

#include <cstddef>

static_assert(sizeof(void*) <= 8);

namespace rulejit {

/**
 * memory model:
 * 
 * REG     --  arg / calculate / local variable, cannot get address
 * CONST   --  const member, cannot get address (but may modified through `LOADcb`)
 * 
 * AUTO    --  return ptr / storage for non-escape class / referenced struct, automatically release when function return.
 *             VM will not check use-after-free error due to return pointers to AUTO vars (todo: when debug switch turned off)
 * 
 * layout on 'reg' stack:
 *   frame 1: [  REG       ]
 *   frame 2:          [  REG      ]
 *                     ^^^^^  function args
 * 
 * layout on 'auto' stack (object on 'auto' stack never moves):
 *   chunk 1: |[  frame1     ][  frame2 ][  frame3  ] --unused-- |
 *   chunk 2: |[  frame4 ][  frame5           ]                  |
 *                                             ^ HEAD
 * 
 * object storage:
 *   1. function are stored as struct contains function pointer and capture pointer
 *   2. trait object are stored as struct contains function table pointer (with type indentifier) and data pointer
 *   3. dynamic object are stored as native map with member
 *     a. '.' for real object
 *     b. '..' for meta-table
 *   4. list object are stored like struct (when all size are known in compile time) or 
 *
 * calling conventions:
 *   1. normal argument
 *     a. normal argument are passed as the order of their declaration
 *     b. 'ref struct' are passed by pointer
 *     c. huge struct (normally bigger than 2 reg.) are passed by pointer, pass-by-value semantic is guaranteed by caller
 *     d. small struct (function is garanteed to be small struct) are passed compressed by reference, which means multi members may placed in single reg.
 *   2. special case
 *     a. when return a huge struct (normally bigger than 2 reg.), should accept a pointer to storage of returned value as the first argument
 * 
 * opcode:
 *   IMM     : signed immediate
 *   OFFSET  : unsigned offest
 *   REG     : register
 * 
 *           |0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1|
 *           |0 1 2 3 4 5 6 7 8 9 a b c d e f 0 1 2 3 4 5 6 7 8 9 a b c d e f|
 *  ABC   :  |  OpCode(8)    |  REG(8)       |  REG(8)       |  REG(8)       |
 *  ABCi  :  |  OpCode(8)    |  REG(8)       |  REG(8)       |  IMM(8)       |
 *  ABCo  :  |  OpCode(8)    |  REG(8)       |  REG(8)       |  OFFSET(8)    |
 *  ABiCo :  |  OpCode(8)    |  REG(8)       |  IMM(8)       |  OFFSET(8)    |
 *  ABoCo :  |  OpCode(8)    |  REG(8)       |  OFFSET(8)    |  OFFSET(8)    |
 *  ABr   :  |  OpCode(8)    |  REG(8)       |  REG(8)       |  [RESERVED]   |
 *  ABo   :  |  OpCode(8)    |  REG(8)       |  OFFSET(16)                   |
 *  ABi   :  |  OpCode(8)    |  REG(8)       |  IMM(16)                      |
 *  Ai    :  |  OpCode(8)    |  IMM(24)                                      |
 * 
 */
enum class OP {
    // ABC
        // R[A] = R[B] op R[C] // int, uint, float
        ADDu, ADDf, SUBu, SUBf, MULi, MULf, DIVi, DIVf, MULu, DIVu, MODu, 
        SHLu, SHRu, 
        GEu, GEi, LEu, LEi, Gu, Gi, Lu, Li, EQu, // float can use uint test
        // R[A] = [R[B] + R[C]]
        LOADrr, // register + register 
        // [R[A] + R[C]] = R[B]
        STORErr, // register + register 
        // R[A] = [R[B]].attr(R[C])
        LOADa, // attr
        // [R[A]].attr(R[C]) = R[B]
        STOREa, // attr
        // R[A] = if (R[C]) R[B] else R[A]
        CMOV,
    // ABCi
        // R[A] = [R[B] + imm]
        LOADo, // offset
        // [R[A] + imm] = R[B]
        STOREo, // offset
    // ABCo
        // R[A] = new(AUTO[OFFSET]) type(R[B])
        ALLOCsr, // stack register
    // ABiCo
        // R[A] = CONST[IMM] (if initialized), else branch to OFFSET in another stack, and put returned value to CONST[IMM] when ret, used in static call
        LOADs, // load static
    // ABoCo
        // R[A] = new(AUTO[OFFSETc]) type(CONST[OFFSETb])
        ALLOCsc, // stack const
    // ABr
        // R[A] = [R[B]]
        LOAD, 
        // [R[A]] = R[B]
        STORE, 
        // R[A] = f R[B]
        DTRANSuf, DTRANSfu, DTRANSif, DTRANSfi, 
        // R[A] = R[B]
        MOV, 
        // R[A] = new type(R[B])
        ALLOChr, // heap register
    // ABo
        // R[A] = new type(CONST[OFFSET])
        ALLOChc, // heap const
        // R[A] = [R[B]].attr(CONST[OFFSET])
        LOADac, 
        // [R[A]].attr(CONST[OFFSET]) = R[B]
        STOREac, 
        // {R[A], R[A+1]}(R[A+2], ..., R[A+OFFSET]), R[A+2:] is not reserved and may store return value
        CALL, 
        // return (R[A], ..., R[A + OFFSET])
        RET, 
        // R[A] = (function template R[A])<R[A], ..., R[A+OFFSET]>
        INSTANf, 
        // R[A] = (type template R[A])<R[A], ..., R[A+OFFSET]>
        INSTANt, 
        // R[A] = CONST[OFFSET]
        LOADc, 
    // ABi
        // if (R[A] op 0) IP += IMM;
        BEZ, BNZ, 
    // Ai
        // branch
        BR, 
    TOTAL_COUNT,
};
static_assert(static_cast<size_t>(OP::TOTAL_COUNT) < 0x80);

}
