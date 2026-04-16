
#include <stdio.h>
#include <assert.h>
#include <ctype.h>

#include "syn_vm.h"

#include "common/string_view.h"
#include "common/args.h"
#include "common/fileio.h"
#include "common/log.h"

//
void usage(const char *program) { fprintf(stderr, "Usage: %s <input.bin> [-l INSTRUCTION_LIMIT]\n", program); }

//
int main(int argc, char *argv[])
{
    char *program;
    char *input_file;

    // argument parsing
    program = shift_args(&argc, &argv);

    if (!argc)
    {
        usage(program);
        LOG_ERROR("no input file provided.\n");
    }

    input_file = shift_args(&argc, &argv);

    int inst_limit = -1;
    while (argc > 0)
    {
        if (strcmp(*argv, "-l") == 0)
        {
            shift_args(&argc, &argv);
            inst_limit = atoi(*argv);
            shift_args(&argc, &argv);
        }
        else
        {
            usage(program);
            LOG_ERROR("unknown option '%s'.\n", *argv);
        }
    }

    // load instructions into vm and execute program
    size_t nbytes = 0;
    char *buffer = fileio_read_file(input_file, "rb", &nbytes);
    assert(nbytes % sizeof(inst_t) == 0 && "incomplete file");
    
    syn_vm_load_instruction(&syn_vm, (inst_t *)buffer, nbytes / sizeof(inst_t));
    syn_vm_execute(&syn_vm, inst_limit);
    syn_vm_dump_stack(&syn_vm);
    
    //
    fileio_free_buffers();
    return 0;

}