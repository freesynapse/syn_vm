#ifndef __SYN_VM_H
#define __SYN_VM_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "common/string_view.h"
#include "common/fileio.h"
#include "common/log.h"


//
#define STACK_MAX_WORD_SIZE             1024
#define MEM_SIZE_BYTES          16*1024*1024
#define PROGRAM_MAX_INST_SIZE           1024

typedef union 
{
    uint64_t u64;
    int64_t  i64;
    double   f64;
    void    *ptr;
} word_t;

// instruction set
typedef enum
{
    INST_NOP,
    // integer arithmetics
    INST_IADD,
    INST_ISUB,
    INST_IMUL,
    INST_IDIV,
    // float arithmetics
    INST_FADD,
    INST_FSUB,
    INST_FMUL,
    INST_FDIV,
    // integer binary ops
    INST_IEQ,
    // float binary ops
    INST_FGE,
    // stack manipulation
    INST_PUSH,
    INST_SCPY,
    INST_SWAP,
    INST_DROP,
    // control flow
    INST_HALT,
    INST_JMP,
    INST_JMP_IF,

} inst_type_t;

// instruction (byte-code)
typedef struct
{
    inst_type_t type;
    // TODO : NaN-boxing for operand, and have the operand reinterpreted
    //        at runtime based on instruction type?
    word_t operand;

} inst_t;

//
const char *inst_to_cstr(inst_t inst)
{
    switch (inst.type)
    {
        case INST_NOP:      return "INST_NOP";
        // integer arithmetics
        case INST_IADD:     return "INST_IADD";
        case INST_ISUB:     return "INST_ISUB";
        case INST_IMUL:     return "INST_IMUL";
        case INST_IDIV:     return "INST_IDIV";
        // float arithmetics
        case INST_FADD:     return "INST_FADD";
        case INST_FSUB:     return "INST_FSUB";
        case INST_FMUL:     return "INST_FMUL";
        case INST_FDIV:     return "INST_FDIV";
        // integer binary ops
        case INST_IEQ:      return "INST_EQ";
        // float binary ops
        case INST_FGE:      return "INST_FGE";
        // stack manipulation
        case INST_PUSH:     return "INST_PUSH";
        case INST_SCPY:     return "INST_SCPY";
        case INST_SWAP:     return "INST_SWAP";
        case INST_DROP:     return "INST_DROP";
        // control flow
        case INST_HALT:     return "INST_HALT";
        case INST_JMP:      return "INST_JMP";
        case INST_JMP_IF:   return "INST_JMP_IF";
        
        default:
            LOG_ERROR("illegal instruction.\n");
    }
}

//
bool inst_has_operand(inst_t inst)
{
    switch (inst.type)
    {
        case INST_NOP:      return 0;
        // integer arithmetics
        case INST_IADD:     return 0;
        case INST_ISUB:     return 0;
        case INST_IMUL:     return 0;
        case INST_IDIV:     return 0;
        // float arithmetics
        case INST_FADD:     return 0;
        case INST_FSUB:     return 0;
        case INST_FMUL:     return 0;
        case INST_FDIV:     return 0;
        // integer binary ops
        case INST_IEQ:      return 0;
        // float binary ops
        case INST_FGE:      return 0;
        // stack manipulation
        case INST_PUSH:     return 1;
        case INST_SCPY:     return 1;
        case INST_SWAP:     return 1;
        case INST_DROP:     return 0;
        // control flow
        case INST_HALT:     return 0;
        case INST_JMP:      return 1;
        case INST_JMP_IF:   return 1;
        default:
            LOG_ERROR("illegal instruction '%s'.\n", inst_to_cstr(inst));
    }

}

// the virtual machine
typedef struct
{
    word_t stack[STACK_MAX_WORD_SIZE];
    uint32_t stack_size = 0;
    uint32_t sp = 0;

    inst_t program[PROGRAM_MAX_INST_SIZE];
    uint32_t program_size = 0;
    uint32_t ip = 0;

    word_t mem[MEM_SIZE_BYTES];

} synvm_t;

synvm_t syn_vm;

//
word_t number_literal_as_word(sv_t sv)
{
    word_t res = { 0 };
    char *endptr;

    memcpy(_tmp_buffer, sv.data, sv.len);
    _tmp_buffer[sv.len] = '\0';

    // Try to parse as an unsigned integer first, calculating the number
    // of parsed characters and comparing to the total number of characters.
    // If these don't match, it indicates a double, e.g. "69.0" will be
    // parsed as two chars to an unsigned int, and hence should be cast as
    // a float.
    //
    // Furthermore, we can cast all integers to unsigned, since in case of 
    // negative numbers, the .u64 will be 2's complement, and thus the .i64 
    // union member will contain the correct value.
    //
    uint64_t u = strtoull(_tmp_buffer, &endptr, 10);
    if (_tmp_buffer == endptr)
        LOG_ERROR("parsing of number literal (u64) '%s' failed.\n", _tmp_buffer);
    if (strlen(_tmp_buffer) == (size_t)(endptr - _tmp_buffer))
        res.u64 = u;
    // implies floating point
    else
    {
        double d = strtod(_tmp_buffer, &endptr);
        if (_tmp_buffer == endptr)
            LOG_ERROR("parsing of number literal (f64) '%s' failed.\n", _tmp_buffer);
        res.f64 = d;
    }

    return res;
}

//
void syn_vm_dump_stack(synvm_t *vm)
{
    LOG_INFO("synvm stack [%d]:\n", vm->stack_size);
    printf("     #\t"
           "                           f64"
           "                   i64"
           "                     u64"
           "                     ptr\n");
    for (uint64_t i = 0; i < vm->stack_size; i++)
        printf("%6ld\t%30.10lf\t%20ld\t%20lu\t%20p\n", i, 
               vm->stack[i].f64, 
               vm->stack[i].i64, 
               vm->stack[i].u64, 
               vm->stack[i].ptr);
}

//
void syn_vm_load_instruction(synvm_t *vm, inst_t *instructions, size_t instruction_count)
{
    for (size_t i = 0; i < instruction_count; i++)
        vm->program[vm->program_size++] = instructions[i];
}

//
void syn_vm_dump_program(synvm_t *vm)
{
    printf("     #\t\tinstruction\n");
    for (uint32_t i = 0; i < vm->program_size; i++)
    {
        inst_t inst = vm->program[i];
        printf("%6d\t\t%s %ld\n", i, inst_to_cstr(inst), inst_has_operand(inst) ? inst.operand.i64 : 0);
    }
}

//
void syn_vm_push_program_instruction(synvm_t *vm, inst_t inst)
{
    vm->program[vm->program_size++] = inst;
}

//
void syn_vm_write_binary_to_file(synvm_t *vm, const char *file_path)
{
    FILE *fp;
    if ((fp = fopen(file_path, "wb")) == NULL) LOG_ERROR("could not open file '%s'.\n", file_path);

    size_t nwords = fwrite(vm->program, sizeof(inst_t), vm->program_size, fp);
    if (ferror(fp)) LOG_ERROR("could not write binary file '%s'.\n", file_path);
    else LOG_INFO("wrote %ld bytes to '%s'.\n", nwords * sizeof(inst_t), file_path);

    fclose(fp);

    fileio_write_buffer_to_file(file_path, "wb", (void *)vm->program, vm->program_size * sizeof(inst_t), NULL);
}

//
void syn_vm_execute(synvm_t *vm, int inst_limit)
{
    // inst_limit == -1 --> loop indefinitely (default)

    uint64_t addr_a;
    uint64_t addr_b ;
    word_t tmp;

    LOG_INFO("executing %d instructions.\n", inst_limit > 0 ? inst_limit : -1);

    int n = 0;
    while (vm->program[vm->ip].type != INST_HALT && n != inst_limit)
    {
        if (vm->ip >= vm->program_size)
            LOG_ERROR("illegal instruction access.\n");

        inst_t inst = vm->program[vm->ip];
        switch (inst.type)
        {
            case INST_NOP:
                break;

            // integer arithmetics
            //
            case INST_IADD:
                if (vm->stack_size < 2) LOG_ERROR("stack underflow.\n");
                vm->stack[vm->stack_size - 2] = { .u64 = vm->stack[vm->stack_size - 2].u64 + vm->stack[vm->stack_size - 1].u64 };
                vm->stack_size--;
                break;

            case INST_ISUB:
                if (vm->stack_size < 2) LOG_ERROR("stack underflow.\n");
                vm->stack[vm->stack_size - 2] = { .u64 = vm->stack[vm->stack_size - 2].u64 - vm->stack[vm->stack_size - 1].u64 };
                vm->stack_size--;
                break;

            case INST_IMUL:
                if (vm->stack_size < 2) LOG_ERROR("stack underflow.\n");
                vm->stack[vm->stack_size - 2] = { .u64 = vm->stack[vm->stack_size - 2].u64 * vm->stack[vm->stack_size - 1].u64 };
                vm->stack_size--;
                break;

            case INST_IDIV:
                if (vm->stack_size < 2) LOG_ERROR("stack underflow.\n");
                vm->stack[vm->stack_size - 2] = { .u64 = vm->stack[vm->stack_size - 2].u64 / vm->stack[vm->stack_size - 1].u64 };
                vm->stack_size--;
                break;

            // float arithmetics
            //
            case INST_FADD:
                if (vm->stack_size < 2) LOG_ERROR("stack underflow.\n");
                vm->stack[vm->stack_size - 2] = { .f64 = vm->stack[vm->stack_size - 2].f64 + vm->stack[vm->stack_size - 1].f64 };
                vm->stack_size--;
                break;

            case INST_FSUB:
                if (vm->stack_size < 2) LOG_ERROR("stack underflow.\n");
                vm->stack[vm->stack_size - 2] = { .f64 = vm->stack[vm->stack_size - 2].f64 - vm->stack[vm->stack_size - 1].f64 };
                vm->stack_size--;
                break;

            case INST_FMUL:
                if (vm->stack_size < 2) LOG_ERROR("stack underflow.\n");
                vm->stack[vm->stack_size - 2] = { .f64 = vm->stack[vm->stack_size - 2].f64 * vm->stack[vm->stack_size - 1].f64 };
                vm->stack_size--;
                break;

            case INST_FDIV:
                if (vm->stack_size < 2) LOG_ERROR("stack underflow.\n");
                vm->stack[vm->stack_size - 2] = { .f64 = vm->stack[vm->stack_size - 2].f64 / vm->stack[vm->stack_size - 1].f64 };
                vm->stack_size--;
                break;

            // integer binary ops
            //
            case INST_IEQ:
                if (vm->stack_size < 1)
                    LOG_ERROR("stack underflow.\n");
                if (vm->stack_size >= STACK_MAX_WORD_SIZE)
                    LOG_ERROR("stack overflow.\n");
                //
                vm->stack[vm->stack_size - 2].u64 = (vm->stack[vm->stack_size - 2].u64 == vm->stack[vm->stack_size - 1].u64);
                vm->stack_size--;
                break;

            // float binary ops
            //
            case INST_FGE:
                if (vm->stack_size < 2)
                    LOG_ERROR("stack underflow.\n");
                vm->stack[vm->stack_size - 2].u64 = (vm->stack[vm->stack_size - 2].f64 >= vm->stack[vm->stack_size - 1].f64);
                vm->stack_size--;
                break;

            // stack manipulation
            //
            case INST_PUSH:
                if (vm->stack_size >= STACK_MAX_WORD_SIZE) LOG_ERROR("stack overflow.\n");
                vm->stack[vm->stack_size++] = inst.operand;
                break;
                            
            case INST_SCPY:
                if (vm->stack_size >= STACK_MAX_WORD_SIZE)
                    LOG_ERROR("stack overflow.\n");
                if (vm->stack_size <= inst.operand.u64)
                    LOG_ERROR("stack underflow.\n");
                if (inst.operand.i64 < 0)
                    LOG_ERROR("illegal operand.\n");
                
                vm->stack[vm->stack_size] = vm->stack[vm->stack_size - 1 - inst.operand.u64];
                vm->stack_size++;
                break;

            case INST_SWAP:
                if (vm->stack_size <= inst.operand.u64)
                    LOG_ERROR("stack underflow.\n");
                if (inst.operand.i64 < 0)
                    LOG_ERROR("illegal operand.\n");
                
                addr_a = vm->stack_size - 1;
                addr_b = vm->stack_size - 1 - inst.operand.u64;
                tmp = vm->stack[addr_a];
                vm->stack[addr_a] = vm->stack[addr_b];
                vm->stack[addr_b] = tmp;
                break;

            case INST_DROP:
                if (vm->stack_size < 1)
                    LOG_ERROR("stack underflow.\n");
                vm->stack_size--;
                break;

            // control flow
            //
            case INST_HALT:
                break;
            
            case INST_JMP:
                if (inst.operand.i64 < 0 || inst.operand.i64 > (int)vm->program_size - 1)
                    LOG_ERROR("%s: illegal address (%ld).\n", inst_to_cstr(inst), inst.operand.i64);
                // need to decrease ip by one since it will be incremented after the switch
                vm->ip = inst.operand.i64 - 1;
                break;

            case INST_JMP_IF:
                if (vm->stack_size < 1)
                    LOG_ERROR("stack underflow.\n");
                if (inst.operand.i64 < 0 || inst.operand.i64 > (int)vm->program_size - 1)
                    LOG_ERROR("%s: illegal address (%ld).\n", inst_to_cstr(inst), inst.operand.u64);
                //
                if (vm->stack[vm->stack_size - 1].i64 > 0)
                {
                    // need to decrease ip by one since it will be incremented after the switch
                    vm->ip = inst.operand.u64 - 1;
                }
                // pop value from the stack
                vm->stack_size--;
                break;

            default:
                LOG_ERROR("illegal instruction %d.\n", inst.type);

        }
        
        vm->ip++;
        n++;
    }

    if (vm->program[vm->ip].type == INST_HALT)
        LOG_INFO("halted (INST_HALT) after %d instructions.\n", n);
    else
        LOG_INFO("halted after %d instructions (limit set).\n", inst_limit);

}


#endif // __SYN_VM_H
