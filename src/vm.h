#ifndef __VM_H
#define __VM_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "string_view.h"

//
#define LOG_DEBUG(msg, ...) do { fprintf(stdout,   "[INFO] " msg "", ##__VA_ARGS__); } while (0)
#define LOG_WARNING(msg, ...) do { fprintf(stderr, "[WARNING] " msg "", ##__VA_ARGS__); } while(0)
#define LOG_ERROR(msg, ...) do { fprintf(stderr,   "[ERROR] " msg "", ##__VA_ARGS__); exit(1); } while(0)


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
    inst_type_t inst;
    int64_t operand;

} inst_t;

// the virtual machine
typedef struct
{
    word_t stack[STACK_MAX_WORD_SIZE];
    uint64_t sp = 0;

    inst_t program[PROGRAM_MAX_INST_SIZE];
    word_t ip = 0;

    word_t mem[MEM_SIZE_BYTES];

} vm;

/*
// loading and saving programs (binary)
void vm_load_binary_from_file(vm *vm, const char *file_path)
{

}

//
void vm_save_binary_to_file(vm *vm, const char *file_path)
{

}

//
void vm_execute(vm *vm)
{
    while (vm->program[vm->ip].inst != INST_HALT)
    {
        //fetch_inst();
        //execute_inst();
    }

}
*/


#endif // __VM_H
