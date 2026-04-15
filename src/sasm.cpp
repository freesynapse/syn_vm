
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
void translate_to_bytecode(string_view_t sv)
{
    // read souce by lines
    while (sv.len > 0)
    {
        string_view_t line = sv_trim(sv_chop_by_delim(&sv, '\n'));
        // convert line to byte-code instruction
        if (line.len > 0)
        {
            printf("#%.*s#\n", (int)line.len, line.data);
            
        }
    }
    // printf("#%.*s#\n", (int)sv.len, sv.data);
}

//
void usage(const char *program) { fprintf(stderr, "Usage: %s <input.sasm>\n", program); }
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
    if (argc < 1)
    {
        usage(program);
        LOG_ERROR("No input file provided.\n");
    }
    
    char *input_file = shift_args(&argc, &argv);
    string_view_t sv = read_sasm_file(input_file);

    printf("source:\n%.*s\n", (int)sv.len, sv.data);

    translate_to_bytecode(sv);

    return 0;
}



