#ifndef __syn_vm_H
#define __syn_vm_H

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

typedef int64_t word_t;

// instruction set
typedef enum
{
    INST_NOP,
    INST_PUSH,
    INST_IADD,
    INST_SCPY,
    INST_JMP,
    INST_HALT,
} inst_type_t;

// instruction (byte-code)
typedef struct
{
    inst_type_t type;
    // TODO : NaN-boxing for operand, and have the operand reinterpreted
    //        at runtime based on instruction type?
    int64_t operand;

} inst_t;

//
const char *inst_to_cstr(inst_t inst)
{
    switch (inst.type)
    {
        case INST_NOP:      return "INST_NOP";
        case INST_PUSH:     return "INST_PUSH";
        case INST_IADD:     return "INST_IADD";
        case INST_SCPY:     return "INST_SCPY";
        case INST_JMP:      return "INST_JMP";
        case INST_HALT:     return "INST_HALT";
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
        case INST_PUSH:     return 1;
        case INST_IADD:     return 0;
        case INST_SCPY:     return 1;
        case INST_JMP:      return 1;
        case INST_HALT:     return 0;
        default:
            LOG_ERROR("illegal instruction '%s'.\n", inst_to_cstr(inst));
    }

}

// the virtual machine
typedef struct
{
    word_t stack[STACK_MAX_WORD_SIZE];
    size_t stack_size = 0;
    int64_t sp = 0;

    inst_t program[PROGRAM_MAX_INST_SIZE];
    size_t program_size = 0;
    int64_t ip = 0;

    word_t mem[MEM_SIZE_BYTES];

} synvm_t;

synvm_t syn_vm;

//
void syn_vm_dump_stack(synvm_t *vm)
{
    LOG_INFO("synvm stack [%ld]:\n", vm->stack_size);
    printf("\t   #\tvalue\n");
    for (uint64_t i = 0; i < vm->stack_size; i++)
        printf("\t%4ld\t%5ld\n", i, vm->stack[i]);
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
    for (uint32_t i = 0; i < vm->program_size; i++)
    {
        inst_t inst = vm->program[i];
        printf("%s %ld\n", inst_to_cstr(inst), inst_has_operand(inst) ? inst.operand : 0);
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

    LOG_INFO("executing %d instructions.\n", inst_limit > 0 ? inst_limit : -1);

    int n = 0;
    while (vm->program[vm->ip].type != INST_HALT && n != inst_limit)
    {
        //fetch_inst();
        inst_t inst = vm->program[vm->ip];
        switch (inst.type)
        {
            case INST_NOP:
                break;

            case INST_PUSH:
                if (vm->stack_size >= STACK_MAX_WORD_SIZE)
                    LOG_ERROR("stack overflow.\n");

                vm->stack[vm->stack_size++] = inst.operand;
                break;
            
            case INST_IADD:
                if (vm->stack_size < 2)
                    LOG_ERROR("stack underflow.\n");

                vm->stack[vm->stack_size - 2] = vm->stack[vm->stack_size - 1] + vm->stack[vm->stack_size - 2];
                vm->stack_size--;
                break;

            case INST_SCPY:
                if (vm->stack_size >= STACK_MAX_WORD_SIZE)
                    LOG_ERROR("stack overflow.\n");
                if (vm->stack_size - inst.operand <= 0)
                    LOG_ERROR("stack underflow.\n");
                if (inst.operand < 0)
                    LOG_ERROR("illegal operand.\n");
                
                vm->stack[vm->stack_size] = vm->stack[vm->stack_size - 1 - inst.operand];
                vm->stack_size++;
                break;

            case INST_JMP:
                if (inst.operand < 0 || inst.operand > (int)vm->program_size - 1)
                    LOG_ERROR("%s: illegal address (%ld).\n", inst_to_cstr(inst), inst.operand);
                vm->ip = inst.operand - 1;
                break;

            case INST_HALT:
                break;
            
            default:
                LOG_ERROR("illegal instruction %d.\n", inst.type);

        }
        
        //execute_inst();
        
        vm->ip++;
        n++;
    }

}


#endif // __syn_vm_H
