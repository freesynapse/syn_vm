
#include <stdio.h>

#include "syn_vm.h"
#include "common/args.h"
#include "common/fileio.h"

//
sv_t read_sasm_file(const char *file_path)
{
    LOG_INFO("reading sasm source from '%s'...", file_path);

    FILE *fp;
    if ((fp = fopen(file_path, "r")) == NULL) LOG_ERROR("could not open file '%s'.\n", file_path);

    fseek(fp, 0, SEEK_END);
    if (ferror(fp)) LOG_ERROR("could not read file '%s'.\n", file_path);

    size_t file_size = ftell(fp);
    if (ferror(fp)) LOG_ERROR("could not read file '%s'.\n", file_path);

    fseek(fp, 0, SEEK_SET);
    if (ferror(fp)) LOG_ERROR("could not read file '%s'.\n", file_path);

    // TODO : free this memory
    char *buffer = (char *)malloc(file_size + 1);
    fread(buffer, file_size, 1, fp);
    buffer[file_size] = '\0';

    fclose(fp);

    sv_t sv = sv_from_cstr(buffer);
    return sv;

}

//
void translate_to_bytecode(synvm_t *vm, sv_t sv_source)
{
    // read souce by svs
    while (sv_source.len > 0)
    {
        sv_t sv = sv_trim(sv_chop_by_delim(&sv_source, '\n'));
        // convert sv to byte-code instruction
        if (sv.len > 0)
        {
            // extract instruction and operand, sv contains the operand, if any
            sv_t inst = sv_chop_by_delim(&sv, ' ');
            if (sv_cmp_cstr(inst, "nop"))
            {
                syn_vm_push_program_instruction(vm, (inst_t) {
                    .type = INST_NOP,
                    .operand = 0,
                });
            }
            else if (sv_cmp_cstr(inst, "push"))
            {
                syn_vm_push_program_instruction(vm, (inst_t) {
                    .type = INST_PUSH,
                    .operand = sv_to_int64(sv),
                });
            }
            else if (sv_cmp_cstr(inst, "iadd"))
            {
                syn_vm_push_program_instruction(vm, (inst_t) {
                    .type = INST_IADD,
                    .operand = 0,
                });
            }
            else if (sv_cmp_cstr(inst, "scpy"))
            {
                syn_vm_push_program_instruction(vm, (inst_t) {
                    .type = INST_SCPY,
                    .operand = sv_to_int64(sv),
                });

            }
            else if (sv_cmp_cstr(inst, "jmp"))
            {
                syn_vm_push_program_instruction(vm, (inst_t) {
                    .type = INST_JMP,
                    .operand = sv_to_int64(sv),
                });
            }
            else if (sv_cmp_cstr(inst, "halt"))
            {
                syn_vm_push_program_instruction(vm, (inst_t) {
                    .type = INST_HALT,
                    .operand = 0,
                });
            }
            else
                LOG_ERROR("unknown instruction '%.*s'.\n", (int)inst.len, inst.data);
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

    // save byte-code to disk
    syn_vm_write_binary_to_file(&syn_vm, output_file);

    //
    fileio_free_buffers();
    return 0;
}



