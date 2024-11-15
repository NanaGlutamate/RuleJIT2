/**
 * @file opcode.hpp
 * @author nanaglutamate
 * @brief define opcodes used in VM
 * @date 2024-11-14
 * 
 * @details 
 * 
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>nanaglutamate</td><td>2024-11-14</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

/**
 * memory model:
 * 
 * REG     --  arg / calculate / local variable, the same reg can only store data or pointer. cannot get address
 * CONST   --  const member, cannot get address (but may modified through `LOADcb`)
 * 
 * AUTO    --  return ptr / storage for non-escape class / referenced struct, automatically release when function return.
 *             if stores struct, should alloc boxed struct (Box<T>)
 *             VM will not check use-after-free error due to return pointers to AUTO vars (todo: when debug switch turned off)
 * 
 * layout on 'reg' stack:
 *   frame 1: [  REG       ]
 *   frame 2:          [  REG      ]
 *                     ^^^^^ function args
 *                          ^^^ returned values
 * 
 * layout on 'auto' stack (object on 'auto' stack never moves):
 *   chunk 1: |[  frame1     ][  frame2 ][  frame3  ] --unused-- |
 *   chunk 2: |[  frame4 ][  frame5           ]                  |
 *                                             ^ HEAD
 * 
 * object storage:
 *   1. object is stored with header
 *   2. struct is stored as-is
 *   3. function are stored as struct contains function pointer
 *      while closure are stored as struct contains function pointer and capture pointer
 *   4. trait object are stored as struct contains function table pointer (with type indentifier) and data pointer (may boxed struct)
 *   5. dynamic object are stored as class contains native map pointer, which has member
 *     a. '.' for real object (boxed if struct)
 *     b. '..' for meta-table
 *     c. '[]' for dyn[] which stores user set member by '[]'
 *     d. other user set member by '.'
 *   6. list object are stored like struct (0: T, 1: T, ...) (static list) 
 *      or class (len: usize, 0: T, 1: T, ...) (dynamic list) 
 *   7. sum type are stored as indexed-enum, may use illegal state to store index (such as Opt<class>(0) for None)
 * 
 * calling conventions:
 *   1. normal argument
 *     a. normal argument are passed as the order of their declaration
 *     b. 'ref struct' are passed by by pointer and offset, caller must make sure class holding this struct will not recycled during GC
 *     c. huge struct (normally bigger than 2 reg.) are passed by pointer, pass-by-value semantic is guaranteed by callee (maybe CoW)
 *     d. small struct (function is garanteed to be small struct) are passed by 1 or 2 reg, low-indexed reg stores lower endian of data.
 *     e. dynamic object are passed like class object, by pointer
 *   2. special case
 *     a. first argument is reserved for capture object
 *     b. when return a huge struct (normally bigger than 2 reg.), should accept a pointer to storage of returned value (just behind capture object)
 * 
 * exception:
 *   1. recommand to return a Opt<T> or Except<T, E> for error (or any type impl 'Err'). throw is also supported:
 *     a. when enter try-catch block, exception trap is recorded
 *     b. when exception throws (must implies Throw), unfold the stack
 *     c. when meet exception trap, jump to target location, clear current exception trap and resume (may check exception type and re-throw it)
 * 
 * function load:
 *   1. when a function is load, all token it referenced is load
 *   2. during template instantiation, CONST is copied (may COW) and initialized
 * 
 * opcode:
 *   IMM     : signed immediate
 *   OFFSET  : unsigned offest
 *   REG     : register
 * 
 *          |0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1|
 *          |0 1 2 3 4 5 6 7 8 9 a b c d e f 0 1 2 3 4 5 6 7 8 9 a b c d e f|
 *  ABC   : |  OpCode(8)    |  REG(8)       |  REG(8)       |  REG(8)       |
 *  ABCi  : |  OpCode(8)    |  REG(8)       |  REG(8)       |  IMM(8)       |
 *  ABCo  : |  OpCode(8)    |  REG(8)       |  REG(8)       |  OFFSET(8)    |
 *  ABiCo : |  OpCode(8)    |  REG(8)       |  IMM(8)       |  OFFSET(8)    |
 *  ABoCo : |  OpCode(8)    |  REG(8)       |  OFFSET(8)    |  OFFSET(8)    |
 *  ABr   : |  OpCode(8)    |  REG(8)       |  REG(8)       |  [RESERVED]   |
 *  ABo   : |  OpCode(8)    |  REG(8)       |  OFFSET(16)                   |
 *  ABi   : |  OpCode(8)    |  REG(8)       |  IMM(16)                      |
 *  Ai    : |  OpCode(8)    |  IMM(24)                                      |
 * 
 */

namespace rulejit {

 // TODO: disable place data on stack, all struct should allocate on 'AUTO', therefore design of template is easier
enum class OPCode {
        ILL = 0, // illegal opcode
        NOP = 1, // NOP
    // ABC
        // R[A] = R[B] op R[C] // int, uint, float
        ADDu, ADDf, SUBu, SUBf, MULi, MULf, DIVi, DIVf, MULu, DIVu, MODu, 
        SHLu, SHRu, 
        AND, OR, XOR, // NOT is ABr
        GEu, GEi, LEu, LEi, Gu, Gi, Lu, Li, EQ, 
        GEf, LEf, Gf, Lf, 
        // R[A] = [R[B] + R[C]]
        LOADrr, LOADrrp, // register + register (pointer)
        // [R[A] + R[C]] = R[B]
        STORErr, STORErrp, // register + register
        // // R[A] = [R[B]].attr(R[C])
        // LOADa, LOADap, // attr string like "size"
        // // [R[A]].attr(R[C]) = R[B]
        // STOREa, STOREap, // attr
        // R[A] = if (R[C]) R[B] else R[A]
        CMOV, 
        // // R[A] = *(((type*)&R[B]) + R[C]), extract member from R[B] ([unsigned] short low / high or char lowlow / ... or float)
        // EXTsr, EXTcr, 
        // EXTusr, EXTucr, 
        // EXTfr, 
        // // *(((type*)&R[A]) + R[C]) = type(R[B]), emplace member to R[A] (short low / high or char lowlow / ...)
        // EMPsr, EMPcr, 
        // EMPusr, EMPucr, 
        // EMPfr, 
    // ABCi
        // R[A] = [R[B] + imm]
        LOADi, LOADip, // imm diff
        // [R[A] + imm] = R[B]
        STOREi, STOREip, // imm diff
    // ABCo
        // R[A] = new(AUTO[OFFSET]) type(R[B])
        ALLOCsr, // stack register
        // throw type(R[A])(R[B], ..., R[B+OFFSET])
        THROW, 
        // // R[A] = *(((type*)&R[B]) + OFFSET), extract member from R[B]
        // EXTs, EXTc, 
        // EXTus, EXTuc, 
        // EXTf, 
        // // *(((type*)&R[A]) + OFFSET) = type(R[B]), emplace member to R[A]
        // EMPs, EMPc, 
        // EMPus, EMPuc, 
        // EMPf, 
    // ABiCo
    // ABoCo
        // R[A] = STATIC[OFFSETb] if initialized, else call function related to the static object to init it.
        LOADst, LOADstp, // load static
        // R[A] = new(AUTO[OFFSETc]) type(CONST[OFFSETb])
        ALLOCsc, // stack const
    // ABr
        // R[A] = !R[B]
        NOT, 
        // // R[A] = [R[B]]
        // LOAD, LOADp, 
        // // [R[A]] = R[B]
        // STORE, STOREp, 
        // R[A] = f R[B]
        DTRANSuf, DTRANSfu, DTRANSif, DTRANSfi, 
        // R[A] = R[B]
        MOV, 
        // R[A] = new type(R[B])
        ALLOChr, // heap register
    // ABo
        // R[A] = new type(CONST[OFFSET])
        ALLOChc, // heap const
        // R[A] = [R[B] + OFFSET]
        LOADao, LOADaop, 
        // [R[A] + OFFSET] = R[B]
        STOREao, STOREaop, 
        // {R[A], R[A+1]}(R[A+2], ..., R[A+OFFSET]), all arg will not modified
        CALLc, // call closure
        // R[A](R[A+1], ..., R[A+OFFSET]), all arg will not modified
        CALLf, // call function
        // return (R[A], ..., R[A + OFFSET])
        RET, 
        // R[A] = (function template R[A])<R[A+1], ..., R[A+OFFSET]>
        INSTANf, 
        // R[A] = (type template R[A])<R[A+1], ..., R[A+OFFSET]>
        INSTANt, 
        // R[A] = CONST[OFFSET]
        LOADc, 
    // ABi
        // if (R[A] op 0) IP += IMM;
        BEZ, BNZ, 
    // Ai
        // branch
        BR, 
        // register exception TRAP
        TRAP, 
    __TOTAL_COUNT, 
};

static_assert(static_cast<size_t>(OPCode::__TOTAL_COUNT) < 0x80);

}
