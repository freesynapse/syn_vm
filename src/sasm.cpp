
#include <stdio.h>

#include "vm.h"

//
string_view_t read_sasm_file(const char *file_path)
{
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

    string_view_t sv = sv_from_cstr(buffer);
    return sv;

}

//
void translate_to_bytecode(vm_t *vm, string_view_t sv_source)
{
    // read souce by svs
    while (sv_source.len > 0)
    {
        string_view_t sv = sv_trim(sv_chop_by_delim(&sv_source, '\n'));
        // convert sv to byte-code instruction
        if (sv.len > 0)
        {
            // extract instruction and operand, sv contains the operand, if any
            string_view_t inst = sv_chop_by_delim(&sv, ' ');
            if (sv_cmp_cstr(inst, "nop"))
            {
                vm_push_program_instruction(vm, (inst_t) {
                    .type = INST_NOP,
                    .operand = 0,
                });
            }
            else if (sv_cmp_cstr(inst, "push"))
            {
                vm_push_program_instruction(vm, (inst_t) {
                    .type = INST_PUSH,
                    .operand = sv_to_int64(sv),
                });
            }
            else if (sv_cmp_cstr(inst, "halt"))
            {
                vm_push_program_instruction(vm, (inst_t) {
                    .type = INST_HALT,
                    .operand = 0,
                });
            }
        }
    }

}

//
void usage(const char *program) { fprintf(stderr, "Usage: %s <input.sasm> <output.bin>\n", program); }
char *shift_args(int *argc, char **argv[])
{
    *argc -= 1;
    char *arg = **argv;
    *argv += 1;
    return arg;
}

//
int main(int argc, char *argv[])
{
    char *program = shift_args(&argc, &argv);
    if (argc < 2)
    {
        usage(program);
        LOG_ERROR("No input/output file provided.\n");
    }
    
    char *input_file = shift_args(&argc, &argv);
    string_view_t source = read_sasm_file(input_file);

    translate_to_bytecode(&vm, source);

    // DEBUG
    vm_dump_program(&vm);

    char *output_file = shift_args(&argc, &argv);
    vm_write_binary_to_file(&vm, output_file);

    return 0;
}



