#ifndef __STRING_VIEW_H
#define __STRING_VIEW_H

#include <stdint.h>
#include <string.h>
#include <ctype.h>

typedef struct
{
    size_t len;
    const char *data;

} sv_t;

#define TMP_BUFFER_SIZE 1024
char _tmp_buffer[TMP_BUFFER_SIZE];

//
int sv_find_char(sv_t sv, char token)
{
    int i = 0;
    while ((size_t)i < sv.len && sv.data[i] != token) i++;
    if ((size_t)i == sv.len)
        return -1;
    return i;
}

//
bool sv_cmp(sv_t sv0, sv_t sv1)
{
    if (sv0.len != sv1.len)
        return false;
    
    return memcmp(sv0.data, sv1.data, sv0.len) == 0;
}

//
bool sv_cmp_cstr(sv_t sv, const char *cstr)
{
    return memcmp(cstr, sv.data, sv.len) == 0;
}

//
int64_t sv_to_int64(sv_t sv)
{
    memset(_tmp_buffer, '\0', TMP_BUFFER_SIZE);
    memcpy(_tmp_buffer, sv.data, sv.len);
    return (int64_t)atoi(_tmp_buffer);
}

//
sv_t sv_from_cstr(const char *cstr)
{
    return (sv_t) {
        .len = strlen(cstr),
        .data = cstr,
    };
}

//
sv_t sv_chop_by_delim(sv_t *sv, char delim)
{
    size_t i = 0;
    while (i < sv->len && sv->data[i] != delim) i++;

    sv_t res = {
        .len = i,
        .data = sv->data,
    };

    if (i < sv->len)
    {
        sv->len  -= i + 1;
        sv->data += i + 1;
    }
    else
    {
        sv->len  -= i;
        sv->data += i;
    }

    return res;

}

//
sv_t sv_trim_left(sv_t sv)
{
    size_t i = 0;
    while (i < sv.len && isspace(sv.data[i])) i++;

    return (sv_t) {
        .len = sv.len - i,
        .data = sv.data + i,
    };
}

//
sv_t sv_trim_right(sv_t sv)
{
    size_t i = 0;
    while (i < sv.len && isspace(sv.data[sv.len - 1 - i])) i++;

    return (sv_t) {
        .len = sv.len - i,
        .data = sv.data,
    };
}

//
sv_t sv_trim(sv_t sv)
{
    return sv_trim_right(sv_trim_left(sv));
}


#endif // __STRING_VIEW_H
