
#include <stdio.h>

#include "syn_vm.h"
#include "common/args.h"
#include "common/fileio.h"

//
#define MAX_JMP_TABLES_SIZE 64
typedef struct
{
    sv_t label;
    uint32_t src_lineno;
} label_addr_t;

typedef struct
{
    label_addr_t labels[MAX_JMP_TABLES_SIZE];
    size_t count = 0;
} label_address_table_t;
//
typedef struct
{
    uint32_t jmp_inst_addr;
    sv_t dest_label;
} jmp_addr_t;
typedef struct
{
    jmp_addr_t addrs[MAX_JMP_TABLES_SIZE];
    size_t count;
} jmp_resolve_table_t;
//
static label_address_table_t label_address_table;
static jmp_resolve_table_t jmp_resolve_table;

//
void translate_to_bytecode(synvm_t *vm, sv_t sv_source)
{
    // read souce by svs
    LOG_INFO("parsing sasm.\n");
    uint32_t lineno = 0;
    while (sv_source.len > 0)
    {
        sv_t sv = sv_trim(sv_chop_by_delim(&sv_source, '\n'));

        // remove comments, starting with ';'
        sv = sv_chop_by_delim(&sv, ';');
        sv = sv_trim(sv);

        if (sv.data[0] == ';')
        {
            // lineno++;
            continue;
        }
        
        // convert sv to byte-code instruction
        if (sv.len == 0)
            continue;
        else if (sv.len > 0)
        {
            // label
            if (sv_find_char(sv, ':') > 0)
            {
                label_address_table.labels[label_address_table.count] = {
                    .label = sv_chop_by_delim(&sv, ':'),
                    .src_lineno = lineno
                };
                label_address_table.count++;
                continue;
            }

            // extract instruction and operand, sv contains the operand, if any
            sv_t inst = sv_chop_by_delim(&sv, ' ');
            if      (sv_cmp_cstr(inst, "nop"))  { syn_vm_push_program_instruction(vm, (inst_t) { .type = INST_NOP,  .operand = 0 }); }
            // integer arithmetics
            else if (sv_cmp_cstr(inst, "iadd")) { syn_vm_push_program_instruction(vm, (inst_t) { .type = INST_IADD, .operand = 0 }); }
            else if (sv_cmp_cstr(inst, "isub")) { syn_vm_push_program_instruction(vm, (inst_t) { .type = INST_ISUB, .operand = 0 }); }
            else if (sv_cmp_cstr(inst, "imul")) { syn_vm_push_program_instruction(vm, (inst_t) { .type = INST_IMUL, .operand = 0 }); }
            else if (sv_cmp_cstr(inst, "idiv")) { syn_vm_push_program_instruction(vm, (inst_t) { .type = INST_IDIV, .operand = 0 }); }
            // float arithmetics
            else if (sv_cmp_cstr(inst, "fadd")) { syn_vm_push_program_instruction(vm, (inst_t) { .type = INST_FADD, .operand = 0 }); }
            else if (sv_cmp_cstr(inst, "fsub")) { syn_vm_push_program_instruction(vm, (inst_t) { .type = INST_FSUB, .operand = 0 }); }
            else if (sv_cmp_cstr(inst, "fmul")) { syn_vm_push_program_instruction(vm, (inst_t) { .type = INST_FMUL, .operand = 0 }); }
            else if (sv_cmp_cstr(inst, "fdiv")) { syn_vm_push_program_instruction(vm, (inst_t) { .type = INST_FDIV, .operand = 0 }); }
            // integer binary ops
            else if (sv_cmp_cstr(inst, "ieq"))  { syn_vm_push_program_instruction(vm, (inst_t) { .type = INST_IEQ,  .operand = 0 }); }
            // float binary ops
            else if (sv_cmp_cstr(inst, "fge"))  { syn_vm_push_program_instruction(vm, (inst_t) { .type = INST_FGE,  .operand = 0 }); }
            // stack manipulation
            else if (sv_cmp_cstr(inst, "push")) { syn_vm_push_program_instruction(vm, (inst_t) { .type = INST_PUSH, .operand = number_literal_as_word(sv) }); }
            else if (sv_cmp_cstr(inst, "scpy")) { syn_vm_push_program_instruction(vm, (inst_t) { .type = INST_SCPY, .operand = number_literal_as_word(sv) }); }
            else if (sv_cmp_cstr(inst, "swap")) { syn_vm_push_program_instruction(vm, (inst_t) { .type = INST_SWAP, .operand = number_literal_as_word(sv) }); }
            else if (sv_cmp_cstr(inst, "drop")) { syn_vm_push_program_instruction(vm, (inst_t) { .type = INST_DROP, .operand = 0 }); }
            // control flow
            else if (sv_cmp_cstr(inst, "halt")) { syn_vm_push_program_instruction(vm, (inst_t) { .type = INST_HALT, .operand = 0 }); }
            else if (sv_cmp_cstr(inst, "jmp"))
            {
                // save programm address for 2nd pass
                jmp_resolve_table.addrs[jmp_resolve_table.count++] = { .jmp_inst_addr = vm->program_size, .dest_label = sv };
                syn_vm_push_program_instruction(vm, (inst_t){ .type = INST_JMP, .operand = 0 });    // operand (addr) resolved in 2nd pass
            }
            else if (sv_cmp_cstr(inst, "jmp_if"))
            {
                // save programm address for 2nd pass
                jmp_resolve_table.addrs[jmp_resolve_table.count++] = { .jmp_inst_addr = vm->program_size, .dest_label = sv };
                syn_vm_push_program_instruction(vm, (inst_t){ .type = INST_JMP_IF, .operand = 0 });    // operand (addr) resolved in 2nd pass
            }
            else
                LOG_ERROR("unknown instruction '%.*s'.\n", (int)inst.len, inst.data);
        }

        lineno++;
    }

    // 2nd pass, resolving jumps and labels
    LOG_INFO("resolving jump address table.\n");
    for (size_t i = 0; i < jmp_resolve_table.count; i++)
    {
        jmp_addr_t jmp_addr = jmp_resolve_table.addrs[i];
        for (size_t j = 0; j < label_address_table.count; j++)
        {
            label_addr_t label_addr = label_address_table.labels[j];
            if (sv_cmp(jmp_addr.dest_label, label_addr.label))
                vm->program[jmp_addr.jmp_inst_addr].operand = { .i64=label_addr.src_lineno };
        }
    }

    
}

//
void usage(const char *program) { fprintf(stderr, "Usage: %s -i <input.sasm> -o <output.bin>\n", program); }

//
int main(int argc, char *argv[])
{
    char *input_file;
    char *output_file;

    // parse arguments
    char *program = shift_args(&argc, &argv);
    if (argc < 2)
    {
        usage(program);
        LOG_ERROR("no input/output file provided.\n");
    }
    
    while (argc > 0)
    {
        if (strcmp(*argv, "-i") == 0)
        {
            shift_args(&argc, &argv);
            input_file = shift_args(&argc, &argv);
        }
        else if (strcmp(*argv, "-o") == 0)
        {
            shift_args(&argc, &argv);
            output_file = shift_args(&argc, &argv);
        }
        else
        {
            usage(program);
            LOG_ERROR("unknown option '%s'\n", *argv);
        }
    }

    // load sasm and translate
    char *buffer = fileio_read_file(input_file, "r", NULL);
    sv_t source = sv_from_cstr(buffer);

    translate_to_bytecode(&syn_vm, source);

    LOG_INFO("translated bytecode instructions:\n");
    syn_vm_dump_program(&syn_vm);

    // save byte-code to disk
    syn_vm_write_binary_to_file(&syn_vm, output_file);

    //
    fileio_free_buffers();
    return 0;
}



