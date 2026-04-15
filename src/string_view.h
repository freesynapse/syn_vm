#ifndef __STRING_VIEW_H
#define __STRING_VIEW_H

#include <stdint.h>
#include <string.h>
#include <ctype.h>

typedef struct
{
    size_t len;
    const char *data;

} string_view_t;

//
string_view_t sv_from_cstr(const char *cstr)
{
    return (string_view_t) {
        .len = strlen(cstr),
        .data = cstr,
    };
}

//
string_view_t sv_chop_by_delim(string_view_t *sv, char delim)
{
    size_t i = 0;
    while (i < sv->len && sv->data[i] != delim) i++;

    string_view_t res = {
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
string_view_t sv_trim_left(string_view_t sv)
{
    size_t i = 0;
    while (i < sv.len && isspace(sv.data[i])) i++;

    return (string_view_t) {
        .len = sv.len - i,
        .data = sv.data + i,
    };
}

//
string_view_t sv_trim_right(string_view_t sv)
{
    size_t i = 0;
    while (i < sv.len && isspace(sv.data[sv.len - 1 - i])) i++;

    return (string_view_t) {
        .len = sv.len - i,
        .data = sv.data,
    };
}

//
string_view_t sv_trim(string_view_t sv)
{
    return sv_trim_right(sv_trim_left(sv));
}


#endif // __STRING_VIEW_H
