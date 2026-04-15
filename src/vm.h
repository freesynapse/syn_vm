#ifndef __VM_H
#define __VM_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "common/string_view.h"

//
#define LOG_INFO(msg, ...) do { fprintf(stdout, "[INFO] " msg "", ##__VA_ARGS__); } while (0)
#define LOG_WARNING(msg, ...) do { fprintf(stderr, "[WARNING] " msg "", ##__VA_ARGS__); } while(0)
#define LOG_ERROR(msg, ...) do { fprintf(stderr, "[ERROR] " msg "", ##__VA_ARGS__); exit(1); } while(0)


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
    INST_HALT,
} inst_type_t;

// instruction (byte-code)
typedef struct
{
    inst_type_t type;
    int64_t operand;

} inst_t;

//
const char *inst_to_cstr(inst_t inst)
{
    switch (inst.type)
    {
        case INST_NOP:      return "INST_NOP";
        case INST_PUSH:     return "INST_PUSH";
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
        case INST_HALT:     return 0;
        default:
            LOG_ERROR("illegal instruction.\n");
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

} vm_t;

static vm_t vm;

//
void vm_dump_stack(vm_t *vm)
{
    for (uint64_t i = 0; i < vm->stack_size; i++)
        printf("%3ld\t%ld\n", i, vm->stack[i]);

}

//
void vm_dump_program(vm_t *vm)
{
    for (uint32_t i = 0; i < vm->program_size; i++)
    {
        inst_t inst = vm->program[i];
        printf("%s %ld\n", inst_to_cstr(inst), inst_has_operand(inst) ? inst.operand : 0);
    }
}

//
void vm_push_program_instruction(vm_t *vm, inst_t inst)
{
    vm->program[vm->program_size++] = inst;
}

// loading and saving programs (binary)
//void vm_load_binary_from_file(vm *vm, const char *file_path)
//{
//
//}

//
void vm_write_binary_to_file(vm_t *vm, const char *file_path)
{
    FILE *fp;
    if ((fp = fopen(file_path, "wb")) == NULL) LOG_ERROR("could not open file '%s'.\n", file_path);

    size_t nwords = fwrite(&vm->program, sizeof(inst_t), vm->program_size, fp);
    if (ferror(fp)) LOG_ERROR("could not write binary file '%s'.\n", file_path);
    else LOG_INFO("wrote %ld bytes to '%s'.\n", nwords * sizeof(inst_t), file_path);

    fclose(fp);
}

//
//void vm_execute(vm *vm)
//{
//    while (vm->program[vm->ip].inst != INST_HALT)
//    {
//        //fetch_inst();
//        //execute_inst();
//    }
//
//}


#endif // __VM_H
