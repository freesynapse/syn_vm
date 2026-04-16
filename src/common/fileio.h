#ifndef __FILEIO_H
#define __FILEIO_H

#include <stdio.h>

#include "string_view.h"
#include "log.h"

//
#define MAX_ALLOC_FILE_BUFFERS 16

static char *alloc_buffers[MAX_ALLOC_FILE_BUFFERS];
static size_t alloc_buffers_size = 0;

//
char *fileio_read_file(const char *file_path, const char *mode, size_t *out_bytes_read)
{

    FILE *fp;
    if ((fp = fopen(file_path, mode)) == NULL) LOG_ERROR("could not open file '%s'.\n", file_path);

    fseek(fp, 0, SEEK_END);
    if (ferror(fp)) LOG_ERROR("could not read file '%s' (%s).\n", file_path, strerror(errno));

    size_t file_size = ftell(fp);
    if (ferror(fp)) LOG_ERROR("could not read file '%s' (%s).\n", file_path, strerror(errno));

    fseek(fp, 0, SEEK_SET);
    if (ferror(fp)) LOG_ERROR("could not read file '%s' (%s).\n", file_path, strerror(errno));

    char *buffer = (char *)malloc(file_size + 1);
    
    size_t nbytes = fread(buffer, 1, file_size, fp);
    if (ferror(fp)) LOG_ERROR("could not read file '%s' (%s).\n", file_path, strerror(errno));
    if (out_bytes_read != NULL)
        *out_bytes_read = nbytes;

    buffer[file_size] = '\0';

    fclose(fp);

    alloc_buffers[alloc_buffers_size++] = buffer;

    return buffer;

}

//
void fileio_write_buffer_to_file(const char *file_path, const char *mode, void *data, size_t bytes, size_t *out_bytes_written)
{
    FILE *fp;

    if ((fp = fopen(file_path, mode)) == NULL) LOG_ERROR("could not open file '%s' (%s).\n", file_path, strerror(errno));
    
    size_t nbytes = fwrite(data, 1, bytes, fp);
    if (out_bytes_written != NULL)
        *out_bytes_written = nbytes;
    
    if (ferror(fp)) LOG_ERROR("could not write binary file '%s' (%s).\n", file_path, strerror(errno));

}



// (optional)
void fileio_free_buffers()
{
    for (size_t i = 0; i < alloc_buffers_size; i++)
        free(alloc_buffers[i]);
}


#endif // __FILEIO_H
